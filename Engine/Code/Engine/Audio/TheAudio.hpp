#pragma once

//---------------------------------------------------------------------------
//#pragma comment( lib, "ThirdParty/fmod/fmodex_vc" ) // Link in the fmodex_vc.lib static library //Unnecessary IF we include the .lib in .sln's Engine/ThirdParty/fmod filter.

//---------------------------------------------------------------------------
#include "Engine/EngineCommon.hpp"
#include "ThirdParty/fmodStudio/fmod.hpp" //Low level API (FMOD 5.x).
#include "ThirdParty/fmodStudio/fmod_studio.hpp" //FMOD Studio API (requires fmod.hpp).
#include <string>
#include <vector>
#include <map>


//---------------------------------------------------------------------------
typedef unsigned int SoundID;
typedef FMOD::Channel* AudioChannelHandle;
const unsigned int MISSING_SOUND_ID = 0xffffffff;


//---------------------------------------------------------------------------
class AudioSystem;
extern AudioSystem* g_theAudio;


/////////////////////////////////////////////////////////////////////////////
class AudioSystem
{
public:
	AudioSystem();
	virtual ~AudioSystem();
	SoundID CreateOrGetSound( const std::string& soundFileName );
	AudioChannelHandle PlaySound( SoundID soundID, float volumeLevel = 1.f, bool loop = false );
	void StopChannel( AudioChannelHandle channel );
	bool isPlaying( AudioChannelHandle channel );
	void Update( ); // Must be called at regular intervals (e.g. every frame)

	void LoadBankFile( const char* bankFilename );
	FMOD::Studio::EventInstance* CreateEventInstance( const char* eventPath );
	void StartOrRestartEventInstance( FMOD::Studio::EventInstance* inst );
	void SetListenerAttributes( int listenerIndex, FMOD_3D_ATTRIBUTES* attribs );

protected:
	void InitializeFMOD();
	void ValidateResult( FMOD_RESULT result );

protected:
	FMOD::Studio::System*				m_fmodSystem; //Used to load sound banks (fsbanks).
	FMOD::System*						m_fmodLowLevelSystem; //Used more like OpenAL.
	std::map< size_t, SoundID >			m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;
	std::vector< FMOD::Studio::Bank* >	m_registeredSoundBanks;
public:
	FMOD::Studio::EventInstance* CreateEventInstanceAtPosition( const char* eventPath, const Vector3f& pos );
};


//---------------------------------------------------------------------------
void InitializeAudio();