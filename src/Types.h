#pragma once

#include <iserverentity.h>

#include "RemoveSourceSpecifics.h"

#include "Helpers.h"

namespace Cosmo
{
// AK::Vector is put into the global namespace, and so we are completely unable to use Source Engine's Vector from
// mathlib.
// The contents of it are super simple, so we can just define it like this.
class SourceVector
{
public:
    vec_t x{}, y{}, z{};

    vec_t& operator[](size_t index)
    {
        VERIFY(index >= 0 && index < 3);
        return reinterpret_cast<vec_t*>(this)[index];
    }
};
}

// CBaseEntity inherits IServerEntity, but it's impossible to say that through forward-declarations alone.
// Use this empty class to give a bit more info.
class CBaseEntity : public IServerEntity
{
public:
    inline bool IsPlayer() const
    {
        return Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) bool (*)(const CBaseEntity*)>(this,
                                                                                                          81)(this);
    }

    inline void SetModel(const String& model) const
    {
        Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) void (*)(const CBaseEntity*, const char*)>(
            this, 25)(this, model.characters());
    }

    inline datamap_t* GetDataDescMap() const
    {
        return Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) datamap_t* (*)(const CBaseEntity*)>(
            this, 12)(this);
    }

    inline const char* GetClassname()
    {
        return get_value_by_type_description<string_t>(
                   Cosmo::find_type_description_from_datamap_by_name_including_base(*GetDataDescMap(), "m_iClassname")
                       .value())
            ->ToCStr();
    }

    inline Cosmo::SourceVector& GetAbsOrigin()
    {
        return *get_value_by_type_description<Cosmo::SourceVector>(
            Cosmo::find_type_description_from_datamap_by_name_including_base(*GetDataDescMap(), "m_vecAbsOrigin")
                .value());
    }

    inline void Teleport(Cosmo::SourceVector* new_position, QAngle* new_angles, Cosmo::SourceVector* new_velocity)
    {
        Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) void (*)(const CBaseEntity*, Cosmo::SourceVector*,
                                                                              QAngle*, Cosmo::SourceVector*)>(
            this, 112)(this, new_position, new_angles, new_velocity);
    }

    inline void Teleport(Optional<Cosmo::SourceVector> new_position, Optional<QAngle> new_angles,
                         Optional<Cosmo::SourceVector> new_velocity)
    {
        Teleport(new_position.has_value() ? &*new_position : nullptr, new_angles.has_value() ? &*new_angles : nullptr,
                 new_velocity.has_value() ? &*new_velocity : nullptr);
    }

    template<typename T>
    inline T* get_value_by_type_description(typedescription_t& type_description)
    {
        return reinterpret_cast<T*>(reinterpret_cast<FlatPtr>(this) + type_description.fieldOffset[TD_OFFSET_NORMAL]);
    }

    inline void ChangeTeam(int team)
    {
        Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) void (*)(const CBaseEntity*, int)>(this, 95)(this,
                                                                                                                  team);
    }

    inline int GetTeamNumber()
    {
        return *get_value_by_type_description<int>(
            Cosmo::find_type_description_from_datamap_by_name_including_base(*GetDataDescMap(), "m_iTeamNum").value());
    }
};
