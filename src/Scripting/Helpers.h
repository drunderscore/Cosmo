#pragma once

#include "../RecipientFilter.h"

#include "../RemoveSourceSpecifics.h"

#include "../Types.h"
#include <LibJS/Forward.h>
#include <LibJS/Runtime/Array.h>
#include <LibJS/Runtime/Value.h>

namespace Cosmo::Scripting
{
JS::ThrowCompletionOr<RecipientFilter> to_recipient_filter(JS::VM&, JS::GlobalObject&, JS::Value);
JS::ThrowCompletionOr<SourceVector> to_source_vector(JS::VM&, JS::GlobalObject&, JS::Value, int number_of_elements = 3);
JS::ThrowCompletionOr<QAngle> to_qangle(JS::VM&, JS::GlobalObject&, JS::Value, int number_of_elements = 3);
JS::Array* from_source_vector(JS::GlobalObject&, SourceVector&);
}