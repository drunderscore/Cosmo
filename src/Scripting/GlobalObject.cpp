#include "TeamRoundTimerPrototype.h"

#include "EntityPrototype.h"
#include "Game.h"
#include "GlobalObject.h"
#include "PlayerPrototype.h"

namespace Cosmo::Scripting
{
GlobalObject::GlobalObject(JS::Realm& realm) : JS::GlobalObject(realm) {}

void GlobalObject::initialize(JS::Realm& realm)
{
    Base::initialize(realm);

    m_entity_prototype = heap().allocate<EntityPrototype>(realm, realm);
    m_team_round_timer_prototype = heap().allocate<TeamRoundTimerPrototype>(realm, realm);
    m_player_prototype = heap().allocate<PlayerPrototype>(realm, realm);

    define_direct_property("Game", m_game_object = heap().allocate<Game>(realm, realm), 0);
}

void GlobalObject::visit_edges(Visitor& visitor)
{
    Base::visit_edges(visitor);

    visitor.visit(m_entity_prototype);
    visitor.visit(m_team_round_timer_prototype);
    visitor.visit(m_player_prototype);
    visitor.visit(m_game_object);
}
}