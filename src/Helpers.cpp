#include "Helpers.h"

namespace Cosmo
{
Optional<typedescription_t&> find_type_description_from_datamap_by_name(datamap_t& datamap, StringView field_name)
{
    for (auto i = 0; i < datamap.dataNumFields; i++)
    {
        if (auto& type_description = datamap.dataDesc[i]; type_description.fieldName == field_name)
            return type_description;
    }

    return {};
}

Optional<typedescription_t&> find_type_description_from_datamap_by_name_including_base(datamap_t& datamap,
                                                                                       StringView field_name)
{
    if (auto maybe_type_description = find_type_description_from_datamap_by_name(datamap, field_name);
        maybe_type_description.has_value())
        return maybe_type_description;

    if (datamap.baseMap)
        return find_type_description_from_datamap_by_name_including_base(*datamap.baseMap, field_name);

    return {};
}
}