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

Entity* Entity::create(JS::Realm& realm, CBaseEntity* entity)
{
    // FIXME: These prototypes should probably not be stored with the global object anymore, but rather
    //        some other place like JS::Runtime::Intrinsics.
    //        For now, we'll cheat and keep grabbing them from our global object.

    auto& global_object = verify_cast<GlobalObject>(realm.global_object());

    if (entity->GetClassname() == "team_round_timer"sv)
        return realm.heap().allocate<Entity>(realm, global_object.team_round_timer_prototype(), entity);

    if (entity->GetClassname() == "player"sv)
        return realm.heap().allocate<Entity>(realm, global_object.player_prototype(), entity);

    return realm.heap().allocate<Entity>(realm, global_object.entity_prototype(), entity);
}

bool Entity::is_valid() { return entity(); }

void Entity::initialize(JS::Realm& realm) { Base::initialize(realm); }
}