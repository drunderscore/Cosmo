#include "../Cosmo.h"

#include "../RemoveSourceSpecifics.h"

#include "Entity.h"
#include "EntityPrototype.h"
#include "GlobalObject.h"

namespace Cosmo::Scripting
{
Entity::Entity(Object& prototype, CBaseEntity* entity) : Object(prototype), m_entity(entity) {}

Entity* Entity::create(GlobalObject& global_object, CBaseEntity* entity)
{
    return global_object.heap().allocate<Entity>(global_object, global_object.entity_prototype(), entity);
}

void Entity::initialize(JS::GlobalObject& global_object) { Object::initialize(global_object); }
}