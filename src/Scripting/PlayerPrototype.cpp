#include "../Cosmo.h"
#include "../Types/BaseEntity.h"
#include <basehandle.h>
#include <iclient.h>
#include <iserver.h>

#include "EntityPrototype.h"
#include "GlobalObject.h"
#include "PlayerPrototype.h"

namespace Cosmo::Scripting
{
PlayerPrototype::PlayerPrototype(GlobalObject& global_object) : PrototypeObject(global_object.entity_prototype()) {}

void PlayerPrototype::initialize(JS::GlobalObject& global_object)
{
    Prototype::initialize(global_object);
    define_native_accessor("name", name_getter, name_setter, 0);
    define_native_accessor("userId", userid_getter, {}, 0);
}

JS_DEFINE_NATIVE_FUNCTION(PlayerPrototype::name_getter)
{
    auto* this_entity = TRY(EntityPrototype::ensure_this_entity(vm, global_object));

    auto* client = Plugin::the().engine_server().GetIServer()->GetClient(
        this_entity->entity()->GetRefEHandle().GetEntryIndex() - 1);

    if (!client || !client->IsConnected())
        return vm.throw_completion<JS::InternalError>(global_object, "Failed to get client");

    return JS::js_string(vm, client->GetClientName());
}

JS_DEFINE_NATIVE_FUNCTION(PlayerPrototype::name_setter)
{
    auto* this_entity = TRY(EntityPrototype::ensure_this_entity(vm, global_object));

    auto name = vm.argument(0);
    if (!name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, name);

    auto* client = Plugin::the().engine_server().GetIServer()->GetClient(
        this_entity->entity()->GetRefEHandle().GetEntryIndex() - 1);

    if (!client || !client->IsConnected())
        return vm.throw_completion<JS::InternalError>(global_object, "Failed to get client");

    // Although we have an IClient, which is an CBaseClient, CBaseClient has multiple inheritance (IGameEventListener2,
    // IClient, and IClientMessageHandler), so we need to go back a vtable (-1)
    call_function_from_vtable_index<__attribute__((cdecl)) void (*)(const IClient*, const char*)>(
        client, 57, -1, name.as_string().string().characters());

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(PlayerPrototype::userid_getter)
{
    auto* this_entity = TRY(EntityPrototype::ensure_this_entity(vm, global_object));

    auto* client = Plugin::the().engine_server().GetIServer()->GetClient(
        this_entity->entity()->GetRefEHandle().GetEntryIndex() - 1);

    if (!client || !client->IsConnected())
        return vm.throw_completion<JS::InternalError>(global_object, "Failed to get client");

    return client->GetUserID();
}
}