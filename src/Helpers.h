#pragma once

#include <datamap.h>

#include "RemoveSourceSpecifics.h"

#include <AK/StringView.h>
#include <AK/Types.h>

namespace Cosmo
{
template<typename Function, typename T>
inline Function get_function_from_vtable_index(const T* object_base_address, int index)
{
    auto** vtable = *reinterpret_cast<Function**>(const_cast<T*>(object_base_address));
    return vtable[index];
}

Optional<typedescription_t&> find_type_description_from_datamap_by_name(datamap_t&, StringView field_name);
Optional<typedescription_t&> find_type_description_from_datamap_by_name_including_base(datamap_t&,
                                                                                       StringView field_name);
}