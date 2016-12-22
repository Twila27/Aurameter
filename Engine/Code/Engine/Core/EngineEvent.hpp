#pragma once


#include "Engine/EngineCommon.hpp"


//Create as many subclasses as needed on the game side to pass around freely.
class EngineEvent
{

public:
	EngineEvent( EngineEventID id ) : m_id( id ) {}
	void SetID( EngineEventID id ) { m_id = id; }
	EngineEventID GetID() const { return m_id; }

private:
	EngineEventID m_id;
	virtual void ValidateDynamicCast() {}
};


//-----------------------------------------------------------------------------
struct MemoryAllocatedEvent : public EngineEvent
{
	MemoryAllocatedEvent() : EngineEvent( "OnMemoryAllocated" ) {}
	int numBytes;
};


//-----------------------------------------------------------------------------
struct MemoryFreedEvent : public EngineEvent
{
	MemoryFreedEvent() : EngineEvent( "OnMemoryFreed" ) {}
	int numBytes;
};