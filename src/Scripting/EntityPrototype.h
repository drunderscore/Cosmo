#pragma once

#include "Entity.h"
#include <LibJS/Runtime/PrototypeObject.h>

namespace Cosmo::Scripting
{
class EntityPrototype final : public JS::PrototypeObject<EntityPrototype, Entity>
{
    JS_PROTOTYPE_OBJECT(EntityPrototype, Entity, Entity);

public:
    explicit EntityPrototype(JS::GlobalObject&);
    virtual void initialize(JS::GlobalObject&) override;
    virtual ~EntityPrototype() override = default;

    static JS::ThrowCompletionOr<Entity*> ensure_this_entity(JS::VM& vm, JS::GlobalObject& global_object);

private:
    JS_DECLARE_NATIVE_FUNCTION(model_getter);
    JS_DECLARE_NATIVE_FUNCTION(model_setter);
    JS_DECLARE_NATIVE_FUNCTION(classname_getter);
    JS_DECLARE_NATIVE_FUNCTION(index_getter);
    JS_DECLARE_NATIVE_FUNCTION(position_getter);
    JS_DECLARE_NATIVE_FUNCTION(position_setter);
    JS_DECLARE_NATIVE_FUNCTION(team_getter);
    JS_DECLARE_NATIVE_FUNCTION(team_setter);

    JS_DECLARE_NATIVE_FUNCTION(is_player);
    JS_DECLARE_NATIVE_FUNCTION(is_valid);

    JS_DECLARE_NATIVE_FUNCTION(dispatch_spawn);
    JS_DECLARE_NATIVE_FUNCTION(teleport);
    JS_DECLARE_NATIVE_FUNCTION(remove);
    JS_DECLARE_NATIVE_FUNCTION(emit_sound);
};
}