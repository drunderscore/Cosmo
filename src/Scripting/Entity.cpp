#include "../Types/BaseEntity.h"

#include "Entity.h"
#include "EntityPrototype.h"
#include "GlobalObject.h"
#include "PlayerPrototype.h"
#include "TeamRoundTimerPrototype.h"

namespace Cosmo::Scripting
{
Entity::Entity(Object& prototype, CBaseEntity* entity) : Entity(prototype, entity->GetRefEHandle()) {}
Entity::Entity(Object& prototype, const CBaseHandle& handle) : Object(prototype), m_handle(handle) {}

Entity* Entity::create(GlobalObject& global_object, CBaseEntity* entity)
{
    if (entity->GetClassname() == "team_round_timer"sv)
        return global_object.heap().allocate<Entity>(global_object, global_object.team_round_timer_prototype(), entity);

    if (entity->GetClassname() == "player"sv)
        return global_object.heap().allocate<Entity>(global_object, global_object.player_prototype(), entity);

    return global_object.heap().allocate<Entity>(global_object, global_object.entity_prototype(), entity);
}

bool Entity::is_valid()
{
    return entity();
}

void Entity::initialize(JS::GlobalObject& global_object) { Object::initialize(global_object); }
}