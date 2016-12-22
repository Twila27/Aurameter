#include "Engine/Core/TheEventSystem.hpp"
#include "Engine/Core/EngineEvent.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/EngineCommon.hpp"


//--------------------------------------------------------------------------------------------------------------
STATIC TheEventSystem* TheEventSystem::s_theEventSystem = nullptr;
STATIC EventRegistryMap TheEventSystem::s_subscriberRegistry;


//--------------------------------------------------------------------------------------------------------------
STATIC TheEventSystem* /*CreateOrGet*/ TheEventSystem::Instance()
{
	if ( s_theEventSystem == nullptr )
	{
		TheEventSystem* eventSystem = (TheEventSystem*)malloc( sizeof(TheEventSystem) ); //Because events fire off in new, can't use it.
		new ( eventSystem ) TheEventSystem();
		s_theEventSystem = eventSystem;
	}

	return s_theEventSystem;
}


//--------------------------------------------------------------------------------------------------------------
void TheEventSystem::RegisterEvent( EngineEventID eventName, EventCallback* handler, void* registryMetadata )
{
	EventSubscriberVector* subscribers = TheEventSystem::CreateOrGetEvent( eventName );
	subscribers->push_back( new EventSubscriber( handler, registryMetadata ) ); 
	//When eventName == "OnMemoryFreed", (ProfilerSample*)registryMetadata->tag == "TheEngine::Update", it deletes "Main" which's in subscribers at that point???
}


//--------------------------------------------------------------------------------------------------------------
bool TheEventSystem::UnregisterEvent( EngineEventID eventName )
{
	EventRegistryMap::iterator found = s_subscriberRegistry.find( eventName );
	if ( found != s_subscriberRegistry.end() )
	{
		EventSubscriberVector& subs = found->second;
		for ( EventSubscriber* sub : subs )
		{
			delete sub;
			sub = nullptr;
		} //No clear(), not a static container.
		s_subscriberRegistry.erase( found );
		return true;
	}
	else return false;
}


//--------------------------------------------------------------------------------------------------------------
bool TheEventSystem::UnregisterSubscriber( EngineEventID eventName, EventCallback* subbedHandler, void* subbedMetadata )
{
	EventRegistryMap::iterator found = s_subscriberRegistry.find( eventName );
	if ( found != s_subscriberRegistry.end() )
	{
		EventSubscriberVector& subs = found->second;
		unsigned int subIndex;
		for ( subIndex = 0; subIndex < subs.size(); subIndex++ )
		{
			EventSubscriber* sub = subs[ subIndex ];
			if ( sub->handler == subbedHandler && sub->registryMetadata == subbedMetadata )
				break;
		}
		if ( subIndex < subs.size() )
		{
			EventSubscriber* sub = subs[ subIndex ];
			subs.erase( subs.begin() + subIndex );
			delete sub;
			return true;
		}
		else return false;
	}
	else return false;
}


//--------------------------------------------------------------------------------------------------------------
bool TheEventSystem::TriggerEvent( EngineEventID eventName, EngineEvent* eventContext /*= nullptr*/ )
{
	EventSubscriberVector* subscribers = TheEventSystem::GetEvent( eventName );
	if ( subscribers == nullptr )
		return false;

	for ( EventSubscriberVector::iterator subIter = subscribers->begin(); subIter != subscribers->end(); )
	{
		EventSubscriber* sub = *subIter;
		bool shouldRemoveSubscriber = sub->handler( eventContext, sub->registryMetadata );
		if ( shouldRemoveSubscriber )
		{
			subIter = subscribers->erase( subIter );
		}
		else ++subIter;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------------------
STATIC EventSubscriberVector* TheEventSystem::CreateOrGetEvent( EngineEventID eventName )
{
	EventRegistryMap::iterator found = s_subscriberRegistry.find( eventName );
	if ( found != s_subscriberRegistry.end() )
		return &found->second;

	s_subscriberRegistry.insert( EventRegistryPair( eventName, EventSubscriberVector() ) );

	return &s_subscriberRegistry.at( eventName );
}


//--------------------------------------------------------------------------------------------------------------
STATIC EventSubscriberVector* TheEventSystem::GetEvent( EngineEventID eventName )
{
	if ( s_subscriberRegistry.size() == 0 )
		return nullptr;

	EventRegistryMap::iterator found = s_subscriberRegistry.find( eventName );
	if ( found != s_subscriberRegistry.end() )
		return &found->second;

	return nullptr;
}
