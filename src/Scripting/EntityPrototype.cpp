#include "../Cosmo.h"
#include "../Types.h"
#include "../Types/BaseEntity.h"
#include <server_class.h>

#include "../RemoveSourceSpecifics.h"

#include "EntityPrototype.h"
#include "Helpers.h"

namespace Cosmo::Scripting
{
EntityPrototype::EntityPrototype(JS::GlobalObject& global_object) : PrototypeObject(*global_object.object_prototype())
{
}

void EntityPrototype::initialize(JS::GlobalObject& global_object)
{
    Object::initialize(global_object);
    define_native_accessor("model", model_getter, model_setter, 0);
    define_native_accessor("index", index_getter, {}, 0);
    define_native_accessor("isPlayer", is_player, {}, 0);
    define_native_accessor("classname", classname_getter, {}, 0);
    define_native_accessor("position", position_getter, position_setter, 0);
    define_native_accessor("team", team_getter, team_setter, 0);
    define_native_accessor("flags", flags_getter, flags_setter, 0);
    define_native_accessor("isValid", is_valid, {}, 0);

    define_native_function("dispatchSpawn", dispatch_spawn, 0, 0);
    define_native_function("teleport", teleport, 3, 0);
    define_native_function("remove", remove, 0, 0);
    define_native_function("emitSound", emit_sound, 1, 0);
    define_native_function("getDataFieldValue", get_data_field_value, 1, 0);
    define_native_function("getSendPropertyValue", get_send_property_value, 1, 0);
    define_native_function("setDataFieldValue", set_data_field_value, 2, 0);
    define_native_function("setSendPropertyValue", set_send_property_value, 2, 0);
}

JS::ThrowCompletionOr<Entity*> EntityPrototype::ensure_this_entity(JS::VM& vm, JS::GlobalObject& global_object)
{
    auto* this_entity = TRY(typed_this_value(global_object));

    if (!this_entity->is_valid())
        return vm.throw_completion<JS::Error>(global_object, "Entity is not valid");

    return this_entity;
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::model_getter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    return JS::js_string(vm, this_entity->entity()->GetModelName().ToCStr());
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::model_setter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    auto model_name = vm.argument(0);
    if (!model_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, model_name);

    if (!Plugin::the().engine_server().IsModelPrecached(model_name.as_string().string().characters()))
        Plugin::the().engine_server().PrecacheModel(model_name.as_string().string().characters());

    this_entity->entity()->SetModel(model_name.as_string().string());

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::index_getter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    return this_entity->entity()->GetRefEHandle().GetEntryIndex();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::is_player)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    return this_entity->entity()->IsPlayer();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::is_valid) { return !ensure_this_entity(vm, global_object).is_error(); }

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::classname_getter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    return JS::js_string(vm, this_entity->entity()->GetClassname());
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::dispatch_spawn)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    Plugin::the().server_tools().DispatchSpawn(this_entity->entity());

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::position_getter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    return from_source_vector(global_object, this_entity->entity()->GetAbsOrigin());
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::position_setter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    auto vector = TRY(to_source_vector(vm, global_object, vm.argument(0)));

    this_entity->entity()->Teleport(&vector, nullptr, nullptr);

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::teleport)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    Optional<SourceVector> new_position;
    Optional<QAngle> new_angles;
    Optional<SourceVector> new_velocity;

    if (auto new_position_argument = vm.argument(0); !new_position_argument.is_nullish())
        new_position = TRY(to_source_vector(vm, global_object, new_position_argument));

    if (auto new_angles_argument = vm.argument(1); !new_angles_argument.is_nullish())
        new_angles = TRY(to_qangle(vm, global_object, new_angles_argument));

    if (auto new_velocity_argument = vm.argument(2); !new_velocity_argument.is_nullish())
        new_velocity = TRY(to_source_vector(vm, global_object, new_velocity_argument));

    this_entity->entity()->Teleport(new_position, new_angles, new_velocity);

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::team_getter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    return this_entity->entity()->GetTeamNumber();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::team_setter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    auto team = vm.argument(0);
    if (!team.is_number())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, team, "number");

    this_entity->entity()->ChangeTeam(team.as_i32());

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::remove)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    Plugin::the().server_tools().RemoveEntity(this_entity->entity());

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::emit_sound)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    Cosmo::Scripting::emit_sound(vm, global_object, vm.argument(0), vm.argument(1), vm.argument(2), vm.argument(3),
                                 this_entity->entity());

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::get_data_field_value)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    auto field_name = vm.argument(0);
    if (!field_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, field_name);

    auto maybe_type_description = find_type_description_from_datamap_by_name_including_base(
        *this_entity->entity()->GetDataDescMap(), field_name.as_string().string());

    if (!maybe_type_description.has_value())
        return vm.throw_completion<JS::Error>(global_object,
                                              String::formatted("Unable to find data field {}", field_name));

    switch (maybe_type_description->fieldType)
    {
        case FIELD_FLOAT:
            return *this_entity->entity()->get_value_by_type_description<float>(*maybe_type_description);
        case FIELD_STRING:
            return JS::js_string(
                vm, this_entity->entity()->get_value_by_type_description<string_t>(*maybe_type_description)->ToCStr());
        case FIELD_VECTOR:
        case FIELD_POSITION_VECTOR:
            return from_source_vector(
                global_object,
                *this_entity->entity()->get_value_by_type_description<SourceVector>(*maybe_type_description));
        case FIELD_INTEGER:
            return *this_entity->entity()->get_value_by_type_description<int>(*maybe_type_description);
        case FIELD_BOOLEAN:
            return *this_entity->entity()->get_value_by_type_description<bool>(*maybe_type_description);
        case FIELD_SHORT:
            return *this_entity->entity()->get_value_by_type_description<i16>(*maybe_type_description);
        case FIELD_CHARACTER:
            return *this_entity->entity()->get_value_by_type_description<char>(*maybe_type_description);
        case FIELD_CLASSPTR:
        {
            auto* entity_pointer =
                *this_entity->entity()->get_value_by_type_description<CBaseEntity*>(*maybe_type_description);

            if (!entity_pointer)
                return JS::js_undefined();

            return Entity::create(verify_cast<GlobalObject>(global_object), entity_pointer);
        }
        case FIELD_EHANDLE:
        {
            auto* handle = this_entity->entity()->get_value_by_type_description<CBaseHandle>(*maybe_type_description);
            if (auto* entity = static_cast<CBaseEntity*>(handle->Get()); entity)
                return Entity::create(verify_cast<GlobalObject>(global_object), entity);

            return JS::js_undefined();
        }
        case FIELD_EDICT:
        {
            auto* edict_pointer =
                *this_entity->entity()->get_value_by_type_description<edict_t*>(*maybe_type_description);

            if (!edict_pointer)
                return JS::js_undefined();

            auto* entity_pointer = Plugin::the().server_game_ents().EdictToBaseEntity(edict_pointer);
            if (!entity_pointer)
                return JS::js_undefined();

            return Entity::create(verify_cast<GlobalObject>(global_object), entity_pointer);
        }
        default:
            return vm.throw_completion<JS::InternalError>(global_object,
                                                          "Unable to represent data field type in script");
    }
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::get_send_property_value)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    auto property_name = vm.argument(0);
    if (!property_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, property_name);

    auto maybe_send_property = find_send_property_from_send_table_including_base(
        *this_entity->entity()->GetServerClass()->m_pTable, property_name.as_string().string());

    if (!maybe_send_property.has_value())
        return vm.throw_completion<JS::Error>(global_object,
                                              String::formatted("Unable to find send property {}", property_name));

    switch (maybe_send_property->GetType())
    {
        case DPT_Int:
            return *this_entity->entity()->get_value_by_send_property<int>(*maybe_send_property);
        case DPT_Float:
            return *this_entity->entity()->get_value_by_send_property<float>(*maybe_send_property);
        case DPT_Vector:
            return from_source_vector(
                global_object, *this_entity->entity()->get_value_by_send_property<SourceVector>(*maybe_send_property));
        case DPT_String:
            return JS::js_string(
                vm, this_entity->entity()->get_value_by_send_property<string_t>(*maybe_send_property)->ToCStr());

        default:
            return vm.throw_completion<JS::InternalError>(global_object,
                                                          "Unable to represent send property type in script");
    }
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::set_data_field_value)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    auto field_name = vm.argument(0);
    if (!field_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, field_name);

    auto maybe_type_description = find_type_description_from_datamap_by_name_including_base(
        *this_entity->entity()->GetDataDescMap(), field_name.as_string().string());

    if (!maybe_type_description.has_value())
        return vm.throw_completion<JS::Error>(global_object,
                                              String::formatted("Unable to find data field {}", field_name));

    auto new_value = vm.argument(1);

    // Not supplying a value (or supplying undefined) isn't what you should be doing -- explicitly supply null.
    if (new_value.is_undefined())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsUndefined, new_value);

    switch (maybe_type_description->fieldType)
    {
        case FIELD_FLOAT:
            if (!new_value.is_number())
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, new_value, "number");

            *this_entity->entity()->get_value_by_type_description<float>(*maybe_type_description) =
                static_cast<float>(new_value.as_double());
            break;
        case FIELD_VECTOR:
        case FIELD_POSITION_VECTOR:
        {
            *this_entity->entity()->get_value_by_type_description<SourceVector>(*maybe_type_description) =
                TRY(to_source_vector(vm, global_object, new_value));
            break;
        }
        case FIELD_INTEGER:
            if (!new_value.is_number())
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, new_value, "number");

            *this_entity->entity()->get_value_by_type_description<int>(*maybe_type_description) = new_value.as_i32();
            break;
        case FIELD_BOOLEAN:
            if (!new_value.is_boolean())
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, new_value, "boolean");

            *this_entity->entity()->get_value_by_type_description<bool>(*maybe_type_description) = new_value.as_bool();
            break;
        case FIELD_SHORT:
            if (!new_value.is_number())
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, new_value, "number");

            *this_entity->entity()->get_value_by_type_description<i16>(*maybe_type_description) =
                static_cast<i16>(new_value.as_i32());
            break;
        case FIELD_CHARACTER:
            if (!new_value.is_number())
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, new_value, "number");

            *this_entity->entity()->get_value_by_type_description<char>(*maybe_type_description) =
                static_cast<char>(new_value.as_i32());
            break;
        case FIELD_CLASSPTR:
        {
            CBaseEntity* entity{};

            if (new_value.is_object() && is<Entity>(new_value.as_object()))
                entity = static_cast<Entity&>(new_value.as_object()).entity();
            else if (!new_value.is_null())
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAnObjectOrNull, new_value);

            *this_entity->entity()->get_value_by_type_description<CBaseEntity*>(*maybe_type_description) = entity;
            break;
        }
        case FIELD_EHANDLE:
        {
            auto value = this_entity->entity()->get_value_by_type_description<CBaseHandle>(*maybe_type_description);
            if (new_value.is_null())
                value->Term();
            else if (new_value.is_object() && is<Entity>(new_value.as_object()))
                *value =
                    const_cast<CBaseHandle&>(static_cast<Entity&>(new_value.as_object()).entity()->GetRefEHandle());
            else
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAnObjectOrNull, new_value);

            break;
        }
        case FIELD_EDICT:
        {
            edict_t* edict{};

            if (new_value.is_object() && is<Entity>(new_value.as_object()))
                edict = static_cast<Entity&>(new_value.as_object()).entity()->GetNetworkable()->GetEdict();
            else if (!new_value.is_null())
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAnObjectOrNull, new_value);

            *this_entity->entity()->get_value_by_type_description<edict_t*>(*maybe_type_description) = edict;
            break;
        }
        default:
            return vm.throw_completion<JS::InternalError>(global_object,
                                                          "Unable to represent data field type in script");
    }

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::set_send_property_value)
{
    auto* this_entity = TRY(typed_this_object(global_object));

    auto property_name = vm.argument(0);
    if (!property_name.is_string())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::NotAString, property_name);

    auto maybe_send_property = find_send_property_from_send_table_including_base(
        *this_entity->entity()->GetServerClass()->m_pTable, property_name.as_string().string());

    if (!maybe_send_property.has_value())
        return vm.throw_completion<JS::Error>(global_object,
                                              String::formatted("Unable to find send property {}", property_name));

    auto new_value = vm.argument(1);

    switch (maybe_send_property->GetType())
    {
        case DPT_Int:
        {
            // Bools are represented as ints, so let's convert so the exposed API is nicer
            int new_integer_value;
            if (new_value.is_number())
                new_integer_value = new_value.as_i32();
            else if (new_value.is_boolean())
                new_integer_value = new_value.as_bool() ? 1 : 0;
            else if (new_value.is_object() && is<Entity>(new_value.as_object()))
                new_integer_value = static_cast<Entity&>(new_value.as_object()).entity()->GetRefEHandle().ToInt();
            else
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, new_value, "number");

            *this_entity->entity()->get_value_by_send_property<int>(*maybe_send_property) = new_integer_value;
            break;
        }
        case DPT_Float:
            if (!new_value.is_number())
                return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, new_value, "number");

            *this_entity->entity()->get_value_by_send_property<float>(*maybe_send_property) =
                static_cast<float>(new_value.as_double());
            break;
        case DPT_Vector:
        {
            auto vector = TRY(to_source_vector(vm, global_object, new_value));

            *this_entity->entity()->get_value_by_send_property<SourceVector>(*maybe_send_property) = vector;
            break;
        }
        default:
            return vm.throw_completion<JS::InternalError>(global_object,
                                                          "Unable to represent send property type in script");
    }

    return JS::js_undefined();
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::flags_getter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    return *this_entity->entity()->get_value_by_type_description<int>(
        *find_type_description_from_datamap_by_name_including_base(*this_entity->entity()->GetDataDescMap(),
                                                                   "m_fFlags"));
}

JS_DEFINE_NATIVE_FUNCTION(EntityPrototype::flags_setter)
{
    auto* this_entity = TRY(ensure_this_entity(vm, global_object));

    auto new_flags = vm.argument(0);
    if (!new_flags.is_number())
        return vm.throw_completion<JS::TypeError>(global_object, JS::ErrorType::IsNotA, new_flags, "number");

    *this_entity->entity()->get_value_by_type_description<int>(
        *find_type_description_from_datamap_by_name_including_base(*this_entity->entity()->GetDataDescMap(),
                                                                   "m_fFlags")) = new_flags.as_i32();

    return JS::js_undefined();
}
}