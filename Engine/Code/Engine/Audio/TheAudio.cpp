//---------------------------------------------------------------------------
#include "Engine/Audio/TheAudio.hpp"
#include "Engine/Error/ErrorWarningAssert.hpp"
#include "Engine/BuildConfig.hpp"

#ifdef PLATFORM_RIFT_CV1
	#include "OculusFMODSpatializerSettings.h"
#endif

//---------------------------------------------------------------------------
AudioSystem* g_theAudio = nullptr;



//---------------------------------------------------------------------------
AudioSystem::AudioSystem()
	: m_fmodSystem( nullptr )
{
	InitializeFMOD();
}


//---------------------------------------------------------------------------
// FMOD startup code based on "GETTING STARTED With FMOD Ex Programmer’s API for Windows" document
//	from the FMOD programming API at http://www.fmod.org/download/
//
void AudioSystem::InitializeFMOD()
{
	const int MAX_AUDIO_DEVICE_NAME_LEN = 256;
	FMOD_RESULT result;
	unsigned int fmodVersion;
	int numDrivers;

	// Create a System object and initialize.
	result = FMOD::Studio::System::create( &m_fmodSystem );
	ValidateResult( result );

	result = m_fmodSystem->getLowLevelSystem( &m_fmodLowLevelSystem );
	ValidateResult( result );

	result = m_fmodLowLevelSystem->getVersion( &fmodVersion );
	ValidateResult( result );

	if( fmodVersion < FMOD_VERSION )
		DebuggerPrintf( "AUDIO SYSTEM ERROR!  Your FMOD .dll is of an older version (0x%08x == %d) than that the .lib used to compile this code (0x%08x == %d).\n", fmodVersion, fmodVersion, FMOD_VERSION, FMOD_VERSION );

	//Allow for no sound drivers.
	result = m_fmodLowLevelSystem->getNumDrivers( &numDrivers );
	ValidateResult( result );
	if( numDrivers == 0 )
	{
		result = m_fmodLowLevelSystem->setOutput( FMOD_OUTPUTTYPE_NOSOUND );
		ValidateResult( result );
	}

#ifdef PLATFORM_RIFT_CV1
	int sampleRate = 0;
	result = m_fmodLowLevelSystem->getSoftwareFormat( &sampleRate, NULL, NULL );
	ValidateResult( result );

	unsigned int bufferSize = 0;
	result = m_fmodLowLevelSystem->getDSPBufferSize( &bufferSize, NULL );
	ValidateResult( result );

	OSP_FMOD_Initialize( sampleRate, bufferSize );
	OSP_FMOD_SetSimpleBoxRoomParameters( 5.f, 2.1f, 3.7f, .75f, .65f, .55f, .25f, .65f, .65f );
#endif

	//Actual bootup, note this happens even without sound.
	result = m_fmodSystem->initialize( 32, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, NULL ); //From 100 (SD1) to 32 (FMOD example).
	if( result == FMOD_ERR_OUTPUT_CREATEBUFFER )
	{
		// Ok, the speaker mode selected isn't supported by this sound card. Switch it
		// back to stereo...
		int prevSampleRate, prevNumRawSpeakers;
		result = m_fmodLowLevelSystem->getSoftwareFormat( &prevSampleRate, NULL, &prevNumRawSpeakers );
		ValidateResult( result );
#ifdef PLATFORM_RIFT_CV1
		result = m_fmodLowLevelSystem->setSoftwareFormat( prevSampleRate, FMOD_SPEAKERMODE_MONO, prevNumRawSpeakers );
#else
		result = m_fmodLowLevelSystem->setSoftwareFormat( prevSampleRate, FMOD_SPEAKERMODE_STEREO, prevNumRawSpeakers );
#endif
		ValidateResult( result );

		// ... and re-init. Down from 100 (SD1) to 32 (Oculus SDK example).
		result = m_fmodSystem->initialize( 32, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0 );
		ValidateResult( result );
	}

#ifdef PLATFORM_RIFT_CV1
	result = m_fmodLowLevelSystem->loadPlugin( "ovrfmod.dll", NULL );
	ValidateResult( result );
#endif

}


//---------------------------------------------------------------------------
AudioSystem::~AudioSystem()
{
	FMOD_RESULT result = FMOD_OK;
	
	result = m_fmodSystem->unloadAll();
	ValidateResult( result );

	result = m_fmodSystem->release();
	ValidateResult( result );
 	m_fmodSystem = nullptr;
}


//---------------------------------------------------------------------------
SoundID AudioSystem::CreateOrGetSound( const std::string& soundFileName )
{
	size_t nameHash = std::hash<std::string>{}( soundFileName );
	std::map< size_t, SoundID >::iterator found = m_registeredSoundIDs.find( nameHash );
	if( found != m_registeredSoundIDs.end() )
	{
		return found->second;
	}
	else
	{
		FMOD::Sound* newSound = nullptr;
		m_fmodLowLevelSystem->createSound( soundFileName.c_str(), FMOD_DEFAULT, nullptr, &newSound );
		if( newSound )
		{
			SoundID newSoundID = m_registeredSounds.size();
			m_registeredSoundIDs[ nameHash ] = newSoundID;
			m_registeredSounds.push_back( newSound );
			return newSoundID;
		}
	}

	return MISSING_SOUND_ID;
}


//---------------------------------------------------------------------------
AudioChannelHandle AudioSystem::PlaySound( SoundID soundID, float volumeLevel /*= 1.f*/, bool loop /*= false*/ )
{
	unsigned int numSounds = m_registeredSounds.size();
	if( soundID < 0 || soundID >= numSounds )
		return nullptr;

	FMOD::Sound* sound = m_registeredSounds[ soundID ];
	if( !sound )
		return nullptr;

	AudioChannelHandle channelAssignedToSound = nullptr;
	m_fmodLowLevelSystem->playSound( sound, FMOD_DEFAULT, false, &channelAssignedToSound );
	if( channelAssignedToSound )
	{
		channelAssignedToSound->setVolume( volumeLevel );

		if ( loop )
			channelAssignedToSound->setMode( FMOD_LOOP_NORMAL );
		else
			channelAssignedToSound->setMode( FMOD_LOOP_OFF );
	}
	m_fmodLowLevelSystem->update();

	return channelAssignedToSound;
}

//---------------------------------------------------------------------------
void AudioSystem::StopChannel( AudioChannelHandle channel )
{
	if ( channel != nullptr )
	{
		FMOD::Channel* fmodchannel = ( FMOD::Channel* ) channel;
		fmodchannel->stop();
	}
}


//---------------------------------------------------------------------------
bool AudioSystem::isPlaying( AudioChannelHandle channel )
{
	bool isPlaying = false;
	if ( channel != nullptr )
	{
		FMOD::Channel* fmodchannel = ( FMOD::Channel* ) channel;
		fmodchannel->isPlaying( &isPlaying );
	}
	return isPlaying;
}


//---------------------------------------------------------------------------
void AudioSystem::Update( )
{
	FMOD_RESULT result = m_fmodSystem->update();
	ValidateResult( result );
}


//---------------------------------------------------------------------------
void AudioSystem::LoadBankFile( const char* bankFilename )
{
	//Usually filename has form "Data/Audio/xxx.bank". Don't forget to load .strings.bank.
	FMOD::Studio::Bank* newBank = NULL;
	FMOD_RESULT result = FMOD_OK;
	result = m_fmodSystem->loadBankFile( bankFilename, FMOD_STUDIO_LOAD_BANK_NORMAL, &newBank );
	ValidateResult( result );
	m_registeredSoundBanks.push_back( newBank );
}


//---------------------------------------------------------------------------
FMOD::Studio::EventInstance* AudioSystem::CreateEventInstance( const char* eventPath )
{
//	FMOD_GUID newID = { 0 };
//	TODO( "CreateOrGets for EventDescs?" );
	FMOD::Studio::EventDescription* newEventDesc = NULL; //Used to spin off an instance.
	FMOD::Studio::EventInstance* newEventInst = NULL;
	FMOD_RESULT result = FMOD_OK;

//	m_fmodSystem->lookupID( eventPath, &newID );
	result = m_fmodSystem->getEvent( eventPath, &newEventDesc );
	ValidateResult( result );

	result = newEventDesc->createInstance( &newEventInst ); //BAD if we're losing and reusing this data a bunch.
		//Better to use loadSampleData in that case, though it means finding a place for a call to unloadSampleData (ref counting based).
	ValidateResult( result );

	return newEventInst;
}


//---------------------------------------------------------------------------
void AudioSystem::StartOrRestartEventInstance( FMOD::Studio::EventInstance* inst )
{
	inst->start(); //Note recalling this counts as a restart.

	m_fmodSystem->update();
}


//---------------------------------------------------------------------------
void AudioSystem::SetListenerAttributes( int listenerIndex, FMOD_3D_ATTRIBUTES* attribs )
{
	m_fmodSystem->setListenerAttributes( listenerIndex, attribs );
}


//---------------------------------------------------------------------------
void AudioSystem::ValidateResult( FMOD_RESULT result )
{
	if( result != FMOD_OK )
	{
		DebuggerPrintf( "AUDIO SYSTEM ERROR: Got error result code %d.\n", result );
		__debugbreak();
	}
}


//---------------------------------------------------------------------------
FMOD::Studio::EventInstance* AudioSystem::CreateEventInstanceAtPosition( const char* eventPath, const Vector3f& pos )
{
	FMOD::Studio::EventInstance* soundInst = g_theAudio->CreateEventInstance( eventPath );
	FMOD_3D_ATTRIBUTES attributes = { { 0 } };
	attributes.position = { pos.x, pos.y, pos.z };
	soundInst->set3DAttributes( &attributes );

	return soundInst;
}
