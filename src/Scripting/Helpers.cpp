#include "../Cosmo.h"

#include "../RemoveSourceSpecifics.h"

#include "../Types.h"
#include "Entity.h"
#include "Helpers.h"
#include <LibJS/Runtime/Array.h>

namespace Cosmo::Scripting
{
JS::ThrowCompletionOr<RecipientFilter> to_recipient_filter(JS::VM& vm, JS::GlobalObject& global_object, JS::Value value)
{
    // If you pass "true" to a RecipientFilter, let this mean broadcast.
    if (value.is_boolean() && value.as_bool())
        return RecipientFilter::all();

    if (TRY(value.is_array(global_object)))
    {
        auto& value_array = value.as_array();
        RecipientFilter filter;

        for (auto it = value_array.indexed_properties().begin(); it != value_array.indexed_properties().end(); ++it)
        {
            auto recipient_value = TRY(value_array.get(it.index()));

            // FIXME: This should be is<Player> when we have that type!
            if (recipient_value.is_object() && is<Entity>(recipient_value.as_object()))
            {
                filter.add_recipient(
                    static_cast<Entity&>(recipient_value.as_object()).entity()->GetRefEHandle().GetEntryIndex());
            }
            else if (recipient_value.is_number())
            {
                filter.add_recipient(recipient_value.as_i32());
            }
            else
            {
                return vm.throw_completion<JS::TypeError>(global_object,
                                                          String::formatted("{} is not a number", recipient_value));
            }
        }

        return filter;
    }

    return vm.throw_completion<JS::TypeError>(global_object,
                                              String::formatted("Cannot create a recipient filter from {}", value));
}
}