#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string>

//#include "Bass.h"

using namespace std;

class BassRadio {
    public:
    BassRadio( string dirInit, string extension, bool aPlay = true ) {
        pNum = 0;
        cur = 0;
        path = NULL;
        radioStream = 0;
        char* dirI = ( char* )( dirInit.c_str() );
        char* extI = ( char* )( extension.c_str() );
        Rescan( dirI, extI );
        autoPlay = aPlay;
        paused = false;
    }
    ~BassRadio() {
        Flush();
    }
    void Append( char* fileName ) {
        pNum++;
        path = ( char* (*)[] )( realloc( path, pNum * sizeof( char* ) ) );
        ( *path )[ pNum - 1 ] = fileName;
    }
    int Next() {
        if ( pNum ) {
            cur = ( cur + 1 ) % pNum;
            return 1;
        }
        return 0;
    }
    char* GetCurrentPath() {
        return ( *path )[ cur ];
    }
    inline void Stop() {
        BASS_ChannelStop( radioStream );
        paused = false;
    }
    inline void Pause() {
        paused = BASS_ChannelPause( radioStream );
    }
    inline bool Paused() {
        return paused;
    }
    inline void Resume() {
        BASS_ChannelPlay( radioStream, FALSE );
        paused = false;
    }
    inline void Play() {
        Stop();
        radioStream = BASS_StreamCreateFile( false, ( *path )[ cur ], 0, 0, BASS_STREAM_AUTOFREE );
        Resume();
    }
    inline bool IsPlaying() {
        if ( paused ) {
            return true;
        }
        return ( BASS_ChannelIsActive( radioStream ) == BASS_ACTIVE_PLAYING );
    }
    inline void TurnAutoPlay( bool aP ) {
        autoPlay = aP;
    }
    inline bool AutoPlay() {
        if ( ( !IsPlaying() ) && ( autoPlay ) ) {
            if ( Next() ) {
                Play();
                return true;
            }
        }
        return false;
    }
    void Rescan( char* newDir, char* newExt ) {
        if ( newDir ) {
            strcpy( dir, newDir );
        }
        if ( newExt ) {
            strcpy( ext, newExt );
        }
        printf( "[ BassRadio %08X ] Scan started at %s*%s\n", int( this ), dir, ext );
        Flush();
        ___LinkPatches( dir, ext );
        printf( "[ BassRadio %08X ] Total: %d\n", int( this ), pNum );
        if ( autoPlay ) {
            Play();
        }
    }
    void Flush() {
        Stop();
        if ( path ) {
            for ( int i = 0; i < pNum; i++ ) {
                free( ( *path )[ i ] );
            }
            free( path );
        }
        path = NULL;
        pNum = 0;
        cur = 0;
    }
    void MixPathes() {
        for ( int i = 0; i < pNum; i++ ) {
            int repl = rand() % pNum;
            if ( cur == i ) {
                cur = repl;
            } else if ( cur == repl ) {
                cur = i;
            }
            char* tmpHandle = ( *path )[ i ];
            ( *path )[ i ] = ( *path )[ repl ];
            ( *path )[ repl ] = tmpHandle;
        }
    }
    void DisplayPathes() {
        printf( "[ BassRadio %08X ] Pathes:\n", int( this ) );
        for ( int i = 0; i < pNum; i++ ) {
            if ( cur == i ) {
                printf( "     > " );
            } else {
                printf( "       " );
            }
            printf( "%d :: %s\n", i, ( *path )[ i ] );
        }
        printf( "[ BassRadio %08X ] Total: %d\n", int( this ), pNum );
    }
    void ResetPlaying() {
        Play();
    }
    void PlayRandom() {
        if ( pNum ) {
            cur = rand() % pNum;
            Play();
        }
    }
    inline int Found() {
        return pNum;
    }
    void SwitchCursor( int v ) {
        if ( pNum > 0 ) {
            cur += ( v % pNum );
            if ( cur < 0 ) {
                cur += pNum;
            } else if ( cur >= pNum ) {
                cur -= pNum;
            }
        }
    }
    private:
    void ___LinkPatches( char* ddir, char* eext, int level = 0 ) {
        char dirLook[ 512 ];
        WIN32_FIND_DATA findData;
        // search subfolders
        dirLook[ 0 ] = 0;
        strcat( dirLook, ddir );
        strcat( dirLook, "*" );
        HANDLE hFind = FindFirstFile( dirLook, &findData );
        if ( hFind != INVALID_HANDLE_VALUE ) {
            do {
                if ( ( findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ) && ( strcmp( findData.cFileName, ".." ) ) && ( strcmp( findData.cFileName, "." ) ) ) {
                    char dirSubLook[ 512 ];
                    dirSubLook[ 0 ] = 0;
                    strcat( dirSubLook, ddir );
                    strcat( dirSubLook, findData.cFileName );
                    strcat( dirSubLook, "/" );
                    ___LinkPatches( dirSubLook, eext, level + 1 );
                }
            } while ( FindNextFile( hFind, &findData ) != 0 );
            FindClose( hFind );
        }
        // then look for remaining eext in current folder
        strcat( dirLook, eext );
        hFind = FindFirstFile( dirLook, &findData );
        if ( hFind != INVALID_HANDLE_VALUE ) {
            do {
                for ( int i = 0; i < level; i++ ) {
                    printf( "  " );
                }
                printf( "[ BassRadio %08X ] Adding %s\n", int( this ), findData.cFileName );
                int sLen = strlen( findData.cFileName ) + strlen( dirLook ) + 1;
                char* data = ( char* )( malloc( sLen * sizeof( char ) ) );
                data[ 0 ] = 0;
                strcat( data, ddir );
                strcat( data, findData.cFileName );
                Append( data );
            } while ( FindNextFile( hFind, &findData ) != 0 );
            FindClose( hFind );
        }
    }
    char* ( *path )[];
    int cur;
    int pNum;
    //HSTREAM radioStream;
    OggStream* radioStream;
    bool autoPlay, paused;
    char dir[ 256 ];
    char ext[ 32 ];
};

BassRadio* radio = NULL;
bool RadioDone = false;

int foundRadio = 0;
int changeRadio = 0;
bool pauseRadio = true;
bool radioSoundState = false;
volatile bool radioEstablished = false;

DWORD WINAPI Radio( LPVOID LpParam ) {
    srand( time( NULL ) );
    BASS_Init( -1, 44100, 0, 0, NULL );
    BASS_SetVolume( 1 );
    // BassRadio
    radio = new BassRadio( "data/MyMusic/", ".mp3" );
    if ( radio -> Found() > 0 ) {
        // main radio loop
        radio -> MixPathes();
        radio -> PlayRandom();
        radioEstablished = true;
        printf( "Now playing: %s\n", radio -> GetCurrentPath() );
        while ( !RadioDone ) {
            if ( pauseRadio ) {
                if ( radio -> Paused() ) {
                    radioSoundState = true;
                    radio -> Resume();
                } else {
                    radioSoundState = false;
                    radio -> Pause();
                }
            }
            pauseRadio = false;
            foundRadio = radio -> Found();
            if ( changeRadio != 0 ) {
                radio -> SwitchCursor( changeRadio );
                radio -> Play();
                changeRadio = 0;
            } else if ( radio -> AutoPlay() ) {
                printf( "Now playing: %s\n", radio -> GetCurrentPath() );
            }
            Sleep( 250 );
        }
    }
    radioEstablished = true;
    delete radio;
    return 0;
}



