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
    JS_DECLARE_NATIVE_FUNCTION(create_entity_by_name);
    JS_DECLARE_NATIVE_FUNCTION(create_fake_client);
    JS_DECLARE_NATIVE_FUNCTION(get_entity_by_index);
};
}