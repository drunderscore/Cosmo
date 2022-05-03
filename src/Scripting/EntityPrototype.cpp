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
    define_native_accessor("model", model_getter, {}, 0);
    define_native_accessor("index", index_getter, {}, 0);
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::model_getter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    return JS::js_string(vm, this_entity->entity()->GetModelName().ToCStr());
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::index_getter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    return this_entity->entity()->GetRefEHandle().GetEntryIndex();
}
}