#pragma once

#include <winsock.h>
#include <windows.h>
#include <math.h>
#include <map>

#include "MPStates.h"

sockaddr_in createService( const char* IP, int port ) {
    sockaddr_in svc;
    memset( &svc, 0, sizeof( svc ) );
    svc.sin_family = AF_INET;
    svc.sin_addr.s_addr = inet_addr( IP );
    svc.sin_port = htons( port );
    return svc;
}

class Thread {
    public:
    Thread() {
        ___thread = 0;
        ___done = false;
    }
    virtual ~Thread() {
        if ( ___thread ) {
            CloseHandle( ___thread );
        }
    }
    virtual void run() = 0;
    void start() {
        ___thread = CreateThread( NULL, 0, ___run, this, 0, NULL );
    }
    void join() {
        WaitForSingleObject( ___thread, INFINITY );
    }
    void stop() {
        ___done = true;
    }
    bool isTerminated() {
        return ___done;
    }
    private:
    static DWORD WINAPI ___run( LPVOID param ) {
        ( ( Thread* ) param ) -> run();
        return 0;
    }
    HANDLE ___thread;
    volatile bool ___done;
};

class Connector : public Thread {
    public:
    Connector( const char* IP, int port ) : Thread() {
        ___connected = false;
        WSAStartup( MAKEWORD( 2, 2 ), &___wsadata );
        ___svc = createService( IP, port );
        ___socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        ___queuemutex = CreateMutex( NULL, FALSE, NULL );
        if ( connect( ___socket, ( SOCKADDR* )( &___svc ), sizeof( ___svc ) ) < 0 ) {
            printf( "Cannot connect to %s:%d.\n", IP, port );
        } else {
            ___connected = true;
            start();
        }
    }
    ~Connector() {
        closesocket( ___socket );
        CloseHandle( ___queuemutex );
        WSACleanup();
    }
    void run() {
        while ( !isTerminated() ) {
            char buffer[ MAX_PACKET_SIZE + 16 ];
            int n;
            int* bInt = ( int* )( buffer );
            if ( ( n = recv( ___socket, buffer, MAX_PACKET_SIZE, 0 ) ) > 0 ) {
                Message* msg = new Message( n - ( sizeof( int ) << 1 ), bInt[ 1 ], buffer + ( sizeof( int ) << 1 ) );
                if ( msg -> getCode() == MESSAGE_CODE_MAP_PART ) {
                    //if ( MainMap ) {
                        MapPart* part = ( MapPart* )( msg -> getData() );
                        int start = part -> begin;
                        int end = start + part -> length;// / sizeof( int );
                        int* data = part -> data;
                        int* mapData = MainMap -> GetRawData();
                        for ( int i = start; i < end; i++ ) {
                            mapData[ i ] = data[ i - start ];
                        }
                    //}
                    delete msg;
                } else if ( msg -> getCode() == MESSAGE_CODE_MAP_PROPERTIES ) {
                    MapProperties* prop = ( MapProperties* )( msg -> getData() );
                    MainMap = new Map( prop -> sizeX, prop -> sizeY, prop -> sizeZ, prop -> blockSize );
                    delete msg;
                } else {
                    lockQueue();
                    ___input.push_back( msg );
                    unlockQueue();
                }
            }
            if ( n < 0 ) {
                printf( "Connector.run(): connection broken [%d].\n", WSAGetLastError() );
                break;
            }
        }
    }
    void lockQueue() {
        WaitForSingleObject( ___queuemutex, INFINITY );
    }
    void unlockQueue() {
        ReleaseMutex( ___queuemutex );
    }
    Message* nextMessage() {
        if ( !___input.empty() ) {
            Message* msg = ___input.front();
            ___input.pop_front();
            return msg;
        }
        return NULL;
    }
    bool directSend( Message* message ) {
        int msgsize = message -> getSize() + ( sizeof( int ) << 1 );
        int n = send( ___socket, ( const char* )( message ), msgsize, 0 );
        return ( n == msgsize );
    }
    bool isConnected() {
        return ___connected;
    }
    private:
    SOCKET ___socket;
    sockaddr_in ___svc;
    WSADATA ___wsadata;
    list < Message* > ___input;
    HANDLE ___queuemutex;
    bool ___connected;
};

Connector* connector = NULL;

class EntityMap {
    public:
    EntityMap() {
    }
    ~EntityMap() {
        map< int, Entity* >::iterator it = ___map.begin();
        while ( it != ___map.end() ) {
            Entity* e = it -> second;
            if ( e != player ) {
                delete e;
            }
            it++;
        }
    }
    void add( int id, Entity* ent ) {
        ___map[ id ] = ent;
    }
    Entity* get( int id ) {
        map< int, Entity* >::iterator found = ___map.find( id );
        if ( found != ___map.end() ) {
            return found -> second;
        }
        return NULL;
    }
    void remove( int id ) {
        ___map.erase( id );
    }
    void flush() {
        ___map.clear();
    }
    map < int, Entity* >::iterator getIterator() {
        return ___map.begin();
    }
    bool allIterated( map < int, Entity* >::iterator it ) {
        return ( it == ___map.end() );
    }
    private:
    map < int, Entity* > ___map;
};

EntityMap* playersOnServer = new EntityMap();

