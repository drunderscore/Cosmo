#pragma once

#include "BaseEntity.h"

#include <server_class.h>

class CTeamRoundTimer : public CBaseEntity
{
public:
    inline float GetTimeRemaining()
    {
        return Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) float (*)(const CBaseEntity*)>(this,
                                                                                                           196)(this);
    }

    inline void SetTimeRemaining(int value)
    {
        Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) void (*)(const CBaseEntity*, int)>(this, 200)(
            this, value);
    }

    inline void AddTimerSeconds(int seconds, int team)
    {
        Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) void (*)(const CBaseEntity*, int, int)>(this, 201)(
            this, seconds, team);
    }

    inline void PauseTimer()
    {
        Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) void (*)(const CBaseEntity*)>(this, 202)(this);
    }

    inline void ResumeTimer()
    {
        Cosmo::get_function_from_vtable_index<__attribute__((cdecl)) void (*)(const CBaseEntity*)>(this, 203)(this);
    }

    inline bool IsTimerPaused()
    {
        return *get_value_by_send_property<bool>(
            *Cosmo::find_send_property_from_send_table_including_base(*GetServerClass()->m_pTable, "m_bTimerPaused"));
    }
};