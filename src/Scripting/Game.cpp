#include "../Cosmo.h"

#include "../RemoveSourceSpecifics.h"

#include "Command.h"
#include "Game.h"
#include "GlobalObject.h"
#include "Helpers.h"
#include "Server.h"

namespace Cosmo::Scripting
{
Game::Game(GlobalObject& global_object) : Object(*global_object.object_prototype()) {}

void Game::initialize(JS::GlobalObject& global_object)
{
    Object::initialize(global_object);

    // FIXME: This is probably better as a direct property (without an accessor), but we construct (and therefore
    //        initialize) this object too soon (in initializer lists), before we get the interfaces we need (and way
    //        before we even Load).
    define_native_accessor("modDescription", mod_description_getter, {}, 0);

    auto& cosmo_global_object = verify_cast<GlobalObject>(global_object);

    define_direct_property("Server", heap().allocate<Server>(cosmo_global_object, cosmo_global_object), 0);
    define_direct_property("Command",
                           m_command_object = heap().allocate<Command>(cosmo_global_object, cosmo_global_object), 0);

    define_native_function("on", on, 2, 0);
}

JS_DEFINE_NATIVE_FUNCTION(Game::mod_description_getter)
{
    auto* game_description = Plugin::the().server_game_dll().GetGameDescription();

    // A mod should never really return nullptr or empty string here...
    if (!game_description || !*game_description)
        return JS::js_undefined();

    return JS::js_string(vm.heap(), game_description);
}

JS_DEFINE_NATIVE_FUNCTION(Game::on)
{
    auto this_value = vm.this_value(global_object);
    if (!this_value.is_object() || !is<Game>(this_value.as_object()))
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAnObjectOfType, "Game");

    auto& game_object = static_cast<Game&>(this_value.as_object());

    auto event_name = vm.argument(0);
    if (!event_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, event_name);

    auto event_callback = vm.argument(1);
    if (!event_callback.is_function())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAFunction, event_callback);

    game_object.m_event_handlers
        .ensure(event_name.as_string().string(), [&vm]() { return JS::MarkedVector<JS::FunctionObject*>(vm.heap()); })
        .append(&event_callback.as_function());

    return JS::js_undefined();
}
}