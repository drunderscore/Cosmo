#pragma once

#include "Forward.h"
#include <AK/Assertions.h>
#include <AK/Badge.h>
#include <AK/HashMap.h>
#include <AK/String.h>
#include <LibJS/Runtime/FunctionObject.h>
#include <LibJS/Runtime/Object.h>

namespace Cosmo::Scripting
{
class Game final : public JS::Object
{
    JS_OBJECT(Game, Object);

public:
    explicit Game(GlobalObject&);
    void initialize(JS::GlobalObject&) override;
    ~Game() override = default;

    Command& command_object() { return *m_command_object; }

private:
    JS_DECLARE_NATIVE_FUNCTION(map_getter);
    JS_DECLARE_NATIVE_FUNCTION(map_setter);
    JS_DECLARE_NATIVE_FUNCTION(mod_description_getter);
    JS_DECLARE_NATIVE_FUNCTION(say_text_2);

    Command* m_command_object{};
};
}