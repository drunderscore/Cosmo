#include "TeamRoundTimerPrototype.h"

#include "EntityPrototype.h"
#include "Game.h"
#include "GlobalObject.h"

namespace Cosmo::Scripting
{
void GlobalObject::initialize_global_object()
{
    Base::initialize_global_object();

    m_entity_prototype = heap().allocate<EntityPrototype>(*this, *this);
    m_team_round_timer_prototype = heap().allocate<TeamRoundTimerPrototype>(*this, *this);

    define_direct_property("Game", m_game_object = heap().allocate<Game>(*this, *this), 0);
}

void GlobalObject::visit_edges(Visitor& visitor)
{
    Base::visit_edges(visitor);

    visitor.visit(m_entity_prototype);
    visitor.visit(m_team_round_timer_prototype);
}
}