#pragma once

#include <AK/Assertions.h>

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