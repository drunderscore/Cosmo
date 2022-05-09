#pragma once

#include "../Forward.h"
#include "Forward.h"
#include <LibJS/Runtime/Object.h>

namespace Cosmo::Scripting
{
class Entity : public JS::Object
{
    JS_OBJECT(Entity, Object);

public:
    Entity(Object&, CBaseEntity*);
    void initialize(JS::GlobalObject&) override;
    ~Entity() override = default;

    static Entity* create(GlobalObject& global_object, CBaseEntity* entity);

    // clang-format off
    template<typename T = CBaseEntity>
    T* entity() requires(IsBaseOf<CBaseEntity, T>)
    {
        return static_cast<T*>(m_entity);
    }
    // clang-format on

private:
    // FIXME: This is very much not correct! Once this entity is destroyed, this pointer is garbage.
    //        This is a UAF in script.
    //        How can we use EHANDLE/CHandle here? Or our own method of knowing an entities lifetime?
    CBaseEntity* m_entity{};
};
}