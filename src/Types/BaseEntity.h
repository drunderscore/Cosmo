#pragma once

#include <basetypes.h> // string_t.h needs this but doesn't include it
#include <iserverentity.h>

#include "../RemoveSourceSpecifics.h"

#include "../Helpers.h"
#include "../Types.h"
#include <AK/String.h>

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

    inline ServerClass* GetServerClass() const
    {
        return Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) ServerClass* (*)(const CBaseEntity*)>(
            this, 10)(this);
    }

    inline datamap_t* GetDataDescMap() const
    {
        return Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) datamap_t* (*)(const CBaseEntity*)>(
            this, 12)(this);
    }

    inline const char* GetClassname()
    {
        return get_value_by_type_description<string_t>(
                   *Cosmo::find_type_description_from_datamap_by_name_including_base(*GetDataDescMap(), "m_iClassname"))
            ->ToCStr();
    }

    inline Cosmo::SourceVector& GetAbsOrigin()
    {
        return *get_value_by_type_description<Cosmo::SourceVector>(
            *Cosmo::find_type_description_from_datamap_by_name_including_base(*GetDataDescMap(), "m_vecAbsOrigin"));
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

    template<typename T>
    inline T* get_value_by_send_property(SendProp& send_prop)
    {
        return reinterpret_cast<T*>(reinterpret_cast<FlatPtr>(this) + send_prop.GetOffset());
    }

    inline void ChangeTeam(int team)
    {
        Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) void (*)(const CBaseEntity*, int)>(this, 95)(this,
                                                                                                                  team);
    }

    inline int GetTeamNumber()
    {
        return *get_value_by_type_description<int>(
            *Cosmo::find_type_description_from_datamap_by_name_including_base(*GetDataDescMap(), "m_iTeamNum"));
    }
};
