#pragma once

#include <iserverentity.h>

#include "RemoveSourceSpecifics.h"

#include "Helpers.h"

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
};
