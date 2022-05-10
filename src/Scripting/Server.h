#pragma once

#include "Forward.h"
#include <LibJS/Runtime/Object.h>

namespace Cosmo::Scripting
{
class Server final : public JS::Object
{
    JS_OBJECT(Server, Object);

public:
    explicit Server(GlobalObject&);
    void initialize(JS::GlobalObject&) override;
    ~Server() override = default;

private:
    JS_DECLARE_NATIVE_FUNCTION(map_getter);
    JS_DECLARE_NATIVE_FUNCTION(map_setter);

    JS_DECLARE_NATIVE_FUNCTION(create_entity_by_name);
    JS_DECLARE_NATIVE_FUNCTION(create_fake_client);
    JS_DECLARE_NATIVE_FUNCTION(get_entity_by_index);
    JS_DECLARE_NATIVE_FUNCTION(emit_sound);
    JS_DECLARE_NATIVE_FUNCTION(say_text_2);
    JS_DECLARE_NATIVE_FUNCTION(get_player_by_userid);
    JS_DECLARE_NATIVE_FUNCTION(get_entity_by_handle);
};
}