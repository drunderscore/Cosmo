#include "Cosmo.h"
#include <server_class.h>

#include "RemoveSourceSpecifics.h"

#include <AK/JsonArray.h>
#include <AK/JsonObject.h>
#include <LibCore/Stream.h>

namespace Cosmo
{
void find_and_add_children(HashMap<String, SendTable*>& send_tables, SendTable* send_table)
{
    for (auto i = 0; i < send_table->GetNumProps(); i++)
    {
        auto* prop = send_table->GetProp(i);
        if (prop->GetType() == DPT_DataTable)
        {
            auto* data_table = prop->GetDataTable();
            if (data_table)
            {
                send_tables.set(data_table->GetName(), data_table);
                find_and_add_children(send_tables, data_table);
            }
        }
    }
}

CON_COMMAND(cosmo_dump, "Dump server classes and data tables to dump.json")
{
    // Let's try to open the file first, so we can avoid doing useless work if we can't even output it.
    auto maybe_output_file = Core::Stream::File::open("dump.json"sv, Core::Stream::OpenMode::Write);
    if (maybe_output_file.is_error())
    {
        Warning("Failed to open dump.json for writing\n");
        return;
    }

    auto output_file = maybe_output_file.release_value();

    JsonObject output;
    HashMap<String, SendTable*> send_tables_found;

    JsonObject server_classes;
    auto* server_class = Plugin::the().server_game_dll().GetAllServerClasses();
    do
    {
        JsonObject server_class_json;
        server_class_json.set("ClassID", server_class->m_ClassID);
        server_class_json.set("InstanceBaselineIndex", server_class->m_InstanceBaselineIndex);

        auto* send_table = server_class->m_pTable;
        if (send_table)
        {
            server_class_json.set("DataTable", send_table->GetName());
            send_tables_found.set(send_table->GetName(), send_table);
            // Let's find all it's child DataTables now, instead of whilst we're iterating (or iterating a second time)
            find_and_add_children(send_tables_found, send_table);
        }

        server_classes.set(server_class->m_pNetworkName, move(server_class_json));
    } while ((server_class = server_class->m_pNext));

    JsonObject data_tables;
    for (auto& kv : send_tables_found)
    {
        JsonObject send_table;

        for (auto i = 0; i < kv.value->GetNumProps(); i++)
        {
            auto* prop = kv.value->GetProp(i);
            JsonObject send_table_property;

            send_table_property.set("Type", prop->GetType());
            send_table_property.set("Flags", prop->GetFlags());
            send_table_property.set("Offset", prop->GetOffset());
            if (prop->GetDataTable())
                send_table_property.set("DataTable", prop->GetDataTable()->GetName());

            send_table.set(prop->GetName(), move(send_table_property));
        }

        data_tables.set(kv.key, move(send_table));
    }

    output.set("ServerClasses", server_classes);
    output.set("DataTables", data_tables);
    if (output_file->write(output.to_string().bytes()).is_error())
    {
        Warning("Failed to write to dump.json\n");
        return;
    }

    Msg("Wrote dump to dump.json\n");
}
}