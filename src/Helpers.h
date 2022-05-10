#pragma once

#include <datamap.h>
#include <dt_send.h>

#include "RemoveSourceSpecifics.h"

#include <AK/StringView.h>
#include <AK/Types.h>

namespace Cosmo
{

// vtable_index_into_object is useful for when you have multiple-inheritance
template<typename Function, typename T>
inline Function get_function_from_vtable_index(const T* object_base_address, int function_index_into_vtable,
                                               int vtable_index_into_object = 0)
{
    auto** vtable = reinterpret_cast<Function**>(const_cast<T*>(object_base_address))[vtable_index_into_object];
    return vtable[function_index_into_vtable];
}

template<typename Function, typename T, typename... Args>
inline auto call_function_from_vtable_index(const T* object_base_address, int function_index_into_vtable,
                                            int vtable_index_into_object = 0, Args&&... args)
{
    return get_function_from_vtable_index<Function, T>(object_base_address, function_index_into_vtable,
                                                       vtable_index_into_object)(
        reinterpret_cast<T*>(reinterpret_cast<FlatPtr>(object_base_address) +
                             (sizeof(FlatPtr) * vtable_index_into_object)),
        move(args...));
}

Optional<typedescription_t&> find_type_description_from_datamap_by_name(datamap_t&, StringView field_name);
Optional<typedescription_t&> find_type_description_from_datamap_by_name_including_base(datamap_t&,
                                                                                       StringView field_name);
Optional<SendProp&> find_send_property_from_send_table(SendTable&, StringView property_name);
Optional<SendProp&> find_send_property_from_send_table_including_base(SendTable&, StringView property_name);
}