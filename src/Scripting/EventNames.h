#pragma once

#include <AK/StringView.h>

#define ENUMERATE_COSMO_EVENT_NAMES(M)                                                                                 \
    M("PlayerConnect"sv, player_connect)                                                                               \
    M("PlayerDisconnect"sv, player_disconnect)                                                                         \
    M("PlayerActive"sv, player_active)                                                                                 \
    M("EntityCreate"sv, entity_create)                                                                                 \
    M("EntitySpawn"sv, entity_spawn)                                                                                   \
    M("GameEvent"sv, game_event)

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