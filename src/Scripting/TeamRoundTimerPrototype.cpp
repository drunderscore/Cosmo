#include "../Types/TeamRoundTimer.h"

#include "EntityPrototype.h"
#include "GlobalObject.h"
#include "TeamRoundTimerPrototype.h"

namespace Cosmo::Scripting
{
TeamRoundTimerPrototype::TeamRoundTimerPrototype(GlobalObject& global_object)
    : PrototypeObject(global_object.entity_prototype())
{
}

void TeamRoundTimerPrototype::initialize(JS::GlobalObject& global_object)
{
    Prototype::initialize(global_object);
    define_native_accessor("timeRemaining", time_remaining_getter, time_remaining_setter, 0);
    define_native_accessor("paused", paused_getter, paused_setter, 0);

    define_native_function("addSeconds", add_seconds, 2, 0);
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::time_remaining_getter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    return this_entity->entity<CTeamRoundTimer>()->GetTimeRemaining();
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::time_remaining_setter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    auto time_remaining = vm.argument(0);
    if (!time_remaining.is_number())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, time_remaining, "number");

    this_entity->entity<CTeamRoundTimer>()->SetTimeRemaining(time_remaining.as_i32());

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::add_seconds)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    auto seconds_to_add = vm.argument(0);
    if (!seconds_to_add.is_number())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, seconds_to_add, "number");

    int team = 0;

    if (auto team_argument = vm.argument(1); team_argument.is_number())
        team = team_argument.as_i32();

    this_entity->entity<CTeamRoundTimer>()->AddTimerSeconds(seconds_to_add.as_i32(), team);

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::paused_getter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    return this_entity->entity<CTeamRoundTimer>()->IsTimerPaused();
}

JS_DEFINE_NATIVE_FUNCTION(TeamRoundTimerPrototype::paused_setter)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    auto is_paused = vm.argument(0);
    if (!is_paused.is_boolean())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, is_paused, "boolean");

    if (is_paused.as_bool())
        this_entity->entity<CTeamRoundTimer>()->PauseTimer();
    else
        this_entity->entity<CTeamRoundTimer>()->ResumeTimer();

    return JS::js_undefined();
}
}