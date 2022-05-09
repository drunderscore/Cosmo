#pragma once

#include <AK/StringView.h>

#define ENUMERATE_COSMO_EVENT_NAMES(M)                                                                                 \
    M("PlayerConnect", player_connect)                                                                                 \
    M("PlayerDisconnect", player_disconnect)                                                                           \
    M("PlayerActive", player_active)                                                                                   \
    M("EntityCreate", entity_create)                                                                                   \
    M("EntitySpawn", entity_spawn)

namespace Cosmo::Scripting
{
struct EventNames
{
#define COSMO_EVENT_ENUMERATION_MACRO(camel_case_string, snake_case_name)                                              \
    static constexpr StringView snake_case_name = camel_case_string;

    ENUMERATE_COSMO_EVENT_NAMES(COSMO_EVENT_ENUMERATION_MACRO);

#undef COSMO_EVENT_ENUMERATION_MACRO
};
}