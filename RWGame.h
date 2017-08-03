#pragma once

#include "Core.h"

/// custom R/W

void ___WriteInt( FILE* handle, int value ) {
    int v = value;
    fwrite( &v, sizeof( int ), 1, handle );
}

int ___ReadInt( FILE* handle ) {
    int ret = 0;
    fread( &ret, sizeof( int ), 1, handle );
    return ret;
}

void ___WriteDouble( FILE* handle, double value ) {
    double v = value;
    fwrite( &v, sizeof( double ), 1, handle );
}

double ___ReadDouble( FILE* handle ) {
    double ret = 0;
    fread( &ret, sizeof( double ), 1, handle );
    return ret;
}

Point3D ___ReadPoint3D( FILE* handle ) {
    Point3D ret = Point( 0.0, 0.0, 0.0 );
    fread( &ret.x, sizeof( double ), 1, handle );
    fread( &ret.y, sizeof( double ), 1, handle );
    fread( &ret.z, sizeof( double ), 1, handle );
    return ret;
}

void ___WritePoint3D( FILE* handle, Point3D p ) {
    fwrite( &p.x, sizeof( double ), 1, handle );
    fwrite( &p.y, sizeof( double ), 1, handle );
    fwrite( &p.z, sizeof( double ), 1, handle );
}

void ___ReadMonsterStorage( FILE* handle ) {
    int mobCount = ___ReadInt( handle );
    for ( int i = 0; i < mobCount; i++ ) {
        int id = ___ReadInt( handle );
        //monsterStorage[ id ] -> UpdateHotSpot(
    }
}

void ___WriteMonsterStorage( FILE* handle ) {
    int mobCount = 0;
    for ( int i = 0; i < MAX_MONSTERS; i++ ) {
        if ( monsterStorage[ i ] ) {
            mobCount++;
        }
    }
    ___WriteInt( handle, mobCount );
    for ( int i = 0; i < MAX_MONSTERS; i++ ) {
        if ( monsterStorage[ i ] ) {
            //monsterStorage[ i ]
        }
    }
}

/// Game load/save

void LoadGameState( string path ) {
    FILE* handle = fopen( path.c_str(), "rb" );
    if ( handle ) {
        player -> UpdateHotSpot( ___ReadPoint3D( handle ) );
        mainCamHandle -> SetAngles( ___ReadPoint3D( handle ) );
        fclose( handle );
    }
}

void SaveGameState( string path ) {
    FILE* handle = fopen( path.c_str(), "wb" );
    if ( handle ) {
        ___WritePoint3D( handle, player -> HotSpot() );
        ___WritePoint3D( handle, mainCamHandle -> GetAngles() );
        fclose( handle );
    }
}

