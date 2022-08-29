#pragma once

#include "Entity.h"

#include "../RemoveSourceSpecifics.h"

#include <LibJS/Runtime/PrototypeObject.h>

namespace Cosmo::Scripting
{
class PlayerPrototype final : public JS::PrototypeObject<PlayerPrototype, Entity>
{
    JS_PROTOTYPE_OBJECT(PlayerPrototype, Entity, Player);

public:
    explicit PlayerPrototype(JS::Realm&);
    virtual void initialize(JS::Realm&) override;
    virtual ~PlayerPrototype() override = default;

private:
    JS_DECLARE_NATIVE_FUNCTION(name_getter);
    JS_DECLARE_NATIVE_FUNCTION(name_setter);
    JS_DECLARE_NATIVE_FUNCTION(userid_getter);
    JS_DECLARE_NATIVE_FUNCTION(disconnect);
};
}