#pragma once

#include "../RecipientFilter.h"

#include "../RemoveSourceSpecifics.h"

#include <LibJS/Forward.h>
#include <LibJS/Runtime/Value.h>

namespace Cosmo::Scripting
{
JS::ThrowCompletionOr<RecipientFilter> to_recipient_filter(JS::VM&, JS::GlobalObject&, JS::Value);
}