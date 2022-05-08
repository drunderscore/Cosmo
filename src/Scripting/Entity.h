#pragma once

#include "../RemoveSourceSpecifics.h"

#include "../Forward.h"
#include "Forward.h"
#include <LibJS/Runtime/Object.h>

namespace Cosmo::Scripting
{
class Entity final : public JS::Object
{
    JS_OBJECT(Entity, Object);

public:
    Entity(GlobalObject&, CBaseEntity*);
    void initialize(JS::GlobalObject&) override;
    ~Entity() override = default;

    static Entity* create(GlobalObject& global_object, CBaseEntity* entity)
    {
        return global_object.heap().allocate<Entity>(global_object, global_object, entity);
    }

    CBaseEntity* entity() { return m_entity; }

private:
    // FIXME: This is very much not correct! Once this entity is destroyed, this pointer is garbage.
    //        This is a UAF in script.
    //        How can we use EHANDLE/CHandle here? Or our own method of knowing an entities lifetime?
    CBaseEntity* m_entity{};
};
}