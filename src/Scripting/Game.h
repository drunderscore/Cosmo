#pragma once

#include "Forward.h"
#include <AK/HashMap.h>
#include <AK/String.h>
#include <LibJS/Runtime/AbstractOperations.h>
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

    template<typename... Args>
    void dispatch_event(StringView event_name, Args&&... args)
    {
        auto maybe_event_handlers = m_event_handlers.get(event_name);
        if (!maybe_event_handlers.has_value())
            return;

        for (auto* callback : *maybe_event_handlers)
        {
            // FIXME: Should the this_value here by myself?
            auto maybe_return_value = JS::call(global_object(), callback, JS::js_undefined(), args...);
            if (maybe_return_value.is_error())
            {
                Warning("Event handler for event %s threw an exception: %s\n",
                        event_name.characters_without_null_termination(),
                        MUST(maybe_return_value.throw_completion().value()->to_string(global_object())).characters());
            }
        }
    }

private:
    JS_DECLARE_NATIVE_FUNCTION(mod_description_getter);
    JS_DECLARE_NATIVE_FUNCTION(on);

    Command* m_command_object{};
    HashMap<String, JS::MarkedVector<JS::FunctionObject*>> m_event_handlers;
};
}