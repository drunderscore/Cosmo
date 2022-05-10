#pragma once

#include <igameevents.h>
#include <tier1/KeyValues.h>

// Taken directly from the Engine
class CGameEventDescriptor
{
public:
    char name[MAX_EVENT_NAME_LENGTH]; // name of this event
    int eventid;                      // network index number, -1 = not networked
    KeyValues* keys;                  // KeyValue describing data types, if NULL only name
    bool local;                       // local event, never tell clients about that
    bool reliable;                    // send this event as reliable message
    //    CUtlVector<CGameEventCallback*> listeners; // registered listeners

    // Stolen from the Engine, I just made it an enum class and renamed members to mirror that.
    enum class Type
    {
        Local = 0, // not networked
        String,    // zero terminated ASCII string
        Float,     // float 32 bit
        Long,      // signed int 32 bit
        Short,     // signed int 16 bit
        Byte,      // unsigned int 8 bit
        Bool       // unsigned int 1 bit
    };
};

class CGameEvent : public IGameEvent
{
public:
    CGameEventDescriptor& descriptor() const { return *m_descriptor; }

private:
    CGameEventDescriptor* m_descriptor;
};