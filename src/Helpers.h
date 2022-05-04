#pragma once

#include <AK/Types.h>

namespace Cosmo
{
template<typename Function, typename T>
inline Function get_function_from_vtable_index(const T* object_base_address, int index)
{
    auto** vtable = *reinterpret_cast<Function**>(const_cast<T*>(object_base_address));
    return vtable[index];
}
}