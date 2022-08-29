#include "../Cosmo.h"

#include "../RemoveSourceSpecifics.h"

#include "Command.h"
#include "Game.h"
#include <LibJS/Runtime/GlobalObject.h>

namespace Cosmo::Scripting
{
Command::Command(JS::Realm& realm) : Object(*realm.intrinsics().object_prototype()) {}

void Command::initialize(JS::Realm& realm)
{
    Base::initialize(realm);

    define_native_function(realm, "register", register_, 2, 0);
    define_native_function(realm, "unregister", unregister, 1, 0);
    define_native_function(realm, "execute", execute, 1, 0);
}

JS::ThrowCompletionOr<JS::Value> Command::internal_get(const JS::PropertyKey& property_key, JS::Value receiver) const
{
    if (property_key.is_string())
    {
        auto* convar = Plugin::the().cvar().FindVar(property_key.as_string().characters());
        if (convar)
        {
            if (convar->IsFlagSet(FCVAR_NEVER_AS_STRING))
                return vm().throw_completion<JS::Error>("ConVar has FCVAR_NEVER_AS_STRING flag");

            return JS::js_string(vm(), convar->GetString());
        }
    }

    return Base::internal_get(property_key, receiver);
}

JS::ThrowCompletionOr<bool> Command::internal_set(const JS::PropertyKey& property_key, JS::Value value,
                                                  JS::Value receiver)
{
    if (property_key.is_string())
    {
        auto* convar = Plugin::the().cvar().FindVar(property_key.as_string().characters());
        if (convar)
        {
            auto value_as_string = TRY(value.to_string(vm()));
            convar->SetValue(value_as_string.characters());
            return true;
        }
    }

    return Base::internal_set(property_key, value, receiver);
}

void Command::visit_edges(Cell::Visitor& visitor)
{
    for (auto& kv : m_commands)
        visitor.visit(kv.value.callback);
}

void Command::on_script_concommand_execute(const CCommand& args)
{
    auto maybe_command = Plugin::the().global_object().game_object().command_object().m_commands.get(args.Arg(0));
    if (!maybe_command.has_value())
    {
        Warning("Script command handler was executed, but it couldn't find it's callback!\n");
        return;
    }

    JS::MarkedVector<JS::Value> arguments(Plugin::the().vm().heap());
    arguments.ensure_capacity(args.ArgC() - 1);
    for (auto i = 1; i < args.ArgC(); i++)
        arguments.unchecked_append(JS::js_string(Plugin::the().vm(), args[i]));

    auto maybe_return_value =
        JS::call(Plugin::the().vm(), *maybe_command->callback, JS::js_undefined(), move(arguments));
    if (maybe_return_value.is_error())
    {
        Warning("Script threw exception in it's command callback: %s\n",
                MUST(maybe_return_value.throw_completion().value()->to_string(Plugin::the().vm())).characters());
    }
}

JS_DEFINE_NATIVE_FUNCTION(Command::register_)
{
    auto this_value = vm.this_value();
    if (!this_value.is_object() || !is<Command>(this_value.as_object()))
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::NotAnObjectOfType, "Command");

    auto& command_object = static_cast<Command&>(this_value.as_object());

    auto command_name = vm.argument(0);
    if (!command_name.is_string())
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::NotAString, command_name);

    auto callback_function = vm.argument(1);
    if (!callback_function.is_function())
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::NotAFunction, callback_function);

    if (command_object.m_commands.contains(command_name.as_string().string()))
        return vm.throw_completion<JS::Error>("A command with that name has already been registered");

    command_object.m_commands.set(
        command_name.as_string().string(),
        {make<ConCommand>(command_name.as_string().string().characters(), on_script_concommand_execute),
         &callback_function.as_function()});

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(Command::unregister)
{
    auto this_value = vm.this_value();
    if (!this_value.is_object() || !is<Command>(this_value.as_object()))
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::NotAnObjectOfType, "Command");

    auto& command_object = static_cast<Command&>(this_value.as_object());

    auto command_name = vm.argument(0);
    if (!command_name.is_string())
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::NotAString, command_name);

    auto command_iterator = command_object.m_commands.find(command_name.as_string().string());

    if (command_iterator != command_object.m_commands.end())
    {
        g_SMAPI->UnregisterConCommandBase(g_PLAPI, command_iterator->value.command);
        command_object.m_commands.remove(command_iterator);
        return true;
    }

    return false;
}

JS_DEFINE_NATIVE_FUNCTION(Command::execute)
{
    auto command_name = vm.argument(0);
    if (!command_name.is_string())
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::NotAString, command_name);

    Plugin::the().engine_server().ServerCommand(
        String::formatted("{}\n", command_name.as_string().string()).characters());
    Plugin::the().engine_server().ServerExecute();

    return JS::js_undefined();
}
}