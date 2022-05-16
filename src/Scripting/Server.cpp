#include "../Cosmo.h"
#include <iclient.h>
#include <iserver.h>

#include "../RemoveSourceSpecifics.h"

#include "../Types/BaseEntity.h"
#include "Entity.h"
#include "Helpers.h"
#include "Server.h"
#include <LibJS/Runtime/GlobalObject.h>

namespace Cosmo::Scripting
{
Server::Server(GlobalObject& global_object) : Object(*global_object.object_prototype()) {}

void Server::initialize(JS::GlobalObject& global_object)
{
    Object::initialize(global_object);

    define_native_accessor("map", map_getter, map_setter, 0);

    define_native_function("createEntityByName", create_entity_by_name, 1, 0);
    define_native_function("createFakeClient", create_fake_client, 1, 0);
    define_native_function("getEntityByIndex", get_entity_by_index, 1, 0);
    define_native_function("emitSound", emit_sound, 2, 0);
    define_native_function("sayText2", say_text_2, 8, 0);
    define_native_function("getPlayerByUserId", get_player_by_userid, 1, 0);
    define_native_function("getEntityByHandle", get_entity_by_handle, 1, 0);
}

JS_DEFINE_NATIVE_FUNCTION(Server::map_getter) { return JS::js_string(vm, g_SMAPI->GetCGlobals()->mapname.ToCStr()); }

JS_DEFINE_NATIVE_FUNCTION(Server::map_setter)
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

JS_DEFINE_NATIVE_FUNCTION(Server::create_entity_by_name)
{
    auto entity_name = vm.argument(0);
    if (!entity_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, entity_name);

    auto* entity = Plugin::the().server_tools().CreateEntityByName(entity_name.as_string().string().characters());
    if (!entity)
        return JS::js_undefined();

    return Entity::create(verify_cast<GlobalObject>(global_object), entity);
}

JS_DEFINE_NATIVE_FUNCTION(Server::create_fake_client)
{
    auto client_name = vm.argument(0);
    if (!client_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, client_name);

    auto* entity = Plugin::the().server_game_ents().EdictToBaseEntity(
        Plugin::the().engine_server().CreateFakeClient(client_name.as_string().string().characters()));
    if (!entity)
        return JS::js_undefined();

    return Entity::create(verify_cast<GlobalObject>(global_object), entity);
}

JS_DEFINE_NATIVE_FUNCTION(Server::get_entity_by_index)
{
    auto index = vm.argument(0);
    if (!index.is_number())
        return vm.throw_completion<JS::TypeError>(global_object, String::formatted("{} is not a number", index));

    auto* maybe_edict = Plugin::the().engine_server().PEntityOfEntIndex(index.as_i32());
    if (!maybe_edict)
        return JS::js_undefined();

    auto* maybe_entity = Plugin::the().server_game_ents().EdictToBaseEntity(maybe_edict);
    if (!maybe_entity)
        return JS::js_undefined();

    return Entity::create(verify_cast<GlobalObject>(global_object), maybe_entity);
}

JS_DEFINE_NATIVE_FUNCTION(Server::emit_sound)
{
    auto entity_or_position = vm.argument(1);
    Variant<CBaseEntity*, Cosmo::SourceVector, Empty> source;

    if (entity_or_position.is_object() && is<Entity>(entity_or_position.as_object()))
        source = static_cast<Entity&>(entity_or_position.as_object()).entity();
    else
        source = TRY(to_source_vector(vm, global_object, entity_or_position));

    // Super sanity
    VERIFY(!source.has<Empty>());

    Cosmo::Scripting::emit_sound(vm, global_object, vm.argument(0), vm.argument(2), vm.argument(3), vm.argument(4),
                                 source.downcast<CBaseEntity*, Cosmo::SourceVector>());

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(Server::say_text_2)
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

    int source_entity_index;
    if (auto source_entity = vm.argument(6); source_entity.is_object() && is<Entity>(source_entity.as_object()))
        source_entity_index = static_cast<Entity&>(source_entity.as_object()).handle().GetEntryIndex();

    auto should_print_to_console = vm.argument(7);

    Plugin::the().say_text_2(filter, source_entity_index,
                             should_print_to_console.is_boolean() ? should_print_to_console.as_bool() : true,
                             message.as_string().string(), param_1, param_2, param_3, param_4);

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(Server::get_player_by_userid)
{
    auto user_id = vm.argument(0);
    if (!user_id.is_number())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, user_id, "number");

    // No nice way to do this -- must iterate all players. This is how the Engine does it too.
    for (auto i = 0; i < Plugin::the().engine_server().GetIServer()->GetClientCount(); i++)
    {
        auto* client = Plugin::the().engine_server().GetIServer()->GetClient(i);
        if (!client->IsConnected())
            continue;

        if (client->GetUserID() == user_id.as_i32())
            return Entity::create(verify_cast<GlobalObject>(global_object),
                                  Plugin::the().server_game_ents().EdictToBaseEntity(
                                      Plugin::the().engine_server().PEntityOfEntIndex(i + 1)));
    }

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(Server::get_entity_by_handle)
{
    auto handle_number = vm.argument(0);
    if (!handle_number.is_number())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, handle_number, "number");

    if (handle_number.as_i32() == INVALID_NETWORKED_EHANDLE_VALUE)
        return JS::js_undefined();

    CBaseHandle handle(handle_number.as_i32());
    if (!handle.IsValid())
        return JS::js_undefined();

    auto* entity = Plugin::the().server_tools().GetBaseEntityByEntIndex(handle.GetEntryIndex());
    if (!entity)
        return JS::js_undefined();

    return Entity::create(verify_cast<GlobalObject>(global_object), entity);
}
}