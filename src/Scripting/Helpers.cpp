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

            CBaseEntity* entity{};

            if (recipient_value.is_object() && is<Entity>(recipient_value.as_object()))
                entity = static_cast<Entity&>(recipient_value.as_object()).entity();
            else if (recipient_value.is_number())
                // TODO: verify it's okay to call PEntityOfEntIndex on null edict? what about range checking entity
                //       index, must we do that too?
                entity = Plugin::the().server_game_ents().EdictToBaseEntity(
                    Plugin::the().engine_server().PEntityOfEntIndex(recipient_value.as_i32()));

            if (!entity)
                return vm.throw_completion<JS::TypeError>(global_object,
                                                          String::formatted("{} is not an entity", recipient_value));

            if (!entity->IsPlayer())
                return vm.throw_completion<JS::TypeError>(
                    global_object, String::formatted("{} is not a player entity", recipient_value));

            filter.add_recipient(entity->GetRefEHandle().GetEntryIndex());
        }

        return filter;
    }

    return vm.throw_completion<JS::TypeError>(global_object,
                                              String::formatted("Cannot create a recipient filter from {}", value));
}
}