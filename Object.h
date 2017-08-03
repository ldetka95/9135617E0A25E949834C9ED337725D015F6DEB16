#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "Model.h"

#define ENTITY_ALERT_RANGE_DEFAULT 3.0

#define STD_BUFFER_SIZE 256

#define MAX_ENT_VAR 64
#define VAR_HP 0
#define VAR_MAX_HP 1
#define VAR_SPEED 2
#define VAR_DMG_MIN 3
#define VAR_DMG_MAX 4
#define VAR_LOGIC 5
#define VAR_HEAR_DIST 6
#define VAR_HEAR_RUN_DIST 7
#define VAR_COLLISION_RADIUS 8
#define VAR_WALKER_ANXIETY 9
#define VAR_WALKER_ALLOWEDRADIUS 10
#define VAR_ENTITY_ALERT_RANGE 11
#define VAR_FLESHSIZE_MIN 12
#define VAR_FLESHSIZE_MAX 13
#define VAR_FLESHCOUNT_MIN 14
#define VAR_FLESHCOUNT_MAX 15
#define VAR_DEATH_SOUND 16
#define VAR_ENTITY_KAYLEY_STATE 32
#define VAR_ENTITY_KAYLEY_DISTSTATE 33
#define VAR_KAYLEY_DISTMIN 34
#define VAR_KAYLEY_DISTMAX 35
#define VAR_KAYLEY_STATECHANGETIMEMIN 36
#define VAR_KAYLEY_STATECHANGETIMEMAX 37
#define VAR_ENTITY_HORSE_STATE 38
#define VAR_ENTITY_DAMAGED 39
#define VAR_ENTITY_MOVE_SPEED_XZ 40
#define VAR_ENTITY_SEEN 41

class Object {
    protected:
    Object() {
        ___Init();
    }
    public:
    Object( const char* fileName ) {
        ___Init();
        FILE* handle = fopen( fileName, "r" );
        //printf( "Opened: [%s]\n", fileName );
        if ( handle ) {
            double mID;
            fscanf( handle, "%lf", &mID );
            //printf( " > Scanned mID: %lf\n", mID );
            if ( ( mID >= 0 ) && ( mID < MAX_MODELS ) ) {
                model = mainModel[ int( mID ) ];
            } else {
                model = NULL;
            }
            for ( int i = 0; i < MAX_ENT_VAR; i++ ) {
                varInit[ i ] = 0.0;
                fscanf( handle, "%lf", &varInit[ i ] );
                //printf( " > Scanned: %d/%lf\n", i, varInit[ i ] );
            }
            fclose( handle );
        }/* else {
            printf( "Cannot open: [%s]\n", fileName );
        }*/
    }
    static Object* loadObject( const char* fileName ) {
        FILE* handle = fopen( fileName, "rb" );
        if ( handle ) {
            Object* o = new Object();
            int mID;
            fread( &mID, sizeof( int ), 1, handle );
            fread( o -> varInit, sizeof( varInit ), 1, handle );
            if ( ( mID >= 0 ) && ( mID < MAX_MODELS ) ) {
                o -> model = mainModel[ mID ];
            } else {
                o -> model = NULL;
            }
            fclose( handle );
            return o;
        }
        return NULL;
    }
    ~Object() {
    }
    Model* GetModel() {
        return model;
    }
    double GetVar( int index ) {
        return varInit[ index ];
    }
    private:
    void ___Init() {
        model = NULL;
        for ( int i = 0; i < MAX_ENT_VAR; i++ ) {
            varInit[ i ] = 0.0;
        }
        varInit[ VAR_MAX_HP ] = 100.0;
    }
    double varInit[ MAX_ENT_VAR ];
    Model* model;
};

#define MAX_OBJECT 64
Object* object[ MAX_OBJECT ];

#define PLAYER_OBJECT_ID 63 // special ID!

/// CUSTOM

void LoadObjects( const char* path ) {
    StringLinker* slinker = new StringLinker( path );
    for ( int i = 0; i < slinker -> GetSize(); i++ ) {
        object[ slinker -> GetIdOf( i ) ] = new Object( ( slinker -> GetStringOf( i ) ).c_str() );
    }
    delete slinker;
}
