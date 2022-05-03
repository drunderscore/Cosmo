#pragma once

#include <iserverentity.h>

// CBaseEntity inherits IServerEntity, but it's impossible to say that through forward-declarations alone.
// Use this empty class to give a bit more info.
class CBaseEntity : public IServerEntity
{
};