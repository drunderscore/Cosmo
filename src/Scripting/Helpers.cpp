#include "../Cosmo.h"
#include "../Types/BaseEntity.h"

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

JS::ThrowCompletionOr<SourceVector> to_source_vector(JS::VM& vm, JS::GlobalObject& global_object, JS::Value value,
                                                     int number_of_elements)
{
    if (!TRY(value.is_array(global_object)))
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotAn, value, "Array");

    SourceVector vector{};
    int number_of_elements_indexed = 0;
    auto& array = value.as_array();
    for (auto it = array.indexed_properties().begin(false); it != array.indexed_properties().end(); ++it)
    {
        if (number_of_elements_indexed == number_of_elements)
            return vm.throw_completion<JS::Error>(global_object, "Too many elements for vector");

        auto element_value = TRY(array.get(it.index()));
        if (!element_value.is_number())
            return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, element_value, "number");

        vector[number_of_elements_indexed] = static_cast<float>(element_value.as_double());
        number_of_elements_indexed++;
    }

    if (number_of_elements_indexed != number_of_elements)
        return vm.throw_completion<JS::Error>(global_object, "Not enough elements for vector");

    return vector;
}

JS::ThrowCompletionOr<QAngle> to_qangle(JS::VM& vm, JS::GlobalObject& global_object, JS::Value value,
                                        int number_of_elements)
{
    auto vector = TRY(to_source_vector(vm, global_object, value, number_of_elements));

    QAngle angle;
    for (auto i = 0; i < number_of_elements; i++)
        angle[i] = vector[i];

    return angle;
}

JS::Array* from_source_vector(JS::GlobalObject& global_object, SourceVector& vector)
{
    return JS::Array::create_from<vec_t>(global_object, {reinterpret_cast<vec_t*>(&vector), 3},
                                         [](const auto& value) { return JS::Value(value); });
}

JS::ThrowCompletionOr<void> emit_sound(JS::VM& vm, JS::GlobalObject& global_object, JS::Value sound_name,
                                       JS::Value filter_argument, JS::Value volume_argument, JS::Value pitch_argument,
                                       Variant<CBaseEntity*, Cosmo::SourceVector> sound_source)
{
    if (!sound_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, sound_name);

    auto filter = filter_argument.is_nullish() ? RecipientFilter::all()
                                               : TRY(to_recipient_filter(vm, global_object, filter_argument));

    auto volume = 1.0f;
    auto pitch = 100;

    if (volume_argument.is_number())
    {
        volume = static_cast<float>(volume_argument.as_double());
        if (volume < 0.0f || volume > 1.0f)
            return vm.throw_completion<JS::RangeError>(global_object,
                                                       String::formatted("Volume {} of is out of range", volume));
    }

    if (pitch_argument.is_number())
    {
        pitch = pitch_argument.as_i32();
        if (pitch <= 0 || pitch > 255)
            return vm.throw_completion<JS::RangeError>(global_object,
                                                       String::formatted("Pitch {} of is out of range", pitch));
    }

    Plugin::the().engine_sound().PrecacheSound(sound_name.as_string().string().characters());

    Cosmo::SourceVector position;
    int entity_index{};

    sound_source.visit(
        [&entity_index](CBaseEntity* entity_source) { entity_index = entity_source->GetRefEHandle().GetEntryIndex(); },
        [&position](Cosmo::SourceVector& position_source) { position = position_source; });

    // pDirection and bUpdatePositions are NOT supported on the server and will assert (must be left as default:
    // nullptr and true)
    // As stated in shareddefs.h, sound time is: "NOT DURATION, but rather, some absolute time in the future until which
    // this sound should be delayed"
    Plugin::the().engine_sound().EmitSound(filter, entity_index, 0, sound_name.as_string().string().characters(),
                                           volume, SNDLVL_NORM, 0, pitch, 0, &position);

    return {};
}
}