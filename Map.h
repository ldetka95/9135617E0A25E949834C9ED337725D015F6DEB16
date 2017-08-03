#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include <string>

#include "Lighter.h"
#include "Textures.h"

#include "BlockProperties.h"
#include "Water.h"

double FOG_START_DIST = 6.0;
double FOG_END_DIST = 10.0;

using namespace std;

float FogColor[ 3 ] = { 0.0, 0.0, 0.0 };
float MainColor[ 3 ] = { 1.0, 1.0, 1.0 };
float ExMulStartColor[ 3 ] = { 1.0, 1.0, 1.0 };
float ExMulFinalColor[ 3 ] = { 1.0, 1.0, 1.0 };
float DeathColor[ 3 ] = { 1.0, 0.0, 0.0 };
int fogOverSky = 0;
int skyTex = 255;
double skyHeight = 3.0;
double skyHorizon = 24.0;

Point3D shootDefaultLightColor = Point( 2.6, 2.0, 0.4 );
Point3D shootCurrentLightColor;
Point3D shootLightColorOffset;
double shootLight_TimeInertia = 0.75;

#define LIGHT_SHOOT_ASSIGNED 2
Lighter* shootLighter = NULL;

void ShootLight( Point3D pos, Point3D dst ) {
    if ( !shootLighter ) {
        shootLighter = new Lighter( LIGHT_SHOOT_ASSIGNED );
        shootCurrentLightColor = Point( 0.0, 0.0, 0.0 );
        shootLightColorOffset = Point( 0.0, 0.0, 0.0 );
    }
    shootLighter -> FlashPrepare( AddPoint( pos, shootLightColorOffset ), dst, shootCurrentLightColor, 180.0 );
    glLightf( GL_LIGHT0 + LIGHT_SHOOT_ASSIGNED, GL_QUADRATIC_ATTENUATION, 0.4 );
    shootCurrentLightColor = Point( shootCurrentLightColor.x * shootLight_TimeInertia, shootCurrentLightColor.y * shootLight_TimeInertia, shootCurrentLightColor.z * shootLight_TimeInertia );
}

void FullShootLight( Point3D offset ) {
    shootCurrentLightColor = shootDefaultLightColor;
    shootCurrentLightColor.x *= MainColor[ 0 ];
    shootCurrentLightColor.y *= MainColor[ 1 ];
    shootCurrentLightColor.z *= MainColor[ 2 ];
    shootLightColorOffset = offset;
}

void ResetMainColor() {
    for ( int i = 0; i < 3; i++ ) {
        MainColor[ i ] = 0.0;
        ExMulFinalColor[ i ] = ExMulStartColor[ i ];
    }
}

void DriveMainColor( double colorEffectInertia ) {
    for ( int i = 0; i < 3; i++ ) {
        MainColor[ i ] = MainColor[ i ] * colorEffectInertia + ExMulFinalColor[ i ] * ( 1.0 - colorEffectInertia );
    }
}

void SetDeathColorLevel( double percentageAlive ) {
    for ( int i = 0; i < 3; i++ ) {
        ExMulFinalColor[ i ] = percentageAlive * ExMulStartColor[ i ] + ( 1.0 - percentageAlive ) * DeathColor[ i ];
    }
}

#define CHUNK_SIZE 16

double level_Gravity = 1.0;

#define STD_NORMAL_VECTOR_COUNT 6
Point3D MAP_NORMAL_VECTOR[ STD_NORMAL_VECTOR_COUNT ] = {
    Point( -1.0, 0.0, 0.0 ),
    Point( 1.0, 0.0, 0.0 ),
    Point( 0.0, -1.0, 0.0 ),
    Point( 0.0, 1.0, 0.0 ),
    Point( 0.0, 0.0, -1.0 ),
    Point( 0.0, 0.0, 1.0 )
};

class Map {
    public:
    Map( int sizeX, int sizeY, int sizeZ, double blockSize ) {
        data = NULL;
        sX = sizeX;
        sY = sizeY;
        sZ = sizeZ;
        sizeT = sizeX * sizeY * sizeZ;
        if ( sizeT ) {
            data = ( int* )( calloc( sizeT, sizeof( int ) ) );
        }
        bSize = blockSize;
        hBlockSize = bSize / 2.0;
        pathR = "";
        mrInfo = new MapRenderInfo( sizeX, sizeY, sizeZ );
        mrInfo -> ___Init_Sides( this );
        oplistSize = ( sX * sZ ) / ( CHUNK_SIZE * CHUNK_SIZE );
        oplist = ( OptimizedPointList* (*)[] )( calloc( sizeof( OptimizedPointList* ), oplistSize ) );
        lastChunkUpdated = -1;
        joinedTex = new Bitmap();
        water = Water::CreateNull();
    }
    Map( string pathRoot ) {
        pathR = pathRoot;
        string pathProp = pathRoot + "properties.dat";
        string pathMain = pathRoot + "level.dat";
        string pathWater = pathRoot + "water.dat";
        FILE* handle = fopen( pathProp.c_str(), "r" );
        if ( handle ) {
            fscanf( handle, "%d %d %d\n%lf\n", &sX, &sY, &sZ, &bSize );
            fscanf( handle, "%lf %lf %lf %lf %lf\n", &spawnX, &spawnY, &spawnZ, &angleXZ, &angleY );
            fscanf( handle, "%f %f %f\n", &FogColor[ 0 ], &FogColor[ 1 ], &FogColor[ 2 ] );
            fscanf( handle, "%f %f %f\n", &ExMulStartColor[ 0 ], &ExMulStartColor[ 1 ], &ExMulStartColor[ 2 ] );
            fscanf( handle, "%d %d %lf %lf\n", &skyTex, &fogOverSky, &skyHeight, &skyHorizon );
            fscanf( handle, "%lf %lf\n", &FOG_START_DIST, &FOG_END_DIST );
            fscanf( handle, "%lf\n", &level_Gravity );
            fclose( handle );
        } else { // not found?
            sX = 256;
            sY = 32;
            sZ = 256;
            bSize = 1.0;
        }
        sizeT = sX * sY * sZ;
        data = NULL;
        if ( sizeT ) {
            data = ( int* )( calloc( sizeT, sizeof( int ) ) );
        }
        FILE* handleMap = fopen( pathMain.c_str(), "rb" );
        if ( handleMap ) {
            fread( data, sizeT, sizeof( int ), handleMap );
            fclose( handleMap );
        } else { // not found?
            Construct_TestMap( 4, 1, 1 );
        }
        hBlockSize = bSize / 2.0;
        mrInfo = new MapRenderInfo( sX, sY, sZ );
        mrInfo -> ___Init_Sides( this );
        oplistSize = ( sX * sZ ) / ( CHUNK_SIZE * CHUNK_SIZE );
        oplist = ( OptimizedPointList* (*)[] )( calloc( sizeof( OptimizedPointList* ), oplistSize ) );
        lastChunkUpdated = -1;
        joinedTex = new Bitmap();
        water = Water::Create( pathWater );
        if ( !water ) {
            water = Water::CreateNull();
        }
    }
    ~Map() {
        if ( data ) {
            free( data );
        }
        delete mrInfo;
        for ( int i = 0; i < oplistSize; i++ ) {
            if ( ( *oplist )[ i ] ) {
                delete ( *oplist )[ i ];
            }
        }
        free( oplist );
        delete joinedTex;
        delete water;
    }
    void SaveMap() {
        if ( pathR != "" ) {
            string pathProp = pathR + "properties.dat";
            string pathMain = pathR + "level.dat";
            string pathWater = pathR + "water.dat";
            FILE* handle = fopen( pathProp.c_str(), "w" );
            if ( handle ) {
                fprintf( handle, "%d %d %d\n%lf\n", sX, sY, sZ, bSize );
                fprintf( handle, "%lf %lf %lf %lf %lf\n", spawnX, spawnY, spawnZ, angleXZ, angleY );
                fprintf( handle, "%f %f %f\n", FogColor[ 0 ], FogColor[ 1 ], FogColor[ 2 ] );
                fprintf( handle, "%f %f %f\n", ExMulStartColor[ 0 ], ExMulStartColor[ 1 ], ExMulStartColor[ 2 ] );
                fprintf( handle, "%d %d %lf %lf\n", skyTex, fogOverSky, skyHeight, skyHorizon );
                fprintf( handle, "%lf %lf\n", FOG_START_DIST, FOG_END_DIST );
                fprintf( handle, "%lf\n", level_Gravity );
                fclose( handle );
            }
            FILE* handleMap = fopen( pathMain.c_str(), "wb" );
            if ( handleMap ) {
                fwrite( data, sizeT, sizeof( int ), handleMap );
                fclose( handleMap );
            }
            water -> trySave( pathWater );
        }
    }
    void RepairMap() {
        for ( int ix = 0; ix < sX; ix++ ) {
            for ( int iy = 0; iy < sY; iy++ ) {
                for ( int iz = 0; iz < sZ; iz++ ) {
                    /*int id = GetID( ix, iy, iz );
                    bool rep = false;
                    if ( ( id > 0 ) && ( id < MAX_TEXTURES ) ) {
                        if ( !image[ id ] ) {
                            rep = true;
                        }
                    } else if ( id >= MAX_TEXTURES ) {
                        rep = true;
                    }
                    if ( rep ) {
                        printf( "Repaired %d (%d, %d, %d)\n", id, ix, iy, iz );
                        SetID( ix, iy, iz, 0 );
                        UpdateSides( ix, iy, iz );
                    }*/
                    UpdateSides( ix, iy, iz );
                }
            }
        }
        RebuildChunks();
    }
    /*void finalizeMap() {
        mrInfo -> ___Init_Sides( this );
    }*/
    string GetMapPath() {
        return pathR;
    }
    inline int GetID( int x, int y, int z ) {
        int retID = -1;
        if ( ( x >= 0 ) && ( y >= 0 ) && ( z >= 0 ) && ( x < sX ) && ( y < sY ) && ( z < sZ ) ) {
            int* index = ( int* )( int( data ) + ( z * sX * sY + y * sX + x ) * sizeof( int ) );
            retID = ( *index );
        }
        return retID;
    }
    inline void SetID( int x, int y, int z, int ID ) {
        if ( ( x >= 0 ) && ( y >= 0 ) && ( z >= 0 ) && ( x < sX ) && ( y < sY ) && ( z < sZ ) ) {
            int* index = ( int* )( int( data ) + ( z * sX * sY + y * sX + x ) * sizeof( int ) );
            ( *index ) = ID;
        }
    }
    int GetX() {
        return sX;
    }
    int GetY() {
        return sY;
    }
    int GetZ() {
        return sZ;
    }
    double GetBlockSize() {
        return bSize;
    }
    int* GetRawData() {
        return data;
    }
    void Construct_TestMap( int Ylevel, int minTex, int maxTex ) {
        int randRange = maxTex - minTex + 1;
        for ( int iz = 0; iz < sZ; iz++ ) {
            for ( int ix = 0; ix < sX; ix++ ) {
                int textureID = rand() % randRange + minTex;
                SetID( ix, Ylevel, iz, textureID );
                if ( ( ix == 0 ) || ( iz == 0 ) || ( ix == sX - 1 ) || ( iz == sZ - 1 ) ) {
                    SetID( ix, Ylevel + 1, iz, textureID );
                    SetID( ix, Ylevel + 2, iz, textureID );
                }
            }
        }
    }
    void UpdateSpawn( double x, double y, double z, double axz, double ay ) {
        spawnX = x;
        spawnY = y;
        spawnZ = z;
        angleXZ = axz;
        angleY = ay;
    }
    double GetSpawnX() {
        return spawnX;
    }
    double GetSpawnY() {
        return spawnY;
    }
    double GetSpawnZ() {
        return spawnZ;
    }
    double GetSpawnAngleXZ() {
        return angleXZ;
    }
    double GetSpawnAngleY() {
        return angleY;
    }
    Point3D GetSpawn() {
        return Point( spawnX, spawnY, spawnZ );
    }
    void StdTextureParam( int w, int h, int count ) {
        joinedTex -> SetDim( w * count, h, 4 );
        tex_stdw = w;
        tex_stdh = h;
        tcount = count;
    }
    void AssignTexture( int id, Bitmap* bmp ) {
        joinedTex -> CopyFrom_32( bmp, 0, 0, tex_stdw * id, 0, tex_stdw, tex_stdh );
    }
    void JoinTextures() {
        jTgl = joinedTex -> GL_Bitmap();
        //joinedTex -> SaveBitmap( "d.bmp" );
    }
    /*inline void Display( int ix, int iy, int iz ) {
        int idDisp = GetID( ix, iy, iz );
        if ( ( idDisp <= 0 ) || ( idDisp > 255 ) ) {
            if ( idDisp > 255 ) {
                printf( "DISPLAY ALERT: x = %d, y = %d, z = %d :: texture ID = %d, out of bounds!\n", ix, iy, iz, idDisp );
            }
            return;
        }
        if ( image[ idDisp ] ) {
            image[ idDisp ] -> UseImage();
            double px = ix * bSize + hBlockSize;
            double py = iy * bSize + hBlockSize;
            double pz = iz * bSize + hBlockSize;
            SideCube( px, py, pz, bSize, idDisp, mrInfo -> Get( ix, iy, iz ) );
        }
    }*/
    void UpdateSides( int x, int y, int z ) {
        mrInfo -> Update( this, x, y, z );
        mrInfo -> Update( this, x - 1, y, z );
        mrInfo -> Update( this, x + 1, y, z );
        mrInfo -> Update( this, x, y - 1, z );
        mrInfo -> Update( this, x, y + 1, z );
        mrInfo -> Update( this, x, y, z - 1 );
        mrInfo -> Update( this, x, y, z + 1 );
    }
    int lastChunkUpdated;
    void UpdateChunk( int x, int y, int z ) {
        if ( ( x < 0 ) || ( x >= sX ) || ( z < 0 ) || ( z >= sZ ) ) {
            return;
        }
        int iZ = z / CHUNK_SIZE;
        int iX = x / CHUNK_SIZE;
        int i = iZ * ( sZ / CHUNK_SIZE ) + iX;
        if ( lastChunkUpdated == i ) { // there is no need to update it
            return;
        }
        if ( ( *oplist )[ i ] ) {
            delete ( *oplist )[ i ];
        }
        ( *oplist )[ i ] = new OptimizedPointList( this, iX * CHUNK_SIZE, iZ * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE );
    }
    void RebuildChunks() {
        for ( int j = 0; j < sZ; j += CHUNK_SIZE ) {
            for ( int i = 0; i < sX; i += CHUNK_SIZE ) {
                UpdateChunk( i, 0, j );
            }
        }
    }
    void TryDrawVisibleFields() {
        if ( mrInfo ) {
            mrInfo -> DrawVisibleFields( this, Point( spawnX, spawnY, spawnZ ) );
        }
    }
    void CreateOptimized() {
        if ( !oplist ) {
            return;
        }
        for ( int iz = 0; iz < sZ / CHUNK_SIZE; iz++ ) {
            for ( int ix = 0; ix < sX / CHUNK_SIZE; ix++ ) {
                int i = iz * ( sZ / CHUNK_SIZE ) + ix;
                ( *oplist )[ i ] = new OptimizedPointList( this, ix * CHUNK_SIZE, iz * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE );
            }
        }
    }
    void DisplayOptimized( Point3D pos, int chunkRange, double xzAngle ) {
        // xz angle must be in radians
        /*if ( water ) {
            water -> display( pos, 6 );
        }*/
        if ( !oplist ) {
            return;
        }
        glEnable( GL_TEXTURE_2D );
        glColor3f( MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
        //glColor3f( 0.0, 0.0, 0.0 );
        /*for ( int j = 1; j < MAX_TEXTURES; j++ ) {
            if ( image[ j ] ) {
                image[ j ] -> UseImage();
            } else {
                continue;
            }
            int borderZ = ( sZ / CHUNK_SIZE );
            int borderX = ( sX / CHUNK_SIZE );
            int xInit = ( pos.x / CHUNK_SIZE );
            int zInit = ( pos.z / CHUNK_SIZE );
            int xFrom = xInit - chunkRange;
            int xTo = xInit + chunkRange;
            int zFrom = zInit - chunkRange;
            int zTo = zInit + chunkRange;
            glBegin( GL_QUADS );
            for ( int iz = zFrom + 1; iz < zTo; iz++ ) {
                for ( int ix = xFrom + 1; ix < xTo; ix++ ) {
                    if ( ( ix < 0 ) || ( ix >= borderX ) || ( iz < 0 ) || ( iz >= borderZ ) ) {
                        continue;
                    }
                    int i = iz * borderX + ix;
                    ( *oplist )[ i ] -> Display_Local( j );
                }
            }
            glEnd();
        }*/
        joinedTex -> UseImage();
        int borderZ = ( sZ / CHUNK_SIZE );
        int borderX = ( sX / CHUNK_SIZE );
        int xInit = ( pos.x / CHUNK_SIZE );
        int zInit = ( pos.z / CHUNK_SIZE );
        int xFrom = xInit - chunkRange;
        int xTo = xInit + chunkRange;
        int zFrom = zInit - chunkRange;
        int zTo = zInit + chunkRange;
        // face culling
        glEnable( GL_CULL_FACE );
        // displayloop
        glBegin( GL_QUADS );
        // Basic render, for non-transparent textures.
        for ( int iz = zFrom + 1; iz < zTo; iz++ ) {
            for ( int ix = xFrom + 1; ix < xTo; ix++ ) {
                if ( ( ix < 0 ) || ( ix >= borderX ) || ( iz < 0 ) || ( iz >= borderZ ) ) {
                    continue;
                }
                // is even visible?
                /*double chunkAngle;
                ctp( xInit, zInit, NULL, &chunkAngle );
                if ( absf( xzAngle - chunkAngle ) > PI ) {
                    continue;
                }*/
                int i = iz * borderX + ix;
                for ( int j = 1; j < MAX_TEXTURES; j++ ) {
                    if ( !isTransparent( j ) ) {
                        ( *oplist )[ i ] -> Display_Local_Heap( j, tex_stdw, tcount );
                    }
                }
            }
        }
        // Second render - for transparent textures only. May need optimization.
        for ( int iz = zFrom + 1; iz < zTo; iz++ ) {
            for ( int ix = xFrom + 1; ix < xTo; ix++ ) {
                if ( ( ix < 0 ) || ( ix >= borderX ) || ( iz < 0 ) || ( iz >= borderZ ) ) {
                    continue;
                }
                int i = iz * borderX + ix;
                for ( int j = 1; j < MAX_TEXTURES; j++ ) {
                    if ( isTransparent( j ) ) {
                        ( *oplist )[ i ] -> Display_Local_Heap( j, tex_stdw, tcount );
                    }
                }
            }
        }
        glEnd();
        glDisable( GL_CULL_FACE );
    }
    static bool isTransparent( int id ) {
        bool transparent = false;
        double renderingTransparency = blockProperties -> getTransparency( id );
        if ( !isnan( renderingTransparency ) ) {
            if ( renderingTransparency > 1E-11 ) {
                transparent = true;
            }
        }
        return transparent;
    }
    static bool isSolid( int id ) {
        bool solid = ( id > 0 );
        double solidValue = blockProperties -> isSolid( id );
        if ( !isnan( solidValue ) ) {
            solid = ( solidValue > 1E-11 );
        }
        return solid;
    }
    void RemoveOptimized() {
        if ( !oplist ) {
            return;
        }
        for ( int i = 0; i < oplistSize; i++ ) {
            if ( ( *oplist )[ i ] ) {
                delete ( *oplist )[ i ];
                ( *oplist )[ i ] = NULL;
            }
        }
    }
    void* GetRenderInfo() {
        return mrInfo;
    }
    int OptimizedChunkCount() {
        return oplistSize;
    }
    void NoChunkUpdated() {
        lastChunkUpdated = -1;
    }
    private:
    class MapRenderInfo {
        public:
        MapRenderInfo( int sizeX, int sizeY, int sizeZ ) {
            sX = sizeX;
            sY = sizeY;
            sZ = sizeZ;
            sYsZ = sY * sZ;
            flag = ( char* )( calloc( sizeof( char ), sizeX * sizeY * sizeZ ) );
        }
        ~MapRenderInfo() {
            if ( flag ) {
                free( flag );
            }
        }
        void ___Init_Sides( Map* map ) {
            if ( !map ) {
                return;
            }
            for ( int ix = 0; ix < sX; ix++ ) {
                for ( int iy = 0; iy < sY; iy++ ) {
                    for ( int iz = 0; iz < sZ; iz++ ) {
                        Update( map, ix, iy, iz );
                    }
                }
            }
        }
        inline void Update( Map* map, int x, int y, int z ) {
            /*
                Update sides available for rendering
                    flag structure:
                    0 = x - 1
                    1 = x + 1
                    2 = y - 1
                    3 = y + 1
                    4 = z - 1
                    5 = z + 1
            */
            int id;
            if ( ( id = map -> GetID( x, y, z ) ) == 0 ) {
                Set( x, y, z, 0x00 );
            } else {
                char surrounding = 0x3F;
                if ( !isTransparent( id ) ) {
                    int idNextTo;
                    if ( ( idNextTo = map -> GetID( x - 1, y, z ) ) != 0 ) {
                        if ( !isTransparent( idNextTo ) ) {
                            surrounding ^= 0x01;
                        }
                    }
                    if ( ( idNextTo = map -> GetID( x + 1, y, z ) ) != 0 ) {
                        if ( !isTransparent( idNextTo ) ) {
                            surrounding ^= 0x02;
                        }
                    }
                    if ( ( idNextTo = map -> GetID( x, y - 1, z ) ) != 0 ) {
                        if ( !isTransparent( idNextTo ) ) {
                            surrounding ^= 0x04;
                        }
                    }
                    if ( ( idNextTo = map -> GetID( x, y + 1, z ) ) != 0 ) {
                        if ( !isTransparent( idNextTo ) ) {
                            surrounding ^= 0x08;
                        }
                    }
                    if ( ( idNextTo = map -> GetID( x, y, z - 1 ) ) != 0 ) {
                        if ( !isTransparent( idNextTo ) ) {
                            surrounding ^= 0x10;
                        }
                    }
                    if ( ( idNextTo = map -> GetID( x, y, z + 1 ) ) != 0 ) {
                        if ( !isTransparent( idNextTo ) ) {
                            surrounding ^= 0x20;
                        }
                    }
                } else {
                    if ( map -> GetID( x - 1, y, z ) == id ) {
                        surrounding ^= 0x01;
                    }
                    if ( map -> GetID( x + 1, y, z ) == id ) {
                        surrounding ^= 0x02;
                    }
                    if ( map -> GetID( x, y - 1, z ) == id ) {
                        surrounding ^= 0x04;
                    }
                    if ( map -> GetID( x, y + 1, z ) == id ) {
                        surrounding ^= 0x08;
                    }
                    if ( map -> GetID( x, y, z - 1 ) == id ) {
                        surrounding ^= 0x10;
                    }
                    if ( map -> GetID( x, y, z + 1 ) == id ) {
                        surrounding ^= 0x20;
                    }
                }
                Set( x, y, z, surrounding );
            }
        }
        int cmsizeX, cmsizeY, cmsizeZ;
        char GetMatrix3D( char* matrix, int x, int y, int z ) {
            if ( ( x >= 0 ) && ( x < cmsizeX ) && ( y >= 0 ) && ( y < cmsizeY ) && ( z >= 0 ) && ( z < cmsizeZ ) ) {
                return ( *( matrix + x * cmsizeY * cmsizeZ + y * cmsizeZ + z ) );
            }
            return -1;
        }
        void SetMatrix3D( char* matrix, int x, int y, int z, char v ) {
            if ( ( x >= 0 ) && ( x < cmsizeX ) && ( y >= 0 ) && ( y < cmsizeY ) && ( z >= 0 ) && ( z < cmsizeZ ) ) {
                ( *( matrix + x * cmsizeY * cmsizeZ + y * cmsizeZ + z ) ) = v;
            }
        }
        void CheckVisibleSides( char* matrix, int x, int y, int z ) {
            char here = GetMatrix3D( matrix, x, y, z );
            if ( here == 0 ) {
                char surrounding = 0x3F;
                if ( GetMatrix3D( matrix, x - 1, y, z ) <= 0 ) {
                    surrounding ^= 0x01;
                }
                if ( GetMatrix3D( matrix, x + 1, y, z ) <= 0 ) {
                    surrounding ^= 0x02;
                }
                if ( GetMatrix3D( matrix, x, y - 1, z ) <= 0 ) {
                    surrounding ^= 0x04;
                }
                if ( GetMatrix3D( matrix, x, y + 1, z ) <= 0 ) {
                    surrounding ^= 0x08;
                }
                if ( GetMatrix3D( matrix, x, y, z - 1 ) <= 0 ) {
                    surrounding ^= 0x10;
                }
                if ( GetMatrix3D( matrix, x, y, z + 1 ) <= 0 ) {
                    surrounding ^= 0x20;
                }
                Set( x, y, z, surrounding );
            }
        }
        inline void RecursiveDVFCheck( Map* m, char* matrix, double x, double y, double z, Stack< Point3D >* stack ) {
            if ( ( m -> GetID( x, y, z ) == 0 ) && ( GetMatrix3D( matrix, x, y, z ) == 0 ) ) {
                stack -> Push( Point( x, y, z ) );
            }
        }
        void DrawVisibleFields( Map* m, Point3D initPos ) {
            Stack< Point3D >* cacheStack = new Stack< Point3D >();
            cmsizeX = m -> GetX();
            cmsizeY = m -> GetY();
            cmsizeZ = m -> GetZ();
            char* checkMap = ( char* )( calloc( sizeof( char ), cmsizeX * cmsizeY * cmsizeZ ) );
            cacheStack -> Push( initPos );
            Point3D* buffer;
            while ( ( buffer = cacheStack -> Pop() ) != NULL ) {
                Point3D b = ( *buffer );
                SetMatrix3D( checkMap, b.x, b.y, b.z, 1 ); // checked
                RecursiveDVFCheck( m, checkMap, b.x - 1, b.y, b.z, cacheStack );
                RecursiveDVFCheck( m, checkMap, b.x + 1, b.y, b.z, cacheStack );
                RecursiveDVFCheck( m, checkMap, b.x, b.y - 1, b.z, cacheStack );
                RecursiveDVFCheck( m, checkMap, b.x, b.y + 1, b.z, cacheStack );
                RecursiveDVFCheck( m, checkMap, b.x, b.y, b.z - 1, cacheStack );
                RecursiveDVFCheck( m, checkMap, b.x, b.y, b.z + 1, cacheStack );
                free( buffer );
            }
            for ( int iz = 0; iz < cmsizeZ; iz++ ) {
                for ( int iy = 0; iy < cmsizeY; iy++ ) {
                    for ( int ix = 0; ix < cmsizeX; ix++ ) {
                        CheckVisibleSides( checkMap, ix, iy, iz );
                    }
                }
            }
            free( checkMap );
            delete cacheStack;
        }
        inline char Get( int x, int y, int z ) {
            if ( ( x >= 0 ) && ( x < sX ) && ( y >= 0 ) && ( y < sY ) && ( z >= 0 ) && ( z < sZ ) ) {
                return *( flag + x * sYsZ + y * sZ + z );
            }
            return -1;
        }
        inline char Get_N( int x, int y, int z ) {
            return *( flag + x * sYsZ + y * sZ + z );
        }
        inline void Set( int x, int y, int z, char f ) {
            if ( ( x >= 0 ) && ( x < sX ) && ( y >= 0 ) && ( y < sY ) && ( z >= 0 ) && ( z < sZ ) ) {
                *( flag + x * sYsZ + y * sZ + z ) = f;
            }
        }
        inline void Set_N( int x, int y, int z, char f ) {
            *( flag + x * sYsZ + y * sZ + z ) = f;
        }
        int sX, sY, sZ;
        int sYsZ;
        char* flag;
    };
    class OptimizedPointList {
        public:
        OptimizedPointList( Map* m, int xS, int zS, int xL, int zL ) {
            for ( int j = 0; j < MAX_TEXTURES; j++ ) {
                //for ( int i = 0; i < STD_NORMAL_VECTOR_COUNT; i++ ) {
                    head[ j ] = NULL;//[ i ] = NULL;
                //}
            }
            // parse map
            MapRenderInfo* mr = ( MapRenderInfo* )( m -> GetRenderInfo() );
            double blockSize = m -> GetBlockSize();
            // constants
            Point3D pos[ 4 ];
            int tex[ 4 ] = { 0, 1, 3, 2 };
            int normal_zero = 0x000000FC;
            int normal[ 6 ] = { 0x00000014, 0x00000094, 0x00000044, 0x00000064, 0x00000050, 0x00000058 };
            // creating
            //for ( int sidei = 0; sidei < 6; sidei++ ) {
                //int sideMask = 1 << sidei;
                for ( int iz = zS; iz < ( zS + zL ); iz++ ) {
                    for ( int iy = 0; iy < m -> GetY(); iy++ ) {
                        for ( int ix = xS; ix < ( xS + xL ); ix++ ) {
                            char buffer = mr -> Get_N( ix, iy, iz );
                            int textureID = m -> GetID( ix, iy, iz );
                            if ( ( !buffer ) || ( textureID <= 0 ) || ( textureID >= MAX_TEXTURES ) ) {
                                continue;
                            }
                            int j = textureID;
                            //int side_num = 0;
                            if ( buffer & 0x01 ) { // x - 1
                                pos[ 0 ] = Point( ix * blockSize, iy * blockSize, iz * blockSize );
                                pos[ 3 ] = Point( ix * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 2 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 1 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                for ( int i = 0; i < 3; i++ ) {
                                    head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                }
                                head[ j ] = new OptimizedPointListElement( pos[ 3 ], tex[ 3 ] | normal[ 0 ], head[ j ] );
                            }
                            if ( buffer & 0x02 ) { // x + 1
                                pos[ 3 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, iz * blockSize );
                                pos[ 0 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 1 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 2 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                for ( int i = 0; i < 3; i++ ) {
                                    head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                }
                                head[ j ] = new OptimizedPointListElement( pos[ 3 ], tex[ 3 ] | normal[ 1 ], head[ j ] );
                            }
                            if ( buffer & 0x04 ) { // y - 1
                                pos[ 2 ] = Point( ix * blockSize, iy * blockSize, iz * blockSize );
                                pos[ 3 ] = Point( ix * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 0 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 1 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, iz * blockSize );
                                for ( int i = 0; i < 3; i++ ) {
                                    head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                }
                                head[ j ] = new OptimizedPointListElement( pos[ 3 ], tex[ 3 ] | normal[ 2 ], head[ j ] );
                            }
                            if ( buffer & 0x08 ) { // y + 1
                                pos[ 1 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                pos[ 0 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 3 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 2 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                for ( int i = 0; i < 3; i++ ) {
                                    head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                }
                                head[ j ] = new OptimizedPointListElement( pos[ 3 ], tex[ 3 ] | normal[ 3 ], head[ j ] );
                            }
                            if ( buffer & 0x10 ) { // z - 1
                                pos[ 3 ] = Point( ix * blockSize, iy * blockSize, iz * blockSize );
                                pos[ 2 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                pos[ 1 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                pos[ 0 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, iz * blockSize );
                                for ( int i = 0; i < 3; i++ ) {
                                    head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                }
                                head[ j ] = new OptimizedPointListElement( pos[ 3 ], tex[ 3 ] | normal[ 4 ], head[ j ] );
                            }
                            if ( buffer & 0x20 ) { // z + 1
                                pos[ 0 ] = Point( ix * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 1 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 2 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                pos[ 3 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                for ( int i = 0; i < 3; i++ ) {
                                    head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                }
                                head[ j ] = new OptimizedPointListElement( pos[ 3 ], tex[ 3 ] | normal[ 5 ], head[ j ] );
                            }
                            /*if ( buffer & sideMask ) {
                                switch ( sideMask ) {
                                    case 0x01: {
                                        pos[ 0 ] = Point( ix * blockSize, iy * blockSize, iz * blockSize );
                                        pos[ 3 ] = Point( ix * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 2 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 1 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                        for ( int i = 0; i < 4; i++ ) {
                                            head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                        }
                                        break;
                                    }
                                    case 0x02: {
                                        pos[ 3 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, iz * blockSize );
                                        pos[ 0 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 1 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 2 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                        for ( int i = 0; i < 4; i++ ) {
                                            head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                        }
                                        break;
                                    }
                                    case 0x04: {
                                        pos[ 2 ] = Point( ix * blockSize, iy * blockSize, iz * blockSize );
                                        pos[ 3 ] = Point( ix * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 0 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 1 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, iz * blockSize );
                                        for ( int i = 0; i < 4; i++ ) {
                                            head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                        }
                                        break;
                                    }
                                    case 0x08: {
                                        pos[ 1 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                        pos[ 0 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 3 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 2 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                        for ( int i = 0; i < 4; i++ ) {
                                            head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                        }
                                        break;
                                    }
                                    case 0x10: {
                                        pos[ 3 ] = Point( ix * blockSize, iy * blockSize, iz * blockSize );
                                        pos[ 2 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                        pos[ 1 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, iz * blockSize );
                                        pos[ 0 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, iz * blockSize );
                                        for ( int i = 0; i < 4; i++ ) {
                                            head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                        }
                                        break;
                                    }
                                    case 0x20: {
                                        pos[ 0 ] = Point( ix * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 1 ] = Point( ix * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 2 ] = Point( ( ix + 1 ) * blockSize, ( iy + 1 ) * blockSize, ( iz + 1 ) * blockSize );
                                        pos[ 3 ] = Point( ( ix + 1 ) * blockSize, iy * blockSize, ( iz + 1 ) * blockSize );
                                        for ( int i = 0; i < 4; i++ ) {
                                            head[ j ] = new OptimizedPointListElement( pos[ i ], tex[ i ] | normal_zero, head[ j ] );
                                        }
                                        break;
                                    }
                                }
                            }
                            */
                        }
                    }
                //}
                /*for ( int j = 1; j < MAX_TEXTURES; j++ ) {
                    head[ j ] = new OptimizedPointListElement( Point( 0.0, 0.0, 0.0 ), tex[ 3 ] | normal[ sidei ], head[ j ] );
                }*/
            }
        }
        ~OptimizedPointList() {
            for ( int i = 0; i < MAX_TEXTURES; i++ ) {
                //for ( int n = 0; n < STD_NORMAL_VECTOR_COUNT; n++ ) {
                    cursor = head[ i ];//[ n ];
                    while ( cursor ) {
                        OptimizedPointListElement* grinder = cursor;
                        cursor = cursor -> next;
                        delete grinder;
                    }
                //}
            }
        }
        /*void Display() {
            glEnable( GL_TEXTURE_2D );
            glColor3f( MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
            for ( int i = 1; i < MAX_TEXTURES; i++ ) {
                if ( image[ i ] ) {
                    image[ i ] -> UseImage();
                } else {
                    continue;
                }
                cursor = head[ i ];
                glBegin( GL_QUADS );
                    while ( cursor ) {
                        unsigned char tex = cursor -> tex;
                        glTexCoord2d( ( tex >> 1 ) & 0x01, ( tex & 0x01 ) );
                        glVertex3f( cursor -> posX, cursor -> posY, cursor -> posZ );
                        cursor = cursor -> next;
                    }
                glEnd();
            }
        }*/
        /*inline void Display_Local( int j ) {
            cursor = head[ j ];
            while ( cursor ) {
                unsigned char tex = ( cursor -> tex );
                if ( ( tex & 0xFC ) != 0xFC ) { // normal vectors
                    glNormal3f( ( ( tex >> 6 ) & 0x03 ) - 1,
                                ( ( tex >> 4 ) & 0x03 ) - 1,
                                ( ( tex >> 2 ) & 0x03 ) - 1 );
                } else {
                    glTexCoord2d( ( tex >> 1 ) & 0x01, ( tex & 0x01 ) );
                    glVertex3f( cursor -> posX, cursor -> posY, cursor -> posZ );
                }
                cursor = cursor -> next;
            }
        }*/
        inline void Display_Local_Heap( int j, double tWidth, double tCount ) {
            double texA = double( j ) / tCount;
            double texW = 1.0 / tCount;
            unsigned char tex;
            //for ( int i = 0; i < STD_NORMAL_VECTOR_COUNT; i++ ) {
                cursor = head[ j ];//[ i ];
                //Point3D n = MAP_NORMAL_VECTOR[ i ];
                //glNormal3f( n.x, n.y, n.z );
                while ( cursor ) {
                    tex = ( cursor -> tex );
                    if ( ( tex & 0xFC ) != 0xFC ) { // normal vectors
                        glNormal3f( ( ( tex >> 6 ) & 0x03 ) - 1,
                                    ( ( tex >> 4 ) & 0x03 ) - 1,
                                    ( ( tex >> 2 ) & 0x03 ) - 1 );
                    }
                    glTexCoord2d( texA + double( ( tex >> 1 ) & 0x01 ) * texW, ( tex & 0x01 ) );
                    glVertex3f( cursor -> posX, cursor -> posY, cursor -> posZ );
                    cursor = cursor -> next;
                }
            //}
        }
        int Length( int j ) {
            int q = 0;
            //for ( int i = 0; i < STD_NORMAL_VECTOR_COUNT; i++ ) {
                cursor = head[ j ];//[ i ];
                while ( cursor ) {
                    q++;
                    cursor = cursor -> next;
                }
            //}
            return q;
        }
        class OptimizedPointListElement {
            public:
            OptimizedPointListElement( Point3D pos, int texI, OptimizedPointListElement* n ) {
                posX = pos.x;
                posY = pos.y;
                posZ = pos.z;
                tex = texI;
                next = n;
            }
            ~OptimizedPointListElement() {
            }
            short int posX, posY, posZ;
            unsigned char tex;
            OptimizedPointListElement* next;
        };
        OptimizedPointListElement* head[ MAX_TEXTURES ];//[ STD_NORMAL_VECTOR_COUNT ];
        OptimizedPointListElement* cursor;
    };
    string pathR;
    double spawnX, spawnY, spawnZ, angleXZ, angleY;
    int* data;
    MapRenderInfo* mrInfo;
    OptimizedPointList* ( *oplist )[];
    int oplistSize;
    int sizeT, sX, sY, sZ;
    double bSize, hBlockSize;
    Bitmap* joinedTex;
    int tex_stdw, tex_stdh, tcount;
    GLuint jTgl;
    Water* water;
};

Map* MainMap = NULL;

void RecursiveSideUpdate( int x, int y, int z ) {
    if ( MainMap ) {
        MainMap -> UpdateSides( x, y, z );
        MainMap -> NoChunkUpdated();
        MainMap -> UpdateChunk( x, y, z );
        MainMap -> UpdateChunk( x + 1, y, z );
        MainMap -> UpdateChunk( x - 1, y, z );
        MainMap -> UpdateChunk( x, y, z + 1 );
        MainMap -> UpdateChunk( x, y, z - 1 );
    }
}

#define MAX_POLYGON_POINTS 2

//#define POLYGON_FLAG_SET

#define WCUBE_SIZE 1.01

void DrawWCube( int x, int y, int z, double r, double g, double b ) {
    glPushMatrix();
    glTranslated( x + 0.5, y + 0.5, z + 0.5 );
    glLineWidth( 1.0 );
    glDisable( GL_LIGHTING );
    glColor3f( r, g, b );
    glutWireCube( WCUBE_SIZE );
    glEnable( GL_LIGHTING );
    glPopMatrix();
}

class PolygonFiller {
    public:
    PolygonFiller() {
        Reset();
    }
    ~PolygonFiller() {
    }
    void Reset() {
        for ( int i = 0; i < MAX_POLYGON_POINTS; i++ ) {
            //flagList[ i ] = 0;
            polygonList[ i ] = Point( 0.0, 0.0, 0.0 );
        }
        last = -1;
        lastStamp = -1;
    }
    void Select( Point3D at ) {
        if ( last < MAX_POLYGON_POINTS - 1 ) {
            last++;
            polygonList[ last ] = at;
        }
        //flagList[ i ] |=
    }
    void Undo() {
        if ( last >= 0 ) {
            last--;
        }
    }
    void Display() {
        double r = 0.2;
        double g = 0.2;
        double b = 0.9;
        if ( lastStamp >= Timer::Current() ) {
            r = 0.9;
            b = 0.2;
        }
        for ( int i = 0; i <= last; i++ ) {
            DrawWCube( polygonList[ i ].x, polygonList[ i ].y, polygonList[ i ].z, r, g, b );
        }
    }
    void Finalize_Wall_Rect( int textureID ) {
        // takse only 2 points!
        if ( last == 1 ) {
            Point3D a = polygonList[ 0 ];
            Point3D b = polygonList[ 1 ];
            int x1 = min( a.x, b.x );
            int y1 = min( a.y, b.y );
            int z1 = min( a.z, b.z );
            int x2 = max( a.x, b.x );
            int y2 = max( a.y, b.y );
            int z2 = max( a.z, b.z );
            char eq = 0;
            if ( x1 == x2 ) {
                for ( int i = y1; i <= y2; i++ ) {
                    for ( int j = z1; j <= z2; j++ ) {
                        MainMap -> SetID( a.x, i, j, textureID );
                        MainMap -> //RecursiveSideUpdate( a.x, i, j );
                            UpdateSides( a.x, i, j );
                    }
                }
            } else {
                eq += 1;
            }
            if ( y1 == y2 ) {
                for ( int i = x1; i <= x2; i++ ) {
                    for ( int j = z1; j <= z2; j++ ) {
                        MainMap -> SetID( i, a.y, j, textureID );
                        MainMap -> //RecursiveSideUpdate( i, a.y, j );
                            UpdateSides( i, a.y, j );
                    }
                }
            } else {
                eq += 2;
            }
            if ( z1 == z2 ) {
                for ( int i = x1; i <= x2; i++ ) {
                    for ( int j = y1; j <= y2; j++ ) {
                        MainMap -> SetID( i, j, a.z, textureID );
                        MainMap -> //RecursiveSideUpdate( i, j, a.z );
                            UpdateSides( i, j, a.z );
                    }
                }
            } else {
                eq += 4;
            }
            if ( eq == 7 ) { // cannot fill
                lastStamp = Timer::Current() + 1.5;
            } else {
                MainMap -> RebuildChunks();
            }
        }
    }
    private:
    //int flagList[ MAX_POLYGON_POINTS ];
    int last;
    double lastStamp;
    Point3D polygonList[ MAX_POLYGON_POINTS ];
};

PolygonFiller* MainMapFiller = new PolygonFiller();
