#include "../Types/TeamRoundTimer.h"

#include "EntityPrototype.h"
#include "GlobalObject.h"
#include "TeamRoundTimerPrototype.h"

namespace Cosmo::Scripting
{
TeamRoundTimerPrototype::TeamRoundTimerPrototype(JS::Realm& realm)
    : PrototypeObject(verify_cast<GlobalObject>(realm.global_object()).entity_prototype())
{
}

void TeamRoundTimerPrototype::initialize(JS::Realm& realm)
{
    Base::initialize(realm);
    define_native_accessor(realm, "timeRemaining", time_remaining_getter, time_remaining_setter, 0);
    define_native_accessor(realm, "paused", paused_getter, paused_setter, 0);

    define_native_function(realm, "addSeconds", add_seconds, 2, 0);
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::time_remaining_getter)
{
    auto* this_entity = TRY(EntityPrototype::ensure_this_entity(vm));

    return this_entity->entity<CTeamRoundTimer>()->GetTimeRemaining();
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::time_remaining_setter)
{
    auto* this_entity = TRY(EntityPrototype::ensure_this_entity(vm));

    auto time_remaining = vm.argument(0);
    if (!time_remaining.is_number())
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::IsNotA, time_remaining, "number");

    this_entity->entity<CTeamRoundTimer>()->SetTimeRemaining(TRY(time_remaining.to_i32(vm)));

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::add_seconds)
{
    auto* this_entity = TRY(EntityPrototype::ensure_this_entity(vm));

    auto seconds_to_add = vm.argument(0);
    if (!seconds_to_add.is_number())
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::IsNotA, seconds_to_add, "number");

    int team = 0;

    if (auto team_argument = vm.argument(1); team_argument.is_number())
        team = TRY(team_argument.to_i32(vm));

    this_entity->entity<CTeamRoundTimer>()->AddTimerSeconds(TRY(seconds_to_add.to_i32(vm)), team);

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::paused_getter)
{
    auto* this_entity = TRY(EntityPrototype::ensure_this_entity(vm));

    return this_entity->entity<CTeamRoundTimer>()->IsTimerPaused();
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::paused_setter)
{
    auto* this_entity = TRY(EntityPrototype::ensure_this_entity(vm));

    auto is_paused = vm.argument(0);
    if (!is_paused.is_boolean())
        return vm.throw_completion<JS::TypeError>(JS::ErrorType::IsNotA, is_paused, "boolean");

    if (is_paused.as_bool())
        this_entity->entity<CTeamRoundTimer>()->PauseTimer();
    else
        this_entity->entity<CTeamRoundTimer>()->ResumeTimer();

    return JS::js_undefined();
}
}