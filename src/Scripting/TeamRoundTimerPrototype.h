#pragma once

#include "Entity.h"

#include "../RemoveSourceSpecifics.h"

#include <LibJS/Runtime/PrototypeObject.h>

namespace Cosmo::Scripting
{
class TeamRoundTimerPrototype final : public JS::PrototypeObject<TeamRoundTimerPrototype, Entity>
{
    JS_PROTOTYPE_OBJECT(TeamRoundTimerPrototype, Entity, TeamRoundTimer);

public:
    explicit TeamRoundTimerPrototype(GlobalObject&);
    virtual void initialize(JS::GlobalObject&) override;
    virtual ~TeamRoundTimerPrototype() override = default;

private:
    JS_DECLARE_NATIVE_FUNCTION(time_remaining_getter);
    JS_DECLARE_NATIVE_FUNCTION(time_remaining_setter);
    JS_DECLARE_NATIVE_FUNCTION(paused_getter);
    JS_DECLARE_NATIVE_FUNCTION(paused_setter);

    JS_DECLARE_NATIVE_FUNCTION(add_seconds);
};
}