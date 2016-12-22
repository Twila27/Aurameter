#pragma once


#include "Engine/Memory/UntrackedAllocator.hpp"
#include "Engine/EngineCommon.hpp"
#include <map>


//-----------------------------------------------------------------------------
class EngineEvent;
typedef bool(EventCallback)( EngineEvent* engineEvent, void* registryMetadata );


//-----------------------------------------------------------------------------
struct EventSubscriber
{
	EventSubscriber( EventCallback* cb, void* data )
		: handler( cb )
		, registryMetadata( data )
	{
	}
	EventCallback* handler;
	void* registryMetadata;
};


//-----------------------------------------------------------------------------
typedef std::vector<EventSubscriber*> EventSubscriberVector;
typedef std::pair<EngineEventID, EventSubscriberVector > EventRegistryPair;
typedef std::map<EngineEventID, EventSubscriberVector, std::less<EngineEventID>, UntrackedAllocator<EventRegistryPair> > EventRegistryMap;


//-----------------------------------------------------------------------------
template < class C, bool (C::*Method)(EngineEvent*) > 
bool MethodEventRedirector( EngineEvent* ev, void* arg ) 
{ 
	//The key: "arg" or "ptr" is the object we invoke the method upon. Redirector wraps methods under a standalone function for the C-style RegisterEvent.
	C* ptr = (C*)arg; 
	return (ptr->*Method)(ev); 
}


//-----------------------------------------------------------------------------
class TheEventSystem
{
public:
	template < class C, bool (C::*Method)(EngineEvent*) > void RegisterEvent( EngineEventID eventName, C* ptr ) { RegisterEvent( eventName, MethodEventRedirector< C, Method >, ptr ); }
		//C++ wrapper around RegisterEvent via Redirector above to allow method registration.
	void RegisterEvent( EngineEventID eventName, EventCallback* handler, void* registryMetadata );
	template < class C, bool (C::*Method)(EngineEvent*) > bool UnregisterSubscriber( EngineEventID eventName, C* ptr ) { return UnregisterSubscriber( eventName, MethodEventRedirector< C, Method >, ptr ); }
		//C++ wrapper around UnregisterSubscriber via Redirector above to allow method registration.
	bool UnregisterSubscriber( EngineEventID eventName, EventCallback* subbedHandler, void* subbedMetadata );
	bool UnregisterEvent( EngineEventID eventName );
	bool TriggerEvent( EngineEventID eventName, EngineEvent* eventContext = nullptr );

	static TheEventSystem* /*CreateOrGet*/Instance();


private:
	static EventSubscriberVector* CreateOrGetEvent( EngineEventID eventName );
	static EventSubscriberVector* GetEvent( EngineEventID eventName );

	static EventRegistryMap s_subscriberRegistry; //The listeners ARE the callbacks, basically. WHAT we're listening for IS the id.
	static TheEventSystem* s_theEventSystem;
};