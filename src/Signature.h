#pragma once

#include <AK/Optional.h>
#include <AK/StringView.h>
#include <AK/Vector.h>

namespace Cosmo
{
class Signature
{
public:
    explicit Signature(StringView signature);

    Optional<void*> find_in_library(const char* library_name);

    const AK::Vector<Optional<u8>>& values() const { return m_values; }

private:
    static Bytes get_bytes_for_library_name(const char* library_name);

    Vector<Optional<u8>> m_values;
};
}