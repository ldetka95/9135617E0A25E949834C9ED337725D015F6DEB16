#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "Stringlinker.h"
#include "Map.h"
#include "Blocks.h"

#include "TreeModel.h"

/*#define CURRENT_INCLINATION 0
#define CURRENT_AZIMUTH 1
#define MAX_INCLINATION 2
#define MAX_AZIMUTH 3
#define DIR_INCLINATION 4
#define DIR_AZIMUTH 5
#define SPD_INCLINATION 6
#define SPD_AZIMUTH 7
#define TEXTURE_INDEX 8
#define COLLISION_RADIUS 9
#define BOOL_MODEL_HEAD 10
#define STD_INCLINATION 11
#define STD_AZIMUTH 12

#define MODELBLOCK_TOTAL_VAR 13*/

/*struct ___ModelBlock {
    public:
    Block* block;
    Point3D offset, hotspot;
    double var[ MODELBLOCK_TOTAL_VAR ];
};

typedef struct ___ModelBlock ModelBlock;*/

void CleanBlock( ModelBlock* m ) {
    m -> block = NULL;
    m -> offset = Point( 0.0, 0.0, 0.0 );
    m -> hotspot = Point( 0.0, 0.0, 0.0 );
    for ( int i = 0; i < MODELBLOCK_TOTAL_VAR; i++ ) {
        m -> var[ i ] = 0.0;
    }
}

class Model {
    public:
    Model( int initial ) {
        mArray = NULL;
        blockNum = 0;
        Resize( initial );
    }
    ~Model() {
        if ( mArray ) {
            free( mArray );
        }
    }
    void Resize( int newNum ) {
        if ( newNum > 0 ) {
            mArray = ( ModelBlock (*)[] )( realloc( mArray, sizeof( ModelBlock ) * newNum ) );
            for ( int i = blockNum; i < newNum; i++ ) {
                CleanBlock( &( *mArray )[ i ] );
            }
        } else {
            free( mArray );
            mArray = NULL;
        }
        blockNum = newNum;
    }
    int Length() {
        return blockNum;
    }
    void Set( int index, ModelBlock modelBlock ) {
        ( *mArray )[ index ] = modelBlock;
    }
    ModelBlock Get( int index ) {
        return ( *mArray )[ index ];
    }
    void SetVar( int index, int varIndex, double value ) {
        ( *mArray )[ index ].var[ varIndex ] = value;
    }
    double GetVar( int index, int varIndex ) {
        return ( *mArray )[ index ].var[ varIndex ];
    }
    void LinkBlock( int index, Block* block ) {
        ( *mArray )[ index ].block = block;
    }
    bool FromFile( const char* path ) {
        FILE* handle = fopen( path, "r" );
        if ( handle ) {
            double count;
            fscanf( handle, "%lf ", &count );
            Resize( count );
            for ( int i = 0; i < ( int )( count ); i++ ) {
                ModelBlock mblock;
                CleanBlock( &mblock );
                Point3D r;
                fscanf( handle, "%lf %lf %lf\n", &r.x, &r.y, &r.z );
                Block* blockLink = new Block( r.x, r.y, r.z );
                mblock.block = blockLink;
                fscanf( handle, "%lf %lf %lf\n", &r.x, &r.y, &r.z );
                mblock.offset = r;
                double textureID, isHead;
                fscanf( handle, "%lf %lf\n", &textureID, &isHead );
                mblock.var[ TEXTURE_INDEX ] = textureID;
                mblock.var[ BOOL_MODEL_HEAD ] = isHead;
                fscanf( handle, "%lf %lf\n", &r.x, &r.y );
                mblock.var[ STD_INCLINATION ] = r.x;
                mblock.var[ STD_AZIMUTH ] = r.y;
                fscanf( handle, "%lf %lf %lf\n", &r.x, &r.y, &r.z );
                mblock.var[ MAX_INCLINATION ] = r.x;
                mblock.var[ DIR_INCLINATION ] = r.y;
                mblock.var[ SPD_INCLINATION ] = r.z;
                fscanf( handle, "%lf %lf %lf\n", &r.x, &r.y, &r.z );
                mblock.var[ MAX_AZIMUTH ] = r.x;
                mblock.var[ DIR_AZIMUTH ] = r.y;
                mblock.var[ SPD_AZIMUTH ] = r.z;
                fscanf( handle, "%lf\n", &r.x );
                mblock.var[ COLLISION_RADIUS ] = r.x;
                Set( i, mblock );
            }
            fclose( handle );
            return true;
        }
        return false;
    }
    void Model_Start( Point3D pos, Point3D lookAt, double rotatedX, double rotatedY, double rotatedZ ) {
        glPushMatrix();
        //glLoadIdentity();
        glTranslated( pos.x, pos.y, pos.z );
        modelRotation.x = rotatedX;
        modelRotation.y = rotatedY;
        modelRotation.z = rotatedZ;
        modelPos = pos;
        modelLookAt = lookAt;
        /*glRotated( rotatedY, 0.0, 1.0, 0.0 );
        glRotated( rotatedX, 1.0, 0.0, 0.0 );
        glRotated( rotatedZ, 0.0, 0.0, 1.0 );*/
        glColor3f( MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
    }
    void Model_Draw( double entitySpeedMul ) {
        for ( int i = 0; i < blockNum; i++ ) {
            ModelBlock block = ( *mArray )[ i ];
            int textureUsed = ( int )( block.var[ TEXTURE_INDEX ] );
            if ( image[ textureUsed ] ) {
                if ( image[ textureUsed ] -> UseImage() ) {
                    //glPushMatrix();
                    //glTranslated( -block.offset.x, block.offset.y, -block.offset.z );
                    Point3D modelRelLook = SubtractPoint( SubtractPoint( modelLookAt, modelPos ), block.offset );
                    ( block.block ) -> DrawRotated_JoinModel( block, modelRotation, modelRelLook, entitySpeedMul );
                    //glPopMatrix();
                }
            }
        }
    }
    void Model_End() {
        glPopMatrix();
    }
    //void Display( Point3D pos, Point3D posDir, double entitySpeedMul );
    void Display( Point3D pos, Point3D posDir, double entitySpeedMul ) {
        double look_inclination, look_azimuth;
        Point3D look_dif = SubtractPoint( posDir, pos );
        cts( look_dif.x, look_dif.y, look_dif.z, NULL, &look_inclination, &look_azimuth );
        Model_Start( pos, posDir, degr( look_inclination ) - 90, 90 - degr( look_azimuth ), 0 );
        Model_Draw( entitySpeedMul );
        Model_End();
    }
    double GetLowestY() {
        if ( blockNum <= 0 ) {
            return 0.0;
        }
        double ret = ( *mArray )[ 0 ].block -> LowestY();
        for ( int i = 1; i < blockNum; i++ ) {
            double r1 = ( *mArray )[ i ].block -> LowestY();
            if ( r1 < ret ) {
                ret = r1;
            }
        }
        return ret;
    }
    private:
    ModelBlock ( *mArray )[];
    int blockNum;
    Point3D modelRotation;
    Point3D modelLookAt;
    Point3D modelPos;
};

#define MAX_MODELS 256

Model* mainModel[ MAX_MODELS ];


