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

    template<typename T>
    inline T* get_value_by_type_description(typedescription_t& type_description)
    {
        return reinterpret_cast<T*>(reinterpret_cast<FlatPtr>(this) + type_description.fieldOffset[TD_OFFSET_NORMAL]);
    }
};
