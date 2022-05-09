#include "Cosmo.h"
#include "RecipientFilter.h"

#include "RemoveSourceSpecifics.h"

#include "Scripting/Command.h"
#include "Scripting/Entity.h"
#include "Scripting/EventNames.h"
#include "Scripting/Game.h"
#include <LibCore/Stream.h>

namespace Cosmo
{
SH_DECL_HOOK5(IServerGameClients, ClientConnect, SH_NOATTRIB, 0, bool, edict_t*, const char*, const char*, char*, int);
SH_DECL_HOOK2_void(IServerGameClients, ClientActive, SH_NOATTRIB, 0, edict_t*, bool);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t*);

Plugin Plugin::s_the;
SpewOutputFunc_t Plugin::s_original_spew_output_func{};
// If a spew is using a default color (it's color is 255, 255, 255), then we use this color instead, based on it's type.
// These colors are from sys_dll.cpp
static Array<Optional<Color>, SPEW_TYPE_COUNT> s_default_spew_colors = {
    Optional<Color>(),
    Color(255, 90, 90),
    Color(255, 20, 20),
    Color(20, 70, 255),
};

typedef CBaseEntity* (*CreateEntityByNameFn)(const char* classname, int forced_edict_index);
Signature Plugin::s_create_entity_by_name_function(
    "55 89 E5 56 53 83 EC 10 8B 5D 0C 8B 75 08 83 FB FF 74 1A A1 ? ? ? ? 8B 10");
subhook_t Plugin::s_create_entity_by_name_subhook;

typedef int (*DispatchSpawnFn)(CBaseEntity*);
Signature Plugin::s_dispatch_spawn_function("55 89 E5 57 56 53 83 EC 2C 8B 5D 08 85 DB 0F ? ? ? ? ? A1 ? ? ? ? 89 C1");
subhook_t Plugin::s_dispatch_spawn_subhook;

// No StringView, ensure it's null-terminated
static String s_server_library_name = "tf/bin/server_srv.so";

PLUGIN_EXPOSE(CosmoPlugin, Plugin::s_the)

class BaseAccessor : public IConCommandBaseAccessor
{
public:
    bool RegisterConCommandBase(ConCommandBase* pCommandBase) override
    {
        /* Always call META_REGCVAR instead of going through the engine. */
        return META_REGCVAR(pCommandBase);
    }
} s_base_accessor;

JS::ThrowCompletionOr<JS::Value> Plugin::Console::printer(JS::Console::LogLevel log_level,
                                                          PrinterArguments printer_arguments)
{
    if (auto* values = printer_arguments.get_pointer<JS::MarkedVector<JS::Value>>())
    {
        auto output = String::join(" ", *values);

        switch (log_level)
        {
            case JS::Console::LogLevel::Debug:
                ConColorMsg({50, 170, 80}, "%s\n", output.characters());
                break;
            case JS::Console::LogLevel::Error:
            case JS::Console::LogLevel::Assert:
                ConColorMsg({255, 0, 0}, "%s\n", output.characters());
                break;
            case JS::Console::LogLevel::Info:
                ConColorMsg({170, 220, 255}, "%s\n", output.characters());
                break;
            case JS::Console::LogLevel::Log:
                Msg("%s\n", output.characters());
                break;
            case JS::Console::LogLevel::Warn:
            case JS::Console::LogLevel::CountReset:
                Warning("%s\n", output.characters());
                break;
            default:
                Msg("%s\n", output.characters());
                break;
        }
    }
    return JS::js_undefined();
}

bool Plugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
    PLUGIN_SAVEVARS()

    auto create_entity_by_name_address =
        s_create_entity_by_name_function.find_in_library(s_server_library_name.characters());
    if (!create_entity_by_name_address.has_value())
    {
        if (error && maxlen)
        {
            strncpy(error, "Could not find signature for CreateEntityByName", maxlen);
            return false;
        }
    }

    auto dispatch_spawn_address = s_dispatch_spawn_function.find_in_library(s_server_library_name.characters());
    if (!dispatch_spawn_address.has_value())
    {
        if (error && maxlen)
        {
            strncpy(error, "Could not find signature for DispatchSpawn", maxlen);
            return false;
        }
    }

    GET_V_IFACE_CURRENT(GetEngineFactory, m_cvar, ICvar, CVAR_INTERFACE_VERSION);
    GET_V_IFACE_ANY(GetServerFactory, m_server_game_dll, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
    GET_V_IFACE_CURRENT(GetEngineFactory, m_engine_server, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
    GET_V_IFACE_CURRENT(GetServerFactory, m_server_tools, IServerTools, VSERVERTOOLS_INTERFACE_VERSION);
    GET_V_IFACE_CURRENT(GetServerFactory, m_server_game_ents, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
    GET_V_IFACE_ANY(GetServerFactory, m_server_game_clients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);

    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientConnect, m_server_game_clients, this, &Plugin::on_client_connect,
                        false);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientActive, m_server_game_clients, this, &Plugin::on_client_active, true);
    SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, m_server_game_clients, this,
                        &Plugin::on_client_disconnect, false);

#if SOURCE_ENGINE >= SE_ORANGEBOX
    g_pCVar = m_cvar;
    ConVar_Register(0, &s_base_accessor);
#else
    ConCommandBaseMgr::OneTimeInit(&s_BaseAccessor);
#endif

    // Huge props to Justin (aka. sigsegv) for this idea -- had no idea this existed in tier0.
    // We'll only turn on ANSI color if you're a dedicated server, and on POSIX.
    // Both these conditions are pretty useless though, as the nature of this plugin requires both of these :^)
    // FIXME: I'd rather not use this macro to determine POSIX, there must be some Engine function to do this!
#ifdef POSIX
    if (m_engine_server->IsDedicatedServer())
    {
        // Print this message _before_ replacing the spew func. If something goes wrong, at least we know the last thing
        // that happened.
        Msg("Running a dedicated server on posix, colorizing your spew!\n");
        s_original_spew_output_func = GetSpewOutputFunc();
        SpewOutputFunc(ansi_true_color_spew_output);
    }
#endif

    s_create_entity_by_name_subhook =
        subhook_new(*create_entity_by_name_address, reinterpret_cast<void*>(create_entity_by_name_hook),
                    static_cast<subhook_flags>(0));
    subhook_install(s_create_entity_by_name_subhook);

    s_dispatch_spawn_subhook = subhook_new(*dispatch_spawn_address, reinterpret_cast<void*>(dispatch_spawn_hook),
                                           static_cast<subhook_flags>(0));
    subhook_install(s_dispatch_spawn_subhook);

    Msg("Loaded Cosmo\n");
    return true;
}

bool Plugin::Unload(char* error, size_t maxlen)
{
    if (s_original_spew_output_func)
    {
        SpewOutputFunc(s_original_spew_output_func);
        s_original_spew_output_func = nullptr;
    }

    auto& command_object = m_global_object.game_object().command_object();

    for (auto& kv : command_object.commands())
        g_SMAPI->UnregisterConCommandBase(g_PLAPI, kv.value.command);

    command_object.commands().clear();

    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientConnect, m_server_game_clients, this, &Plugin::on_client_connect,
                           false);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientActive, m_server_game_clients, this, &Plugin::on_client_active,
                           true);
    SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, m_server_game_clients, this,
                           &Plugin::on_client_disconnect, false);

    if (subhook_is_installed(s_create_entity_by_name_subhook))
        subhook_remove(s_create_entity_by_name_subhook);
    subhook_free(s_create_entity_by_name_subhook);

    if (subhook_is_installed(s_dispatch_spawn_subhook))
        subhook_remove(s_dispatch_spawn_subhook);
    subhook_free(s_dispatch_spawn_subhook);

    return true;
}

void Plugin::say_text_2(const RecipientFilter& filter, int source_entity_index, bool should_print_to_console,
                        const String& message, Optional<const String&> param_1, Optional<const String&> param_2,
                        Optional<const String&> param_3, Optional<const String&> param_4)
{
    // I'll accept the const_cast here -- it makes the end-user API nicer, by allowing a reference to a temporary,
    // so we can just use the std::initializer_list constructor of RecipientFilter implicitly
    // It's only really used in MessageEnd anyway (which calls into BroadcastMessage)
    auto user_message_buffer =
        m_engine_server->UserMessageBegin(&const_cast<RecipientFilter&>(filter), g_SMAPI->FindUserMessage("SayText2"));

    user_message_buffer->WriteByte(source_entity_index);
    user_message_buffer->WriteByte(should_print_to_console);
    user_message_buffer->WriteString(message.characters());
    user_message_buffer->WriteString(param_1.value_or("").characters());
    user_message_buffer->WriteString(param_2.value_or("").characters());
    user_message_buffer->WriteString(param_3.value_or("").characters());
    user_message_buffer->WriteString(param_4.value_or("").characters());

    m_engine_server->MessageEnd();
}

SpewRetval_t Plugin::ansi_true_color_spew_output(SpewType_t spew_type, const tchar* msg)
{
    // Let's NEVER touch anything meant for the log.
    if (spew_type == SPEW_LOG)
        return s_original_spew_output_func(spew_type, msg);

    // Copy it, so we can override it later if we need to
    auto output_color = GetSpewOutputColor();

    // This white color is s_DefaultOutputColor in dbg.cpp
    if (output_color.r() == 255 && output_color.g() == 255 && output_color.b() == 255)
    {
        // If the default color is used, and we don't have a spew color for it, then let's not do anything to it.
        // This will cover a ton of cases.
        auto maybe_spew_type_output_color = s_default_spew_colors[spew_type];
        if (!maybe_spew_type_output_color.has_value())
            return s_original_spew_output_func(spew_type, msg);

        output_color = maybe_spew_type_output_color.value();
    }

    // My terminal had issues with the ANSI reset being after a newline, and leaked color into the next spew. So
    // instead, let's replace the newline with our reset, and then re-add the newline.

    // All this string copying is pretty unfortunate :(
    StringView msg_view(msg);
    StringBuilder corrected_message_builder;
    corrected_message_builder.appendff("\u001b[38;2;{};{};{}m", output_color.r(), output_color.g(), output_color.b());

    // This will be 99% of spew, so try to put the ANSI color reset BEFORE the newline
    if (msg_view.ends_with('\n'))
    {
        corrected_message_builder.append(msg_view.substring_view(0, msg_view.length() - 1));
        corrected_message_builder.append("\u001B[0m");
        corrected_message_builder.append('\n');
    }
    else
    {
        corrected_message_builder.appendff(msg_view);
        corrected_message_builder.append("\u001B[0m");
    }

    return s_original_spew_output_func(spew_type, corrected_message_builder.to_string().characters());
}

CBaseEntity* Plugin::create_entity_by_name_hook(const char* classname, int forced_edict_index)
{
    auto* entity = reinterpret_cast<CreateEntityByNameFn>(subhook_get_trampoline(s_create_entity_by_name_subhook))(
        classname, forced_edict_index);
    if (entity)
    {
        auto& global_object = Plugin::the().global_object();

        global_object.game_object().dispatch_event(Scripting::EventNames::entity_create,
                                                   Scripting::Entity::create(global_object, entity));
    }

    return entity;
}

int Plugin::dispatch_spawn_hook(CBaseEntity* entity)
{
    auto success = reinterpret_cast<DispatchSpawnFn>(subhook_get_trampoline(s_dispatch_spawn_subhook))(entity);
    if (success == 0)
    {
        auto& global_object = Plugin::the().global_object();

        global_object.game_object().dispatch_event(Scripting::EventNames::entity_spawn,
                                                   Scripting::Entity::create(global_object, entity));
    }

    return success;
}

bool Plugin::on_client_connect(edict_t* player_edict, const char* name, const char* address, char* reject_message,
                               int reject_message_max_length)
{
    // FIXME: The edict has no CBaseEntity* yet, but eventually it will. Do we care?
    // FIXME: Events have no way of giving information back to the dispatcher, so how can we reject this connection,
    //        with a rejection message?

    global_object().game_object().dispatch_event(Scripting::EventNames::player_connect,
                                                 JS::js_string(vm().heap(), name), JS::js_string(vm().heap(), address));

    return true;
}

void Plugin::on_client_active(edict_t* player_edict, bool)
{
    global_object().game_object().dispatch_event(
        Scripting::EventNames::player_active,
        Scripting::Entity::create(global_object(), m_server_game_ents->EdictToBaseEntity(player_edict)));
}

void Plugin::on_client_disconnect(edict_t* player_edict)
{
    global_object().game_object().dispatch_event(
        Scripting::EventNames::player_disconnect,
        Scripting::Entity::create(global_object(), m_server_game_ents->EdictToBaseEntity(player_edict)));
}

CON_COMMAND(cosmo_run, "Run a script")
{
    if (args.ArgC() < 2)
        return;

    // FIXME: It would be nice if we could print all the Errors here. There's a formatter for it, not too hard.
    auto maybe_file_stream = Core::Stream::File::open(args.Arg(1), Core::Stream::OpenMode::Read);
    if (maybe_file_stream.is_error())
    {
        Warning("Unable to open script %s\n", args.Arg(1));
        return;
    }

    auto file_stream = maybe_file_stream.release_value();
    auto maybe_file_stream_size = file_stream->size();
    if (maybe_file_stream_size.is_error())
    {
        Warning("Failed to get script file size\n");
        return;
    }

    auto maybe_file_contents_buffer = ByteBuffer::create_uninitialized(maybe_file_stream_size.value());
    if (maybe_file_contents_buffer.is_error())
    {
        Warning("Unable to allocate buffer for script file contents\n");
        return;
    }

    auto maybe_file_contents_bytes = file_stream->read(maybe_file_contents_buffer.value());
    if (maybe_file_contents_bytes.is_error())
    {
        Warning("Unable to read script file contents\n");
        return;
    }
    auto file_contents_string = StringView(maybe_file_contents_bytes.value());

    auto& interpreter = Plugin::the().interpreter();
    auto maybe_script = JS::Script::parse(file_contents_string, interpreter.realm());
    if (maybe_script.is_error())
    {
        Warning("Script parsing failed:\n");
        for (auto& error : maybe_script.error())
            Warning("\t%s\n", error.message.characters());
    }
    else
    {
        auto maybe_return_value = interpreter.run(maybe_script.value());
        if (maybe_return_value.is_error())
        {
            VERIFY(maybe_return_value.throw_completion().value().has_value());
            Warning("Script execution threw an exception: %s\n",
                    MUST(maybe_return_value.throw_completion().value()->to_string(Plugin::the().global_object()))
                        .characters());
        }
    }
}
}