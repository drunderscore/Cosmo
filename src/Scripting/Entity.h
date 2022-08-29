#pragma once

#include "../Forward.h"
#include "Forward.h"
#include <LibJS/Runtime/Object.h>
#include <basehandle.h>

namespace Cosmo::Scripting
{
class Entity : public JS::Object
{
    JS_OBJECT(Entity, Object);

public:
    Entity(Object&, CBaseEntity*);
    Entity(Object&, const CBaseHandle&);
    void initialize(JS::Realm&) override;
    ~Entity() override = default;

    static Entity* create(JS::Realm&, CBaseEntity*);

    bool is_valid();

    const CBaseHandle& handle() const { return m_handle; }

    // clang-format off
    template<typename T = CBaseEntity>
    T* entity() requires(IsBaseOf<CBaseEntity, T>)
    {
        return static_cast<T*>(m_handle.Get());
    }

private:
    // clang-format on
    CBaseHandle m_handle;
};
}