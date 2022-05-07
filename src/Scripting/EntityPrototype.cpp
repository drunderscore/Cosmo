#include "../Cosmo.h"
#include "../Types.h"

#include "../RemoveSourceSpecifics.h"

#include "EntityPrototype.h"
#include "Helpers.h"

namespace Cosmo::Scripting
{
EntityPrototype::EntityPrototype(JS::GlobalObject& global_object) : PrototypeObject(*global_object.object_prototype())
{
}

void EntityPrototype::initialize(JS::GlobalObject& global_object)
{
    Object::initialize(global_object);
    define_native_accessor("model", model_getter, model_setter, 0);
    define_native_accessor("index", index_getter, {}, 0);
    define_native_accessor("isPlayer", is_player, {}, 0);
    define_native_accessor("classname", classname_getter, {}, 0);
    define_native_accessor("position", position_getter, position_setter, 0);

    define_native_function("teleport", teleport, 3, 0);
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::model_getter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    return JS::js_string(vm, this_entity->entity()->GetModelName().ToCStr());
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::model_setter)
{
    auto* this_entity = TRY(typed_this_value(global_object));

    auto model_name = vm.argument(0);
    if (!model_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, model_name);

    if (!Plugin::the().engine_server().IsModelPrecached(model_name.as_string().string().characters()))
        Plugin::the().engine_server().PrecacheModel(model_name.as_string().string().characters());

    this_entity->entity()->SetModel(model_name.as_string().string());

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::index_getter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    return this_entity->entity()->GetRefEHandle().GetEntryIndex();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::is_player)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    return this_entity->entity()->IsPlayer();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::classname_getter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    return JS::js_string(vm, this_entity->entity()->GetClassname());
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::position_getter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    return from_source_vector(global_object, this_entity->entity()->GetAbsOrigin());
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::position_setter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    auto vector = TRY(to_source_vector(vm, global_object, vm.argument(0)));

    this_entity->entity()->Teleport(&vector, nullptr, nullptr);

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::teleport)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    Optional<SourceVector> new_position;
    Optional<QAngle> new_angles;
    Optional<SourceVector> new_velocity;

    if (auto new_position_argument = vm.argument(0); !new_position_argument.is_nullish())
        new_position = TRY(to_source_vector(vm, global_object, new_position_argument));

    if (auto new_angles_argument = vm.argument(1); !new_angles_argument.is_nullish())
        new_angles = TRY(to_qangle(vm, global_object, new_angles_argument));

    if (auto new_velocity_argument = vm.argument(2); !new_velocity_argument.is_nullish())
        new_velocity = TRY(to_source_vector(vm, global_object, new_velocity_argument));

    this_entity->entity()->Teleport(new_position, new_angles, new_velocity);

    return JS::js_undefined();
}
}