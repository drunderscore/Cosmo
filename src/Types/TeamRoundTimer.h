#pragma once

#include "BaseEntity.h"

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
};