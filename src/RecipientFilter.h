#pragma once

#include <irecipientfilter.h>

#include "RemoveSourceSpecifics.h"

#include <AK/Vector.h>

namespace Cosmo
{
class RecipientFilter : public IRecipientFilter
{
public:
    RecipientFilter() = default;
    RecipientFilter(std::initializer_list<int> recipients) : m_recipients(recipients) {}
    ~RecipientFilter() override = default;

    bool IsReliable() const override { return m_reliable; }
    bool IsInitMessage() const override { return false; }

    int GetRecipientCount() const override { return m_recipients.size(); }
    int GetRecipientIndex(int index) const override { return m_recipients.at(index); }

    void set_reliable(bool value) { m_reliable = value; }
    void add_recipient(int value) { m_recipients.append(value); }
    void remove_recipient(int value)
    {
        m_recipients.remove_all_matching([value](auto existing_value) { return existing_value == value; });
    }

    void remove_recipient_index(int index) { m_recipients.remove(index); }

    static RecipientFilter all()
    {
        RecipientFilter filter;
        for (auto i = 1; i <= g_SMAPI->GetCGlobals()->maxClients; i++)
        {
            if (auto edict = Plugin::the().engine_server().PEntityOfEntIndex(i);
                edict && Plugin::the().server_game_ents().EdictToBaseEntity(edict))
                filter.add_recipient(i);
        }
        return filter;
    }

private:
    AK::Vector<int> m_recipients;
    // Let's just always be reliable by default
    bool m_reliable{true};
};
}