#include "Signature.h"

#include <AK/String.h>
#include <AK/StringUtils.h>
#include <ctype.h>
#include <link.h>
#include <sys/mman.h>
#include <sys/stat.h>

namespace Cosmo
{
Signature::Signature(StringView signature)
{
    // -1, because we don't want to get to the VERY last character (we want groups of two)
    for (auto i = 0; i < signature.length() - 1; i++)
    {
        auto c = signature[i];

        // Spaces are insignificant
        if (isspace(c))
            continue;

        if (c == '?')
        {
            m_values.append({});
            continue;
        }

        m_values.append(*AK::StringUtils::convert_to_uint_from_hex(signature.substring_view(i, 2)));
        i++; // Skip two characters at a time
    }
}

Optional<void*> Signature::find_in_library(const char* library_name)
{
    auto library_bytes = get_bytes_for_library_name(library_name);
    for (auto i = 0; i < library_bytes.size() - m_values.size(); i++)
    {
        bool failed = false;

        for (auto j = 0; j < m_values.size(); j++)
        {
            auto& value = m_values[j];
            if (!value.has_value())
                continue;

            if (*(library_bytes.data() + i + j) != *value)
            {
                failed = true;
                break;
            }
        }

        if (!failed)
            return static_cast<void*>(library_bytes.data() + i);
    }

    return {};
}

Bytes Signature::get_bytes_for_library_name(const char* library_name)
{
    // dlopen actually returns a link_map*
    auto* dynamic_library = static_cast<link_map*>(dlopen(library_name, RTLD_NOW));
    VERIFY(dynamic_library);

    // We have the base address of the library, but we need the size. stat-ing the l_name for the size should be fine.
    // If we pass nullptr to dlopen, it gives us ourselves. However, that doesn't give us a path that we can stat.
    // We can use "/proc/self/exe", which is just a symlink to the executable. Really nifty!

    const char* path_to_binary = library_name ? dynamic_library->l_name : "/proc/self/exe";
    struct stat dynamic_library_stat = {};
    VERIFY(stat(path_to_binary, &dynamic_library_stat) != -1);

    // FIXME: Why do we mprotect here? Shouldn't we do it somewhere else (at least to add PROT_WRITE and PROT_EXEC?)
    VERIFY(mprotect(reinterpret_cast<void*>(dynamic_library->l_addr), dynamic_library_stat.st_size,
                    PROT_READ | PROT_WRITE | PROT_EXEC) == 0);

    return {reinterpret_cast<uint8_t*>(dynamic_library->l_addr), static_cast<size_t>(dynamic_library_stat.st_size)};
}
}