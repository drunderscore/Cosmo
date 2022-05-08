#include "../Cosmo.h"

#include "../RemoveSourceSpecifics.h"

#include "Entity.h"
#include "EntityPrototype.h"
#include "GlobalObject.h"

namespace Cosmo::Scripting
{
Entity::Entity(GlobalObject& global_object, CBaseEntity* entity)
    : Object(global_object.entity_prototype()), m_entity(entity)
{
}

Entity* Entity::create(GlobalObject& global_object, CBaseEntity* entity)
{
    return global_object.heap().allocate<Entity>(global_object, global_object, entity);
}

void Entity::initialize(JS::GlobalObject& global_object) { Object::initialize(global_object); }
}