#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <ogg/ogg.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>

#include <windows.h>

#include <al/al.h>
#include <al/alc.h>

#include <string>
#include <list>

using namespace std;

ALenum FORMAT[ 4 ] = { AL_FORMAT_STEREO16, AL_FORMAT_STEREO8, AL_FORMAT_MONO16, AL_FORMAT_MONO8 };

#define STD_STREAM_BUFFERS_QUEUED 64
#define STD_BUFFER_LEN 16 * 1024

class AudioChannel {
    public:
    static void Init() {
        alGetError();
        ___mainAudioDevice = alcOpenDevice( NULL );
        ___mainContext = alcCreateContext( ___mainAudioDevice, NULL );
        alcMakeContextCurrent( ___mainContext );
    }
    static void Cleanup() {
        alcMakeContextCurrent( NULL );
        alcDestroyContext( ___mainContext );
        alcCloseDevice( ___mainAudioDevice );
    }
    AudioChannel() {
        alGenSources( 1, &___ALSource );
        alSourcef( ___ALSource, AL_PITCH, 1 );
        alSourcef( ___ALSource, AL_GAIN, 1 );
        alSource3f( ___ALSource, AL_POSITION, 0, 0, 0 );
        alSource3f( ___ALSource, AL_VELOCITY, 0, 0, 0 );
        alSourcei( ___ALSource, AL_LOOPING, AL_FALSE );
        ___caller = NULL;
        ___state = 0;
    }
    ~AudioChannel() {
        Stop();
        alDeleteSources( 1, &___ALSource );
    }
    float GetVolume() {
        float volume;
        alGetSourcef( ___ALSource, AL_GAIN, &volume );
        return volume;
    }
    void SetVolume( float volume ) {
        alSourcef( ___ALSource, AL_GAIN, volume );
    }
    void SetPitch( float pitch ) {
        alSourcef( ___ALSource, AL_PITCH, pitch );
    }
    bool Play( void* caller, bool stream_caller ) {
        Grinder();
        if ( !___caller ) {
            ___caller = caller;
            ___state |= 0x01 + ( ( ( int )( stream_caller ) + 1 ) << 2 );
            alSourcePlay( ___ALSource );
            return true;
        }
        return false;
    }
    bool PlayForced( void* caller, bool stream_caller ) {
        Stop();
        ___caller = caller;
        ___state |= 0x01 + ( ( ( int )( stream_caller ) + 1 ) << 2 );
        alSourcePlay( ___ALSource );
        return true;
    }
    bool AddQueue( void* buffer, int length, void* caller, int channels, int bytespersample, int freq ) {
        int state;
        alGetSourcei( ___ALSource, AL_BUFFERS_QUEUED, &state );
        if ( ( ___caller == caller ) && ( ( IsStreamOccupied() ) || ( state == 0 ) ) ) {
            ALuint buf;
            alGenBuffers( 1, &buf );
            ALenum fmt = FORMAT[ ( ( ( channels << 1 ) & 0x02 ) + ( bytespersample & 0x01 ) ) ];
            alBufferData( buf, fmt, buffer, length, freq );
            alSourceQueueBuffers( ___ALSource, 1, &buf );
            return true;
        }
        return false;
    }
    bool CompareCaller( void* caller ) {
        return ( caller == ___caller );
    }
    void Stop() {
        alSourceStop( ___ALSource );
        GrindQueue();
        ___state &= 0xF0;
        ___caller = NULL;
    }
    void Pause() {
        alSourcePause( ___ALSource );
        ___state |= 0x02;
    }
    void Resume() {
        alSourcePlay( ___ALSource );
        ___state &= 0xFD;
    }
    void GrindQueue() {
        int queued;
        alGetSourcei( ___ALSource, AL_BUFFERS_QUEUED, &queued );
        while ( queued > 0 ) {
            ALuint buf;
            alSourceUnqueueBuffers( ___ALSource, 1, &buf );
            alDeleteBuffers( 1, &buf );
            queued--;
        }
    }
    void Grinder() {
        int proc;
        alGetSourcei( ___ALSource, AL_BUFFERS_PROCESSED, &proc );
        while ( proc > 0 ) {
            if ( IsSampleOccupied() ) {
                Stop();
            }
            ALuint buf;
            alSourceUnqueueBuffers( ___ALSource, 1, &buf );
            alDeleteBuffers( 1, &buf );
            proc--;
        }
    }
    bool IsPlaying() {
        int state;
        alGetSourcei( ___ALSource, AL_SOURCE_STATE, &state );
        return ( state == AL_PLAYING );
    }
    bool IsStopped() {
        int state;
        alGetSourcei( ___ALSource, AL_SOURCE_STATE, &state );
        return ( state == AL_STOPPED );
    }
    bool IsPaused() {
        int state;
        alGetSourcei( ___ALSource, AL_SOURCE_STATE, &state );
        return ( state == AL_PAUSED );
    }
    bool IsFree() {
        return ( ( ___state & 0x0C ) == 0x00 );
    }
    bool IsSampleOccupied() {
        return ( ( ___state & 0x0C ) == 0x04 );
    }
    bool IsStreamOccupied() {
        return ( ( ___state & 0x0C ) == 0x08 );
    }
    bool BufferAuto( void* caller, OggVorbis_File* vf, vorbis_info* vi, int* bs, bool* eof, bool looped ) {
        //printf( " > PUMP: checking...\n" );
        if ( caller != ___caller ) {
            return false;
        }
        char buffer[ STD_BUFFER_LEN ];
        int queued;
        ALuint buf;
        ALenum fmt = AL_FORMAT_STEREO16;
        int rate = 44100;
        if ( vi ) {
            fmt = FORMAT[ ( ( vi -> channels ) & 0x01 ) ];
            rate = vi -> rate;
        }
        alGetSourcei( ___ALSource, AL_BUFFERS_QUEUED, &queued );
        if ( queued < STD_STREAM_BUFFERS_QUEUED ) {
            while ( queued < STD_STREAM_BUFFERS_QUEUED ) {
                int bytes_read = ov_read( vf, buffer, STD_BUFFER_LEN, 0, 2, 1, bs );
                if ( bytes_read > 0 ) {
                    alGenBuffers( 1, &buf );
                    alBufferData( buf, fmt, buffer, bytes_read, rate );
                    alSourceQueueBuffers( ___ALSource, 1, &buf );
                } else if ( looped ) {
                    ov_raw_seek( vf, 0 );
                }
                queued++;
            }
        } else {
            int proc;
            alGetSourcei( ___ALSource, AL_BUFFERS_PROCESSED, &proc );
            while ( proc > 0 ) {
                int bytes_read = ov_read( vf, buffer, STD_BUFFER_LEN, 0, 2, 1, bs );
                if ( bytes_read > 0 ) {
                    alSourceUnqueueBuffers( ___ALSource, 1, &buf );
                    alBufferData( buf, fmt, buffer, bytes_read, rate );
                    alSourceQueueBuffers( ___ALSource, 1, &buf );
                } else if ( looped ) {
                    ov_raw_seek( vf, 0 );
                }
                proc--;
            }
        }
        return true;
    }
    private:
    ALuint ___ALSource;
    void* ___caller;
    char ___state;
    static ALCdevice* ___mainAudioDevice;
    static ALCcontext* ___mainContext;
};

ALCdevice* AudioChannel::___mainAudioDevice = NULL;
ALCcontext* AudioChannel::___mainContext = NULL;

#define MAX_DEF_CHANNELS 24
AudioChannel* audioChannel[ MAX_DEF_CHANNELS ];

class OggAudio {
    private:
    struct ___oggbufferpart {
        public:
        char* buf;
        int len;
    };
    typedef struct ___oggbufferpart ___OggBufferPart;
    public:
    OggAudio() {
        ___PCMbuffer = NULL;
        ___PCMlength = 0;
        ___rate = 44100;
        ___channels = 2;
        ___bytespersample = 2;
    }
    ~OggAudio() {
        if ( ___PCMbuffer ) {
            free( ___PCMbuffer );
        }
    }
    static OggAudio* Open( string path ) {
        OggAudio* ret = NULL;
        FILE* handle = fopen( path.c_str(), "rb" );
        if ( handle ) {
            OggVorbis_File vf;
            if ( ov_open_callbacks( handle, &vf, NULL, 0, OV_CALLBACKS_DEFAULT ) == 0 ) {
                vorbis_info* vi = ov_info( &vf, -1 );
                ret = new OggAudio();
                ret -> ___rate = vi -> rate;
                ret -> ___channels = vi -> channels;
                list< ___OggBufferPart > l;
                int bytes_read;
                char temp_buffer[ STD_BUFFER_LEN ];
                int total_length = 0;
                int cs;
                do {
                    bytes_read = ov_read( &vf, temp_buffer, STD_BUFFER_LEN, 0, 2, 1, &cs );
                    if ( bytes_read > 0 ) {
                        char* buffer = ( char* )( malloc( bytes_read ) );
                        for ( int i = 0; i < bytes_read; i++ ) {
                            buffer[ i ] = temp_buffer[ i ];
                        }
                        ___OggBufferPart obp;
                        obp.buf = buffer;
                        obp.len = bytes_read;
                        l.push_back( obp );
                        total_length += bytes_read;
                    }
                } while ( bytes_read > 0 );
                ret -> ___PCMbuffer = ( char* )( malloc( total_length ) );
                ret -> ___PCMlength = total_length;
                int clen = 0;
                while ( !( l.empty() ) ) {
                    ___OggBufferPart obp = l.front();
                    for ( int i = 0; i < obp.len; i++ ) {
                        ret -> ___PCMbuffer[ clen + i ] = obp.buf[ i ];
                    }
                    clen += obp.len;
                    free( obp.buf );
                    l.pop_front();
                }
            }
            ov_clear( &vf );
        }
        return ret;
    }
    bool Play( int channelNumber ) {
        //printf( "          > play channel...\n" );
        if ( audioChannel[ channelNumber ] -> Play( this, false ) ) {
            if ( audioChannel[ channelNumber ] -> AddQueue( ___PCMbuffer, ___PCMlength, this, ___channels, ___bytespersample, ___rate ) ) {
                //printf( "          > resuming...\n" );
                audioChannel[ channelNumber ] -> Resume();
                //printf( "          > (true)...\n" );
                return true;
            }
        }
        return false;
    }
    private:
    char* ___PCMbuffer;
    int ___PCMlength;
    int ___rate, ___channels, ___bytespersample;
};

class OggStream {
    public:
    OggStream() {
        ___BitStream = 0;
        ___state = 0;
        ___OVInfo = NULL;
        ___eof = false;
        ___looped = false;
    }
    ~OggStream() {
        ov_clear( &___OVFile );
    }
    static OggStream* Open( string path, bool looped ) {
        OggStream* ret = NULL;
        FILE* handle = fopen( path.c_str(), "rb" );
        if ( handle ) {
            ret = new OggStream();
            ret -> ___looped = looped;
            if ( ov_open_callbacks( handle, &( ret -> ___OVFile ), NULL, 0, OV_CALLBACKS_DEFAULT ) == 0 ) {
                ret -> ___OVInfo = ov_info( &( ret -> ___OVFile ), -1 );
            }
        }
        return ret;
    }
    bool Pump( AudioChannel* ch ) {
        return ch -> BufferAuto( this, &___OVFile, ___OVInfo, &___BitStream, &___eof, ___looped );
    }
    void Rewind( AudioChannel* ch ) {
        ch -> Stop();
        ov_raw_seek( &___OVFile, 0 );
        Play( ch );
        ch -> Resume();
        ___eof = false;
    }
    bool Play( AudioChannel* ch ) {
        if ( ch -> Play( this, true ) ) {
            Pump( ch );
            ___state |= 0x02;
            return true;
        }
        return false;
    }
    void Stop() {
        ___state &= 0xFD;
    }
    void Pause() {
        ___state |= 0x01;
    }
    void Resume() {
        ___state &= 0xFE;
    }
    bool IsPlaying() {
        return ( ( ___state & 0x03 ) == 0x02 );
    }
    bool IsStopped() {
        return ( ( ___state & 0x02 ) == 0x00 );
    }
    bool IsPaused() {
        return ( ( ___state & 0x01 ) == 0x01 );
    }
    private:
    OggVorbis_File ___OVFile;
    vorbis_info* ___OVInfo;
    int ___BitStream;
    int ___state;
    bool ___eof;
    bool ___looped;
};

volatile bool initializedAudio = false;

class StreamPlayer {
    private:
    class ___StreamBind {
        public:
        AudioChannel* channel;
        OggStream* stream;
    };
    public:
    static void Exit() {
        ___done = true;
        WaitForSingleObject( ___thread, INFINITE );
    }
    static void AddMaintenance( AudioChannel* ch, OggStream* os ) {
        ___StreamBind b;
        b.channel = ch;
        b.stream = os;
        //printf( "    > Addmaintenance: Lock()\n" );
        Lock();
        /*if ( ch -> CompareCaller( os ) ) {
            os -> Rewind();
        }*/
        //ch -> Stop();
        bool present = false;
        //printf( "    > Addmaintenance: loop break #1()\n" );
        for ( unsigned int i = 0; i < ___StreamList.size(); i++ ) {
            ___StreamBind a = ___StreamList.front();
            ___StreamList.pop_front();
            if ( a.channel == ch ) {
                //a.stream -> Rewind( ch );
                //delete os;
                delete a.stream;
                a.stream = os;
                a.stream -> Rewind( ch );
                present = true;
            }
            ___StreamList.push_back( a );
            //printf( "    > // Iteration #%d passed\n", i );
        }
        //printf( "    > Addmaintenance: isPresent?\n" );
        if ( !present ) {
            if ( os -> Play( ch ) ) {
                //printf( "       > ch -> Resume: Lock()\n" );
                ch -> Resume();
                ___StreamList.push_back( b );
            } else {
                printf( "       > delete os\n" );
                delete os;
            }
        }
        //printf( "    > Addmaintenance: Unlock()\n" );
        Unlock();
    }
    static DWORD WINAPI StreamMaintain( LPVOID v ) {
        while ( !initializedAudio ) {
            Sleep( 20 );
        }
        while ( !___done ) {
            Lock();
            //printf( "Size = %d\n", ___StreamList.size() );
            for ( unsigned int i = 0; i < ___StreamList.size(); i++ ) {
                //printf( "Stream maintenance... " );
                ___StreamBind b = ___StreamList.front();
                ___StreamList.pop_front();
                //printf( "pumping... " );
                b.stream -> Pump( b.channel );
                if ( !( b.channel -> IsPlaying() ) ) {
                    if ( !( b.channel -> IsPaused() ) ) {
                        //printf( "removed.\n" );
                        delete b.stream;
                        b.channel -> Stop();
                        continue;
                    }
                }
                //printf( "pushed back.\n" );
                ___StreamList.push_back( b );
            }
            Unlock();
            Sleep( 10 );
        }
        CloseHandle( ___semaphore );
        return 0;
    }
    static void Lock() {
        WaitForSingleObject( ___semaphore, INFINITE );
    }
    static void Unlock() {
        ReleaseSemaphore( ___semaphore, 1, NULL );
    }
    static void WaitForSemaphore() {
        while( !___semaphore ) {
            Sleep( 20 );
        }
    }
    static void Start() {
        ___semaphore = CreateSemaphore( NULL, 1, 1, NULL );
        ___thread = CreateThread( NULL, 0, StreamPlayer::StreamMaintain, NULL, 0, NULL );
    }
    private:
    static HANDLE ___thread;
    static volatile bool ___done;
    static list< ___StreamBind > ___StreamList;
    static volatile HANDLE ___semaphore;
};

HANDLE StreamPlayer::___thread = NULL;
volatile bool StreamPlayer::___done = false;
list< StreamPlayer::___StreamBind > StreamPlayer::___StreamList;
volatile HANDLE StreamPlayer::___semaphore = NULL;

//volatile bool initializedAudio = false;

void Audio_Create() {
    AudioChannel::Init();
    for ( int i = 0; i < MAX_DEF_CHANNELS; i++ ) {
        audioChannel[ i ] = new AudioChannel();
        //StreamPlayer::AddChannel( audioChannel[ i ] );
    }
    initializedAudio = true;
    StreamPlayer::Start();
}

void Audio_Destroy() {
    StreamPlayer::Exit();
    for ( int i = 0; i < MAX_DEF_CHANNELS; i++ ) {
        delete audioChannel[ i ];
    }
    AudioChannel::Cleanup();
}



