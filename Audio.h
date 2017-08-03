#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "Stringlinker.h"

//#include "Bass.h"

#include <string>

using namespace std;
/*
StringLinker* customAudio = NULL;

using namespace std;

void InitAudio() {
    BASS_Init( -1, 44100, 0, 0, NULL );
    BASS_SetVolume( 1 );
    customAudio = new StringLinker( "data/audio/customSound.slf" );
}

#define MAX_AUDIO_STREAM 32
HSTREAM streamAudio[ MAX_AUDIO_STREAM ];

#define AUDIO_PATH "data/audio/"

void StopSound( int channelNumber ) {
    BASS_ChannelStop( streamAudio[ channelNumber ] );
}

void CleanAudio() {
    for ( int i = 0; i < MAX_AUDIO_STREAM; i++ ) {
        StopSound( i );
    }
}

void PlaySound( int channelNumber, string path ) {
    StopSound( channelNumber );
    //printf( "Play attempt: %s\n", ( AUDIO_PATH + path ).c_str() );
    streamAudio[ channelNumber ] = BASS_StreamCreateFile( false, ( AUDIO_PATH + path ).c_str(), 0, 0, BASS_STREAM_AUTOFREE );
    BASS_ChannelPlay( streamAudio[ channelNumber ], FALSE );
}

void PauseSound( int channelNumber ) {
    BASS_ChannelPause( streamAudio[ channelNumber ] );
}

void ResumeSound( int channelNumber ) {
    BASS_ChannelPlay( streamAudio[ channelNumber ], FALSE );
}

void LoopSound( int channelNumber, string path ) {
    streamAudio[ channelNumber ] = BASS_StreamCreateFile( false, ( AUDIO_PATH + path ).c_str(), 0, 0, BASS_SAMPLE_LOOP | BASS_STREAM_AUTOFREE );
    BASS_ChannelPlay( streamAudio[ channelNumber ], FALSE );
}

bool IsPlaying( int channelNumber ) {
    return ( BASS_ChannelIsActive( streamAudio[ channelNumber ] ) == BASS_ACTIVE_PLAYING );
}

void SetVolumeDist( int channelNumber, double distance, double soundFade ) {
    BASS_ChannelSetAttribute( streamAudio[ channelNumber ], BASS_ATTRIB_VOL, 1.0 / ( 1.0 + distance * soundFade ) );
}

float GetVolume( int channelNumber ) {
    float volume;
    BASS_ChannelGetAttribute( streamAudio[ channelNumber ], BASS_ATTRIB_VOL, &volume );
    return volume;
}

void SetVolume( int channelNumber, double volume ) {
    BASS_ChannelSetAttribute( streamAudio[ channelNumber ], BASS_ATTRIB_VOL, volume );
}

#define CHANNEL_BACKGROUND 1
#define CHANNEL_STEPS 2
#define CHANNEL_PLAYER_DAMAGE 3
#define CHANNEL_PAIN 4
#define CHANNEL_SHOOT 5
#define CHANNEL_HIT 6
#define CHANNEL_DEATH 7
#define CHANNEL_PICKUP 8
#define CHANNEL_KAYLEY 9
#define CHANNEL_CUSTOM 16

#define SOUND_DOG_DEATH 1
#define SOUND_HORSE_DEATH 2
#define SOUND_HORSE_ATTACK 32
#define SOUND_KAYLEY_ATTACK 33
#define SOUND_KAYLEY_BREATH 34

void PlayCustomSound( int channelNumber, int soundID ) {
    if ( customAudio ) {
        PlaySound( channelNumber, customAudio -> GetString( soundID ) );
    }
}

void LoopCustomSound( int channelNumber, int soundID ) {
    if ( customAudio ) {
        LoopSound( channelNumber, customAudio -> GetString( soundID ) );
    }
}
*/

#include "VorbisAudio.h"

StringLinker* customAudio = NULL;

void InitAudio() {
    Audio_Create();
    customAudio = new StringLinker( "data/audio/customSound.slf" );
}

#define AUDIO_PATH "data/audio/"

//#define MAX_AUDIO_STREAM 32
//HSTREAM streamAudio[ MAX_AUDIO_STREAM ];

void DeleteAudio() {
    Audio_Destroy();
    delete customAudio;
}

void StopSound( int channelNumber ) {
    audioChannel[ channelNumber ] -> Stop();
}

void CleanAudio() {
    for ( int i = 0; i < MAX_DEF_CHANNELS; i++ ) {
        StopSound( i );
    }
}

void PlaySound( int channelNumber, string path ) {
    OggStream* stream = OggStream::Open( AUDIO_PATH + path, false );
    if ( stream ) {
        StreamPlayer::AddMaintenance( audioChannel[ channelNumber ], stream );
    } else {
        printf( "Can't play: %s.\n", path.c_str() );
    }
}

void PlaySoundAbsolute( int channelNumber, string path ) {
    OggStream* stream = OggStream::Open( path, false );
    if ( stream ) {
        StreamPlayer::AddMaintenance( audioChannel[ channelNumber ], stream );
    } else {
        printf( "Can't play: %s.\n", path.c_str() );
    }
}

void PauseSound( int channelNumber ) {
    //BASS_ChannelPause( streamAudio[ channelNumber ] );
    audioChannel[ channelNumber ] -> Pause();
}

void ResumeSound( int channelNumber ) {
    //BASS_ChannelPlay( streamAudio[ channelNumber ], FALSE );
    audioChannel[ channelNumber ] -> Resume();
}

void LoopSound( int channelNumber, string path ) {
    //streamAudio[ channelNumber ] = BASS_StreamCreateFile( false, ( AUDIO_PATH + path ).c_str(), 0, 0, BASS_SAMPLE_LOOP | BASS_STREAM_AUTOFREE );
    //BASS_ChannelPlay( streamAudio[ channelNumber ], FALSE );
    OggStream* stream = OggStream::Open( AUDIO_PATH + path, true );
    if ( stream ) {
        StreamPlayer::AddMaintenance( audioChannel[ channelNumber ], stream );
    } else {
        printf( "Can't loop: %s.\n", path.c_str() );
    }
}

bool IsPlaying( int channelNumber ) {
    return audioChannel[ channelNumber ] -> IsPlaying();//( BASS_ChannelIsActive( streamAudio[ channelNumber ] ) == BASS_ACTIVE_PLAYING );
}

void SetVolumeDist( int channelNumber, double distance, double soundFade ) {
    //BASS_ChannelSetAttribute( streamAudio[ channelNumber ], BASS_ATTRIB_VOL, 1.0 / ( 1.0 + distance * soundFade ) );
    audioChannel[ channelNumber ] -> SetVolume( 1.0 / ( 1.0 + distance * soundFade ) );
}

float GetVolume( int channelNumber ) {
    //float volume;
    //BASS_ChannelGetAttribute( streamAudio[ channelNumber ], BASS_ATTRIB_VOL, &volume );
    return audioChannel[ channelNumber ] -> GetVolume();//volume;
}

void SetVolume( int channelNumber, double volume ) {
    //BASS_ChannelSetAttribute( streamAudio[ channelNumber ], BASS_ATTRIB_VOL, volume );
    audioChannel[ channelNumber ] -> SetVolume( volume );
}

void SetPitch( int channelNumber, double pitch ) {
    audioChannel[ channelNumber ] -> SetPitch( pitch );
}

#define CHANNEL_BACKGROUND 1
#define CHANNEL_STEPS 2
#define CHANNEL_PLAYER_DAMAGE 3
#define CHANNEL_PAIN 4
#define CHANNEL_SHOOT 5
#define CHANNEL_HIT 6
#define CHANNEL_DEATH 7
#define CHANNEL_PICKUP 8
#define CHANNEL_KAYLEY 9
#define CHANNEL_CHEAT 15
#define CHANNEL_CUSTOM 16
#define CHANNEL_RADIO 17
#define CHANNEL_MENUMUSIC 18

#define SOUND_DOG_DEATH 1
#define SOUND_HORSE_DEATH 2
#define SOUND_HORSE_ATTACK 32
#define SOUND_KAYLEY_ATTACK 33
#define SOUND_KAYLEY_BREATH 34

void PlayCustomSound( int channelNumber, int soundID ) {
    if ( customAudio ) {
        PlaySound( channelNumber, customAudio -> GetString( soundID ) );
    }
}

void LoopCustomSound( int channelNumber, int soundID ) {
    if ( customAudio ) {
        LoopSound( channelNumber, customAudio -> GetString( soundID ) );
    }
}
