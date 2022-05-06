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

    define_native_accessor("map", map_getter, map_setter, 0);
    // FIXME: This is probably better as a direct property (without an accessor), but we construct (and therefore
    //        initialize) this object too soon (in initializer lists), before we get the interfaces we need (and way
    //        before we even Load).
    define_native_accessor("modDescription", mod_description_getter, {}, 0);

    auto& cosmo_global_object = verify_cast<GlobalObject>(global_object);

    define_direct_property("Server", heap().allocate<Server>(cosmo_global_object, cosmo_global_object), 0);
    define_direct_property("Command",
                           m_command_object = heap().allocate<Command>(cosmo_global_object, cosmo_global_object), 0);

    define_native_function("on", on, 2, 0);
    define_native_function("sayText2", say_text_2, 8, 0);
}

JS_DEFINE_NATIVE_FUNCTION(Game::map_getter) { return JS::js_string(vm, g_SMAPI->GetCGlobals()->mapname.ToCStr()); }

JS_DEFINE_NATIVE_FUNCTION(Game::map_setter)
{
    auto map_name = vm.argument(0);
    if (!map_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, map_name);

    auto& map_name_string = map_name.as_string().string();

    // At some point in history, IsMapValid automatically formatted the passed "filename" as "maps/%s.bsp"
    // At some point in history, it stopped doing that (the present), as maps can live elsewhere than just "maps". But
    // changelevel doesn't do this it doesn't seem? A changelevel of "maps/cp_metalworks" isn't valid, so I'm not sure
    // why this change was made...
    // TODO: We SHOULD support maps outside of "maps/%s.bsp".
    if (!Plugin::the().engine_server().IsMapValid(String::formatted("maps/{}.bsp", map_name_string).characters()))
        return vm.throw_completion<JS::Error>(global_object, "Invalid map");

    Plugin::the().engine_server().ChangeLevel(map_name_string.characters(), nullptr);
    return JS::js_undefined();
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

JS_DEFINE_NATIVE_FUNCTION(Game::say_text_2)
{
    auto message = vm.argument(0);
    if (!message.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, message);

    auto filter_argument = vm.argument(1);
    auto filter = filter_argument.is_nullish() ? RecipientFilter::all()
                                               : TRY(to_recipient_filter(vm, global_object, filter_argument));

    auto get_say_text_parameter = [&vm](int argument_index) -> Optional<const String&> {
        auto argument = vm.argument(argument_index);
        if (argument.is_string())
            return argument.as_string().string();

        return {};
    };

    auto param_1 = get_say_text_parameter(2);
    auto param_2 = get_say_text_parameter(3);
    auto param_3 = get_say_text_parameter(4);
    auto param_4 = get_say_text_parameter(5);

    auto source_entity_index = vm.argument(6);
    auto should_print_to_console = vm.argument(7);

    Plugin::the().say_text_2(filter, source_entity_index.is_number() ? source_entity_index.as_i32() : 0,
                             should_print_to_console.is_boolean() ? should_print_to_console.as_bool() : true,
                             message.as_string().string(), param_1, param_2, param_3, param_4);

    return JS::js_undefined();
}
}