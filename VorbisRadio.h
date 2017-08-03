#pragma once

#include "Audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string>
#include <list>

using namespace std;

class VorbisRadio {
    public:
    VorbisRadio() {
        ___sem = CreateSemaphore( NULL, 1, 1, NULL );
        detect( "data/MyMusic/" );
        ___done = false;
        ___thread = CreateThread( NULL, 0, ___radioThreadFunc, this, 0, NULL );
    }
    ~VorbisRadio() {
        ___done = true;
        WaitForSingleObject( ___thread, INFINITE );
        CloseHandle( ___thread );
    }
    int detect( string rootDir ) {
        ___lock();
        StopSound( CHANNEL_RADIO );
        ___trackList.clear();
        int found = ___recursiveDetect( rootDir, ".ogg" );
        ___mashup_nolock();
        if ( found > 0 ) {
            PlaySoundAbsolute( CHANNEL_RADIO, ___trackList.front() );
        }
        ___unlock();
        return found;
    }
    void mashup() {
        ___lock();
        ___mashup_nolock();
        ___unlock();
    }
    private:
    int ___recursiveDetect( string currentDir, string ext ) {
        int found = 0;
        WIN32_FIND_DATA findData;
        HANDLE hFind = FindFirstFile( ( currentDir + "*" ).c_str(), &findData );
        if ( hFind != INVALID_HANDLE_VALUE ) {
            do {
                string fn = findData.cFileName;
                if ( ( fn != ".." ) && ( fn != "." ) ) {
                    if ( findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY ) {
                        found += ___recursiveDetect( currentDir + fn + "/", ext );
                    } else if ( fn.substr( fn.length() - ext.length(), ext.length() ) == ext ) {
                        const char* ccopy = ( currentDir + fn ).c_str();
                        printf( " > Found file: %s\n", ccopy );
                        int len = strlen( ccopy ) + 1;
                        char* cchar = ( char* )( malloc( len ) );
                        for ( int i = 0; i < len; i++ ) {
                            cchar[ i ] = ccopy[ i ];
                        }
                        ___trackList.push_back( cchar );
                        found++;
                    }
                }
            } while ( FindNextFile( hFind, &findData ) != 0 );
            FindClose( hFind );
        }
        return found;
    }
    void ___run() {
        while ( !___done ) {
            if ( !IsPlaying( CHANNEL_RADIO ) ) {
                ___lock();
                if ( ___trackList.size() > 0 ) {
                    char* pushedTrack = ___trackList.front();
                    ___trackList.pop_front();
                    ___trackList.push_back( pushedTrack );
                    PlaySoundAbsolute( CHANNEL_RADIO, ___trackList.front() );
                }
                ___unlock();
            }
            Sleep( 100 );
        }
        while ( ___trackList.size() > 0 ) {
            char* track = ___trackList.front();
            ___trackList.pop_front();
            free( track );
        }
    }
    void ___lock() {
        WaitForSingleObject( ___sem, INFINITE );
    }
    void ___unlock() {
        ReleaseSemaphore( ___sem, 1, NULL );
    }
    void ___mashup_nolock() {
        int s = ___trackList.size();
        if ( s > 1 ) {
            char** tl_array = ( char** )( malloc( sizeof( char* ) * s ) );
            for ( int i = 0; i < s; i++ ) {
                tl_array[ i ] = ___trackList.front();
                ___trackList.pop_front();
            }
            for ( int i = 0; i < ( s >> 1 ) + 1; i++ ) {
                int b = rand() % ( s - 1 );
                if ( b >= i ) {
                    b++;
                }
                char* tmp = tl_array[ i ];
                tl_array[ i ] = tl_array[ b ];
                tl_array[ b ] = tmp;
                /*for ( int i = 0; i < s; i++ ) {
                    printf( " > %d : %s\n", i, tl_array[ i ] );
                }*/
            }
            for ( int i = 0; i < s; i++ ) {
                ___trackList.push_back( tl_array[ i ] );
            }
            free( tl_array );
        }
    }
    list< char* > ___trackList;
    volatile bool ___done;
    HANDLE ___thread;
    HANDLE ___sem;
    static WINAPI DWORD ___radioThreadFunc( LPVOID param ) {
        ( ( VorbisRadio* )( param ) ) -> ___run();
        return 0;
    }
};
