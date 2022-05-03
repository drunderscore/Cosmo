#pragma once

#include <AK/Types.h>

namespace Cosmo
{
template<typename Function, typename T>
inline Function get_function_from_vtable_index(const T* object_base_address, int index)
{
    auto* vtable = *reinterpret_cast<FlatPtr**>(const_cast<T*>(object_base_address));

    return reinterpret_cast<Function>(vtable[index]);
}
}