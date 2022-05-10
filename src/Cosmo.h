#pragma once

#include "Forward.h"
#include "Types/IEngineSound.h"
#include <ISmmPlugin.h>
#include <eiface.h>
#include <igameevents.h>
#include <toolframework/itoolentity.h>

#include "RemoveSourceSpecifics.h"

#include "Scripting/GlobalObject.h"
#include "Signature.h"
#include <LibJS/Console.h>
#include <LibJS/Interpreter.h>
#include <LibJS/Runtime/VM.h>
#include <subhook/subhook.h>

namespace Cosmo
{
static constexpr StringView git_sha1 = COSMO_GIT_SHA1;
static constexpr StringView git_sha1_short = COSMO_GIT_SHA1_SHORT;

class Plugin final : public ISmmPlugin
{
public:
    // TODO: Perhaps we should have our own import module hook? Can the default implementation be abused?
    Plugin()
        : m_vm(JS::VM::create()), m_interpreter(JS::Interpreter::create<Cosmo::Scripting::GlobalObject>(m_vm)),
          m_console(m_interpreter->global_object().console()),
          m_global_object(verify_cast<Scripting::GlobalObject>(m_interpreter->global_object()))
    {
        m_global_object.console().set_client(m_console);
        m_vm->enable_default_host_import_module_dynamically_hook();
    }

    bool Load(PluginId, ISmmAPI*, char* error, size_t maxlength, bool late) override;
    bool Unload(char* error, size_t maxlen) override;

    const char* GetAuthor() override { return "James Puleo (Dr. Underscore)"; }
    const char* GetName() override { return "Cosmo"; }
    const char* GetDescription() override { return "Adds a JavaScript runtime for plugins"; }
    const char* GetURL() override { return "https://github.com/drunderscore/Cosmo"; }
    const char* GetLicense() override { return "GPL-3.0-only"; }
    const char* GetVersion() override
    {
        // This IS null-terminated, it just doesn't _have to_ be.
        return git_sha1_short.characters_without_null_termination();
    }
    const char* GetDate() override { return __DATE__; }
    const char* GetLogTag() override { return "Cosmo"; }

    JS::VM& vm() { return *m_vm; }
    JS::Interpreter& interpreter() { return *m_interpreter; }

    Scripting::GlobalObject& global_object() { return m_global_object; }

    // We want String& here, so we can ensure they are null-terminated
    void say_text_2(const RecipientFilter&, int source_entity_index, bool should_print_to_console,
                    const String& message, Optional<const String&> param_1 = {}, Optional<const String&> param_2 = {},
                    Optional<const String&> param_3 = {}, Optional<const String&> param_4 = {});

    ICvar& cvar() const { return *m_cvar; }
    IServerGameDLL& server_game_dll() const { return *m_server_game_dll; }
    IVEngineServer& engine_server() const { return *m_engine_server; }
    IServerTools& server_tools() const { return *m_server_tools; }
    IServerGameEnts& server_game_ents() const { return *m_server_game_ents; }
    IServerGameClients& server_game_clients() const { return *m_server_game_clients; }
    IEngineSound& engine_sound() const { return *m_engine_sound; }
    IGameEventManager2& game_event_manager() const { return *m_game_event_manager; }

    static Plugin& the() { return s_the; }
    static Plugin s_the;
    static SpewOutputFunc_t s_original_spew_output_func;

private:
    class Console final : public JS::ConsoleClient
    {
    public:
        explicit Console(JS::Console& console) : ConsoleClient(console) {}

        // I don't think there's any reason to ever allow script to clear the console.
        void clear() override {}

        void end_group() override {}

        JS::ThrowCompletionOr<JS::Value> printer(JS::Console::LogLevel, PrinterArguments) override;
    };

    static SpewRetval_t ansi_true_color_spew_output(SpewType_t spewType, const tchar* pMsg);

    static CBaseEntity* create_entity_by_name_hook(const char* classname, int forced_edict_index);

    static int dispatch_spawn_hook(CBaseEntity*);

    bool on_client_connect(edict_t*, const char* name, const char* address, char* reject_message,
                           int reject_message_max_length);

    void on_client_active(edict_t*, bool load_game);

    void on_client_disconnect(edict_t*);

    bool on_fire_event(IGameEvent*, bool dont_broadcast);

    ICvar* m_cvar{};
    IServerGameDLL* m_server_game_dll{};
    IVEngineServer* m_engine_server{};
    IServerTools* m_server_tools{};
    IServerGameEnts* m_server_game_ents{};
    IServerGameClients* m_server_game_clients{};
    IEngineSound* m_engine_sound{};
    IGameEventManager2* m_game_event_manager;

    NonnullRefPtr<JS::VM> m_vm;
    NonnullOwnPtr<JS::Interpreter> m_interpreter;
    Scripting::GlobalObject& m_global_object;
    Console m_console;

    static Signature s_create_entity_by_name_function;
    static subhook_t s_create_entity_by_name_subhook;

    static Signature s_dispatch_spawn_function;
    static subhook_t s_dispatch_spawn_subhook;
};

PLUGIN_GLOBALVARS()
}