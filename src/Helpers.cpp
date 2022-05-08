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

Optional<SendProp&> find_send_property_from_send_table(SendTable& send_table, StringView property_name)
{
    for (auto i = 0; i < send_table.GetNumProps(); i++)
    {
        if (auto& property = *send_table.GetProp(i); property.GetName() == property_name)
            return property;
    }

    return {};
}

Optional<SendProp&> find_send_property_from_send_table_including_base(SendTable& send_table, StringView property_name)
{
    if (auto maybe_property = find_send_property_from_send_table(send_table, property_name); maybe_property.has_value())
        return maybe_property;

    // Assume a SendTable with a property named "baseclass" is a DPT_DataTable, and has a valid SendTable*
    if (auto base_send_table = find_send_property_from_send_table(send_table, "baseclass"); base_send_table.has_value())
        return find_send_property_from_send_table_including_base(*base_send_table->GetDataTable(), property_name);

    return {};
}
}