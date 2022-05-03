#pragma once

#include "Forward.h"
#include <LibJS/Runtime/Object.h>

namespace Cosmo::Scripting
{
class Command final : public JS::Object
{
    JS_OBJECT(Command, Object);

public:
    struct ConCommandWithCallbackFunction
    {
        NonnullOwnPtr<ConCommand> command;
        JS::FunctionObject* callback;
    };

    explicit Command(GlobalObject&);
    void initialize(JS::GlobalObject&) override;
    ~Command() override = default;

    JS::ThrowCompletionOr<JS::Value> internal_get(const JS::PropertyKey&, JS::Value receiver) const override;
    JS::ThrowCompletionOr<bool> internal_set(const JS::PropertyKey&, JS::Value value, JS::Value receiver) override;
    // FIXME: We should also override "internal_own_property_keys", so the keys (ConVar names) can be enumerated.
    //        However, enumeration requires that the property has a descriptor, and is configured to be enumerable.
    //        We override "internal_get" to directly give a JS::Value from a name, so there isn't even a descriptor.

    HashMap<String, ConCommandWithCallbackFunction, CaseInsensitiveStringTraits>& commands() { return m_commands; }

private:
    void visit_edges(Cell::Visitor&) override;

    static void on_script_concommand_execute(const CCommand& args);

    // "register" is a C keyword... pretty unfortunate
    JS_DECLARE_NATIVE_FUNCTION(register_);
    JS_DECLARE_NATIVE_FUNCTION(execute);

    HashMap<String, ConCommandWithCallbackFunction, CaseInsensitiveStringTraits> m_commands;
};
}