#pragma once

#include "Forward.h"
#include <LibJS/Runtime/GlobalObject.h>

namespace Cosmo::Scripting
{
class GlobalObject : public JS::GlobalObject
{
    JS_OBJECT(ReplObject, JS::GlobalObject);

public:
    GlobalObject() = default;
    void initialize_global_object() override;
    ~GlobalObject() override = default;

    EntityPrototype& entity_prototype() { return *m_entity_prototype; }
    Game& game_object() { return *m_game_object; }

protected:
    void visit_edges(Visitor&) override;

private:
    EntityPrototype* m_entity_prototype{};
    Game* m_game_object{};
};
}