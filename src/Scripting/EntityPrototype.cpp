#include "../Cosmo.h"
#include "../Types.h"

#include "../RemoveSourceSpecifics.h"

#include "EntityPrototype.h"

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
}