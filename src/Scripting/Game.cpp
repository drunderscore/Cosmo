#include "../Cosmo.h"

#include "../RemoveSourceSpecifics.h"

#include "Command.h"
#include "Game.h"
#include "GlobalObject.h"
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
}