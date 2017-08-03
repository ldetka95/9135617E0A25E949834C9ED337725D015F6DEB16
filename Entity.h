#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include "Point3D.h"
#include "Blocks.h"
#include "Audio.h"
#include "Textures.h"
#include "Object.h"

#include "Weapon.h"

#include "BlockProperties.h"

class SpawnClock {
    public:
    SpawnClock( int IDS, double Tmin, double Tmax ) {
        IDSpawned = IDS;
        timer = new Timer( Tmin, Tmax );
    }
    ~SpawnClock() {
        delete timer;
    }
    bool SpawnTime() {
        return timer -> Tick();
    }
    int SpawnID() {
        return IDSpawned;
    }
    private:
    int IDSpawned;
    Timer* timer;
};

class SpawnScheme {
    public:
    SpawnScheme( const char* path ) {
        sNum = 0;
        spawnArray = NULL;
        FILE* handle = fopen( path, "r" );
        if ( handle ) {
            int RDam = 0;
            do {
                Point3D pRD;
                RDam = fscanf( handle, "%lf %lf %lf\n", &pRD.x, &pRD.y, &pRD.z );
                if ( RDam > 0 ) {
                    SpawnClock* sClock = new SpawnClock( int( pRD.x ), pRD.y, pRD.z );
                    Append( sClock );
                }
            } while ( RDam > 0 );
            fclose( handle );
        }
    }
    ~SpawnScheme() {
        Flush();
    }
    void Resize( int newSize ) {
        if ( newSize <= 0 ) {
            Flush();
            return;
        }
        spawnArray = ( SpawnClock* (*)[] )( realloc( spawnArray, sizeof( SpawnClock* ) * newSize ) );
        for ( int i = sNum; i < newSize; i++ ) {
            ( *spawnArray )[ i ] = NULL;
        }
        sNum = newSize;
    }
    void Append( SpawnClock* sc ) {
        Resize( sNum + 1 );
        ( *spawnArray )[ sNum - 1 ] = sc;
    }
    void Flush() {
        if ( spawnArray ) {
            for ( int i = 0; i < sNum; i++ ) {
                delete ( *spawnArray )[ i ];
            }
            free( spawnArray );
            spawnArray = NULL;
        }
        sNum = 0;
    }
    bool SpawnTime( int index ) {
        return ( *spawnArray )[ index ] -> SpawnTime();
    }
    int SpawnID( int index ) {
        return ( *spawnArray )[ index ] -> SpawnID();
    }
    int SpawnNumber() {
        return sNum;
    }
    private:
    SpawnClock* ( *spawnArray )[];
    int sNum;
};

SpawnScheme* currentSpawnScheme = NULL;

void ReloadScheme( int id ) {
    StringLinker* slinker = new StringLinker( "data/level/spawn.slf" );
    if ( currentSpawnScheme ) {
        delete currentSpawnScheme;
        currentSpawnScheme = NULL;
    }
    currentSpawnScheme = new SpawnScheme( ( slinker -> GetString( id ) ).c_str() );
    delete slinker;
}

#define MAX_ENTITY_TIMERS 16

class Entity {
    public:
    Entity( double x, double y, double z, int points, Object* o ) {
        hotspot = Point( x, y, z );
        if ( points ) {
            pointOffset = ( Point3D (*)[] )( calloc( points, sizeof( Point3D ) ) );
        } else {
            pointOffset = NULL;
        }
        pointNum = points;
        yV = 0.0;
        if ( o ) {
            for ( int i = 0; i < MAX_ENT_VAR; i++ ) {
                var[ i ] = o -> GetVar( i );
                //printf( " > clone var[ %d ]: %g << %g\n", i, var[ i ], o -> GetVar( i ) );
            }
        }
        mother = o;
        if ( o ) {
            Model* m = o -> GetModel();
            if ( ( m ) && ( points ) ) {
                RegisterOffset( 0, 0.0, m -> GetLowestY(), 0.0 );
            }
        }
        faceTo = Point( x + Random( -1.0, 1.0 ), 0.0, z + Random( -1.0, 1.0 ) );
        for ( int i = 0; i < MAX_ENTITY_TIMERS; i++ ) {
            timers[ i ] = NULL;
        }
        spawnPoint = hotspot;
        velocity = Point( 0.0, 0.0, 0.0 );
        prevVelocity = velocity;
    }
    ~Entity() {
        if ( pointOffset ) {
            free( pointOffset );
        }
        for ( int i = 0; i < MAX_ENTITY_TIMERS; i++ ) {
            Remove_Timer( i );
        }
    }
    void RegisterOffset( int pointIndex, double ox, double oy, double oz ) {
        Point3D p3d;
        p3d.x = ox;
        p3d.y = oy;
        p3d.z = oz;
        ( *pointOffset )[ pointIndex ] = p3d;
    }
    void RegisterOffset( int pointIndex, Point3D p ) {
        if ( pointIndex >= pointNum ) {
            ___ResizeCollisionArray( pointIndex + 1 );
        }
        ( *pointOffset )[ pointIndex ] = p;
    }
    char Collide( Map* m, Point3D baseSwitch ) {
        char col[ 3 ] = { 0, 0, 0 };
        if ( m ) {
            for ( int i = 0; i < pointNum; i++ ) {
                Point3D obuffer = AddPoint( ( *pointOffset )[ i ], hotspot );
                Point3D buffer = obuffer;
                buffer.x += ( baseSwitch.x ) / m -> GetBlockSize();
                buffer.y += ( baseSwitch.y ) / m -> GetBlockSize();
                buffer.z += ( baseSwitch.z ) / m -> GetBlockSize();
                int blockid;
                if ( ( blockid = m -> GetID( buffer.x, obuffer.y, obuffer.z ) ) > 0 ) { // x
                    if ( Map::isSolid( blockid ) ) {
                        col[ 0 ] = 0x01;
                    }
                }
                if ( ( blockid = m -> GetID( obuffer.x, buffer.y, obuffer.z ) ) > 0 ) { // y
                    if ( Map::isSolid( blockid ) ) {
                        col[ 1 ] = 0x02;
                    }
                }
                if ( ( blockid = m -> GetID( obuffer.x, obuffer.y, buffer.z ) ) > 0 ) { // z
                    if ( Map::isSolid( blockid ) ) {
                        col[ 2 ] = 0x04;
                    }
                }
            }
            Model* model = NULL;
            if ( mother ) {
                model = mother -> GetModel();
            }
            if ( model ) {
                for ( int i = 0; i < model -> Length(); i++ ) {
                    ModelBlock modelBlock = model -> Get( i );
                    Point3D obuffer = AddPoint( modelBlock.offset, hotspot );
                    Point3D buffer = obuffer;
                    buffer.x += ( baseSwitch.x ) / m -> GetBlockSize();
                    buffer.y += ( baseSwitch.y ) / m -> GetBlockSize();
                    buffer.z += ( baseSwitch.z ) / m -> GetBlockSize();
                    int blockid;
                    if ( ( blockid = m -> GetID( buffer.x, obuffer.y, obuffer.z ) ) > 0 ) { // x
                        if ( Map::isSolid( blockid ) ) {
                            col[ 0 ] = 0x01;
                        }
                    }
                    if ( ( blockid = m -> GetID( obuffer.x, buffer.y, obuffer.z ) ) > 0 ) { // y
                        if ( Map::isSolid( blockid ) ) {
                            col[ 1 ] = 0x02;
                        }
                    }
                    if ( ( blockid = m -> GetID( obuffer.x, obuffer.y, buffer.z ) ) > 0 ) { // z
                        if ( Map::isSolid( blockid ) ) {
                            col[ 2 ] = 0x04;
                        }
                    }
                }
            }
        }
        return col[ 0 ] + col[ 1 ] + col[ 2 ];
    }
    int getBlockStanding( Map* m ) {
        return m -> GetID( hotspot.x, hotspot.y - 0.1, hotspot.z );
    }
    void MoveAtMap( Map* m, Point3D sw ) {
        prevVelocity = velocity;
        int blockStanding = getBlockStanding( m );
        int currentBlock = m -> GetID( hotspot.x, hotspot.y + 0.1, hotspot.z );
        // standing block affection
        double speedFactor = blockProperties -> getWalkSpeedFactor( blockStanding );
        double slipFactor = blockProperties -> getSlipFactor( blockStanding );
        double liquidSlowDown = blockProperties -> getLiquidSlowDown( currentBlock );
        if ( !isnan( speedFactor ) ) {
            sw.x *= speedFactor;
            sw.z *= speedFactor;
        }
        if ( !isnan( slipFactor ) ) {
            double yvel = sw.y;
            //printf( "Slipfactor( %d ) = %g\n", blockStanding, slipFactor );
            sw = Balance( sw, prevVelocity, slipFactor );
            sw.y = yvel;
        }
        if ( !isnan( liquidSlowDown ) ) {
            ScalePoint3D( &sw, 1.0 / ( liquidSlowDown + 1.0 ) );
        }
        velocity = sw;
        // collision check - must be after block affection
        char collision = Collide( m, sw );
        if ( collision & 0x01 ) { // x
            sw.x = 0; // stahp
        }
        if ( collision & 0x02 ) { // y
            sw.y = 0;
        }
        if ( collision & 0x04 ) { // z
            sw.z = 0;
        }
        //sw.x += hotspot.x;
        //sw.y += hotspot.y;
        //sw.z += hotspot.z;
        hotspot = AddPoint( hotspot, sw );
    }
    Point3D CollidedAtMap( Map* m, Point3D sw ) {
        //Point3D hotspotHolder = hotspot;
        //hotspot = pos;
        char collision = Collide( m, sw );
        if ( collision & 0x01 ) { // x
            sw.x = 0; // stahp
        }
        if ( collision & 0x02 ) { // y
            sw.y = 0;
        }
        if ( collision & 0x04 ) { // z
            sw.z = 0;
        }
        return sw;
    }
    bool TryJump( Map* m, double power ) {
        if ( Collide( m, grndVector ) & 0x02 ) {
            yV = power;
            return true;
        }
        return false;
    }
    bool ReachedGround() {
        bool ret = rground;
        rground = false;
        return ret;
    }
    Point3D HotSpot() {
        return hotspot;
    }
    Point3D FixedPlayerSpot() {
        Point3D spot = hotspot;
        spot.y += pHeight;
        return spot;
    }
    void UpdateHotSpot( Point3D hspot ) {
        hotspot = hspot;
    }
    void GravityAffect( Map* m, double power, double inertia, Point3D* point ) {
        power *= level_Gravity;
        yV = ( yV * inertia ) - ( power * ( 1.0 - inertia ) );
        Point3D p3d = *point;
        p3d.y += yV;
        char collision = Collide( m, p3d );
        if ( collision & 0x02 ) { // y
            yV = 0.0;
        }
        ( *point ) = p3d;
    }
    void Respawn( Map* m ) {
        if ( m ) {
            hotspot.x = m -> GetSpawnX();
            hotspot.y = m -> GetSpawnY();
            hotspot.z = m -> GetSpawnZ();
        }
    }
    bool CanCollide( Point3D point, double radius ) {
        for ( int i = 0; i < pointNum; i++ ) {
            if ( Dist3D( AddPoint( ( *pointOffset )[ i ], hotspot ), point ) <= radius ) {
                return true;
            }
        }
        return false;
    }
    bool CanCollide( Point3D point ) { // read radius from model
        if ( mother ) {
            Model* model = mother -> GetModel();
            if ( model ) {
                for ( int i = 0; i < model -> Length(); i++ ) {
                    ModelBlock mBlock = model -> Get( i );
                    Point3D collisionPoint = mBlock.offset;
                    double collisionRadius = mBlock.var[ COLLISION_RADIUS ];
                    if ( Dist3D( AddPoint( collisionPoint, hotspot ), point ) <= collisionRadius ) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    void CollideEntity( Entity* e, Point3D* movement, double collisionRadius ) {
        if ( movement ) {
            Point3D entNextPos = AddPoint( hotspot, ( *movement ) );
            Point3D secEntPos = e -> HotSpot();
            double dist = Dist3D( entNextPos, secEntPos );
            Point3D collisionRemove = { 0.0, 0.0, 0.0 };
            if ( ( dist < collisionRadius ) && ( dist > 0.0 ) ) {
                collisionRemove = SubtractPoint( entNextPos, secEntPos );
                ScalePoint3D( &collisionRemove, 0.1 );
            }
            ( *movement ) = AddPoint( ( *movement ), collisionRemove );
        }
    }
    void SetVar( int index, double value ) {
        var[ index ] = value;
    }
    double GetVar( int index ) {
        return var[ index ];
    }
    double GetLowestYPoint() {
        double ret = hotspot.y;
        for ( int i = 0; i < pointNum; i++ ) {
            if ( ( *pointOffset )[ i ].y < ret ) {
                ret = ( *pointOffset )[ i ].y;
            }
        }
        return ret;
    }
    inline void FaceTo( Point3D posDir ) {
        faceTo = posDir;
    }
    inline Point3D Facing() {
        return faceTo;
    }
    /*void Display( Point3D pos ) {
        Model* model = mother -> GetModel();
        if ( model ) {
            model -> Display( pos, faceTo, var[ VAR_ENTITY_MOVE_SPEED_XZ ] / var[ VAR_SPEED ] );
        }
    }*/
    void Display( Point3D pos ) {
        Model* model = mother -> GetModel();
        if ( model ) {
            model -> Display( pos, faceTo, var[ VAR_ENTITY_MOVE_SPEED_XZ ] / var[ VAR_SPEED ] );
        }
    }
    inline Object* GetMother() {
        return mother;
    }
    inline Timer* Apply_Timer( int index, double minInterval, double maxInterval, bool* created = NULL ) {
        if ( timers[ index ] ) {
            if ( created ) {
                ( *created ) = false;
            }
            return timers[ index ];
        } else {
            if ( created ) {
                ( *created ) = true;
            }
            Timer* ret = new Timer( minInterval, maxInterval );
            timers[ index ] = ret;
            return ret;
        }
    }
    inline void Remove_Timer( int index ) {
        if ( timers[ index ] ) {
            delete timers[ index ];
            timers[ index ] = NULL;
        }
    }
    inline Point3D SpawnedAt() {
        return spawnPoint;
    }
    private:
    Timer* timers[ MAX_ENTITY_TIMERS ];
    Point3D ( *pointOffset )[];
    int pointNum;
    Point3D hotspot, velocity, prevVelocity;
    double yV;
    bool rground;
    double var[ MAX_ENT_VAR ];
    Object* mother;
    Point3D faceTo, spawnPoint;
    void ___ResizeCollisionArray( int a ) {
        pointOffset = ( Point3D (*)[] )( realloc( pointOffset, a * sizeof( Point3D ) ) );
        for ( int i = pointNum; i < a; i++ ) {
            ( *pointOffset )[ i ] = Point( 0.0, 0.0, 0.0 );
        }
        pointNum = a;
    }
};

Entity* player = NULL;

int TIMEDISPLAY_MIN_TIME = 10000;
int TIMEDISPLAY_MAX_TIME = 20000;

double TIMEDISPLAY_MIN_SIZE = 1.0;
double TIMEDISPLAY_MAX_SIZE = 2.25;

char STD_TIMEDISPLAY_TEXCOORD_INIT[] = { 0, 1, 0, 0, 1, 0, 1, 1 };

class TimeDisplay {
    public:
    TimeDisplay( Point3D pos, int textureMinID, int textureMaxID ) {
        timer = new Timer( TIMEDISPLAY_MIN_TIME, TIMEDISPLAY_MAX_TIME );
        textureID = Random( textureMinID, textureMaxID );
        angle = Random( 360.0 );
        size = Random( TIMEDISPLAY_MAX_SIZE - TIMEDISPLAY_MIN_SIZE ) + TIMEDISPLAY_MIN_SIZE;
        place = pos;
        ObtainVertex();
    }
    TimeDisplay( Point3D pos, int NtextureID ) {
        timer = new Timer( TIMEDISPLAY_MIN_TIME, TIMEDISPLAY_MAX_TIME );
        textureID = NtextureID;
        angle = Random( 360.0 );
        size = Random( TIMEDISPLAY_MAX_SIZE - TIMEDISPLAY_MIN_SIZE ) + TIMEDISPLAY_MIN_SIZE;
        place = pos;
        ObtainVertex();
    }
    ~TimeDisplay() {
        delete timer;
    }
    bool Finished() {
        return timer -> Tick();
    }
    int GetTexture() {
        return textureID;
    }
    void Display() {
        glPushMatrix();
        //glMatrixMode( GL_MODELVIEW );
        glTranslatef( place.x, place.y, place.z );
        image[ textureID ] -> UseImage();
        glRotated( angle, 0.0, 1.0, 0.0 );
        //glEnable( GL_TEXTURE_2D );
        glBegin( GL_QUADS );

            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( size / 2, 0.0, size / 2 );
            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( size / 2, 0.0, - size / 2 );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( - size / 2, 0.0, - size / 2 );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( - size / 2, 0.0, size / 2 );

        glEnd();
        //glDisable( GL_TEXTURE_2D );
        glPopMatrix();
    }
    inline void Display_Local() {
        //int hsize = ( int )( size ) >> 1;
        //glTexCoord2d( 0.0, 1.0 );
        //glVertex3f( place.x + hsize, place.y, place.z + hsize );
        //glTexCoord2d( 0.0, 0.0 );
        //glVertex3f( place.x + hsize, place.y, place.z - hsize );
        //glTexCoord2d( 1.0, 0.0 );
        //glVertex3f( place.x - hsize, place.y, place.z - hsize );
        //glTexCoord2d( 1.0, 1.0 );
        //glVertex3f( place.x - hsize, place.y, place.z + hsize );
        for ( int i = 0; i < 4; i++ ) {
            // glTexCoord2d( texcoord[ ( i << 1 ) ], texcoord[ ( i << 1 ) | 0x01 ] );
            glTexCoord2d( STD_TIMEDISPLAY_TEXCOORD_INIT[ ( i << 1 ) ], STD_TIMEDISPLAY_TEXCOORD_INIT[ ( i << 1 ) | 0x01 ] );
            glVertex3f( vertex[ i ].x, vertex[ i ].y, vertex[ i ].z );
        }
    }
    void ObtainVertex() {
        double hsize = size / 2.0;
        vertex[ 0 ] = Point( place.x + hsize, place.y, place.z + hsize );
        vertex[ 1 ] = Point( place.x + hsize, place.y, place.z - hsize );
        vertex[ 2 ] = Point( place.x - hsize, place.y, place.z - hsize );
        vertex[ 3 ] = Point( place.x - hsize, place.y, place.z + hsize );
        for ( int i = 0; i < 4; i++ ) {
            Point3D buffer = SubtractPoint( vertex[ i ], place );
            double vertexRadius, vertexAngle;
            ctp( buffer.x, buffer.z, &vertexRadius, &vertexAngle );
            ptc( vertexRadius, vertexAngle + angle * PI / 180.0, &buffer.x, &buffer.z );
            //printf( "%d: x = %lf, z = %lf\n", i, buffer.x, buffer.z );
            vertex[ i ] = AddPoint( buffer, place );
        }
        /*for ( int i = 0; i < 8; i++ ) {
            texcoord[ i ] = STD_TIMEDISPLAY_TEXCOORD_INIT[ i ];
        }*/
    }
    private:
    Timer* timer;
    int textureID;
    double angle;
    double size;
    Point3D vertex[ 4 ];
    // tex coords will be obtained by constant coding
    // char texcoord[ 8 ]; // external constant array
    Point3D place;
};

const int MAX_BLOOD_DEBRIS = 1024;
TimeDisplay* bloodDebris[ MAX_BLOOD_DEBRIS ];

int RegisterBlood( Point3D pos, int id, int startFrom = 0 ) {
    for ( int i = startFrom; i < MAX_BLOOD_DEBRIS; i++ ) {
        if ( !bloodDebris[ i ] ) {
            pos.y += double( i + 1 ) * 0.0001; // Display order fix, do NOT remove!
            bloodDebris[ i ] = new TimeDisplay( pos, id );
            return i;
        }
    }
    return MAX_BLOOD_DEBRIS;
}

int BLOOD_MIN_NUM = 16;
int BLOOD_MAX_NUM = 32;
double RAND_BLOOD_DISPERSION_MAX = 3.15;

int BLOOD_MIN_INDEX = 192;
int BLOOD_MAX_INDEX = 196;

void KillBloodEntity( Entity* e ) {
    int sFrom = 0;
    int splatNum = Random( BLOOD_MIN_NUM, BLOOD_MAX_NUM );
    for ( int i = 0; i < splatNum; i++ ) {
        double r = Random( RAND_BLOOD_DISPERSION_MAX );
        double a = Random( 2 * PI );
        Point3D randPos = { 0.0, e -> GetLowestYPoint(), 0.0 };
        ptc( r, a, &randPos.x, &randPos.z );
        randPos = AddPoint( randPos, e -> HotSpot() );
        int splatTextureID = Random( BLOOD_MIN_INDEX, BLOOD_MAX_INDEX );
        sFrom = RegisterBlood( randPos, splatTextureID, sFrom ) + 1;
    }
}

void ShatterBloodEntity( Entity* e ) {
    double r = Random( RAND_BLOOD_DISPERSION_MAX );
    double a = Random( 2 * PI );
    Point3D randPos = { 0.0, e -> GetLowestYPoint(), 0.0 };
    ptc( r, a, &randPos.x, &randPos.z );
    randPos = AddPoint( randPos, e -> HotSpot() );
    int splatTextureID = Random( BLOOD_MIN_INDEX, BLOOD_MAX_INDEX );
    RegisterBlood( randPos, splatTextureID, 0 );
}

void DisplayDebris_Optimized() {
    /// Can NOT be used now;
    /// because transformations can't be called inside OpenGL "throttle",
    /// I have to recalculate vertexes in order to display them fast.
    /// ( required external Point3D rotating and scaling implementation in Debris constructor )
    for ( int j = BLOOD_MIN_INDEX; j < BLOOD_MAX_INDEX; j++ ) {
        if ( !image[ j ] ) {
            continue;
        }
        image[ j ] -> UseImage();
        glBegin( GL_QUADS );
            glNormal3f( 0.0, 1.0, 0.0 );
            for ( int i = 0; i < MAX_BLOOD_DEBRIS; i++ ) {
                if ( bloodDebris[ i ] ) {
                    if ( bloodDebris[ i ] -> GetTexture() != j ) {
                        continue;
                    }
                    bloodDebris[ i ] -> Display_Local();
                }
            }
        glEnd();
    }
}

void ProcessBloodVestiges() {
    for ( int i = 0; i < MAX_BLOOD_DEBRIS; i++ ) {
        if ( bloodDebris[ i ] ) {
            // bloodDebris[ i ] -> Display(); /// TO-CHANGE
            if ( bloodDebris[ i ] -> Finished() ) {
                delete bloodDebris[ i ];
                bloodDebris[ i ] = NULL;
            }
        }
    }
    DisplayDebris_Optimized();
}

//#define STD_TRACEBUFFER_SIZE 48

class Bullet {
    public:
    Bullet( Entity* Cshooter, Point3D posinit, Point3D target, Point3D color, double damage, double speed, double range, int nvelocity ) {
        shooter = Cshooter;
        positionStart = posinit;
        position = posinit;
        velocity = SubtractPoint( target, posinit );
        double dist = Dist3D( posinit, target );
        if ( dist ) {
            ScalePoint3D( &velocity, speed / dist );
        }
        maxRange = range;
        dmg = damage;
        spd = speed;
        traceVelocity = nvelocity;
        traceCursor = 0;
        trace = ( Point3D* )( calloc( sizeof( Point3D ), nvelocity ) );
        for ( int i = 0; i < nvelocity; i++ ) {
            trace[ i ] = posinit;
        }
        bulletColor = color;//Point( 1.0, 1.0, 1.0 );
    }
    ~Bullet() {
        free( trace );
    }
    bool Move( double scale ) { // returns false when exceeded maximum range
        Point3D currentVelocity = velocity;
        ScalePoint3D( &currentVelocity, scale );
        position = AddPoint( position, currentVelocity );
        traceCursor = ( traceCursor + 1 ) % traceVelocity;//STD_TRACEBUFFER_SIZE;
        trace[ traceCursor ] = position;
        return ( Dist3D( position, positionStart ) < maxRange );
    }
    bool Collide( Map* m ) {
        if ( m ) {
            if ( m -> GetID( position.x, position.y, position.z ) > 0 ) {
                return true;
            }
        }
        return false;
    }
    int Collide( Entity* eArray[], int entityNum ) {
        for ( int i = 0; i < entityNum; i++ ) {
            if ( ( eArray[ i ] ) && ( eArray[ i ] != shooter ) ) {
                if ( eArray[ i ] -> CanCollide( position ) ) { // , eArray[ i ] -> GetVar( VAR_COLLISION_RADIUS )
                    return i;
                }
            }
        }
        return -1; // no collision
    }
    void Display() {
        Point3D head = trace[ traceCursor ];
        Point3D tail = trace[ ObtainVelocitedCursor() ];
        glLineWidth( 4.25f );
        glBegin( GL_LINES );
            glPointSize( 3.0 );
            glVertex3f( head.x, head.y, head.z );
            glVertex3f( tail.x, tail.y, tail.z );
            glPointSize( 1.0 );
        glEnd();
    }
    void Display_Fast() {
        Point3D head = trace[ traceCursor ];
        Point3D tail = trace[ ObtainVelocitedCursor() ];
        glColor3f( bulletColor.x, bulletColor.y, bulletColor.z );
        glVertex3f( head.x, head.y, head.z );
        glVertex3f( tail.x, tail.y, tail.z );
    }
    int ObtainVelocitedCursor() {
        /*int ret = traceCursor - traceVelocity;
        if ( ret >= STD_TRACEBUFFER_SIZE ) {
            ret -= STD_TRACEBUFFER_SIZE;
        }
        if ( ret < 0 ) {
            ret += STD_TRACEBUFFER_SIZE;
        }
        return ret;*/
        return ( traceCursor + 1 ) % traceVelocity;
    }
    Entity* shooter;
    Point3D positionStart;
    Point3D position;
    Point3D velocity;
    Point3D bulletColor;
    double dmg;
    double maxRange;
    double spd;
    Point3D* trace;//[ STD_TRACEBUFFER_SIZE ];
    int traceCursor;
    int traceVelocity;
};

const int MAX_MONSTERS = 32;
Entity* monsterStorage[ MAX_MONSTERS ];

const int MAX_BULLETS = 256;
Bullet* bulletStorage[ MAX_BULLETS ];

#define STD_BULLET_MAX_DISTANCE 50.0
#define STD_BULLET_SPEED 0.85

#define STD_SHOOT_SOUND_PATH "shoot2.ogg"//mp3"
#define STD_HIT_SOUND_PATH "emptyHit.ogg"//mp3"

#define STD_DEATH_DOG_SOUND_PATH "Dog_death.ogg"//mp3"

#define MAX_SHOOT_DISPERSION 5.2 // degreess

int SHOOT_INTERVAL = 90; // shooting speed

/* Point3D Shoot( Entity* shooter, Point3D posFrom, Point3D posTo, double damage ) {
    Point3D ret = { 0.0, 0.0, 0.0 };
    for ( int i = 0; i < MAX_BULLETS; i++ ) {
        if ( !bulletStorage[ i ] ) { // free slot
            bulletStorage[ i ] = new Bullet( shooter, posFrom, posTo, damage, STD_BULLET_SPEED, STD_BULLET_MAX_DISTANCE, 3 );
            if ( shooter == player ) {
                double dispersion = ( Random( MAX_SHOOT_DISPERSION ) - MAX_SHOOT_DISPERSION / 2.0 ) * PI / 180.0;
                double dispersionAngle = Random( 2 * PI );
                ret.x = sin( dispersionAngle ) * dispersion;
                ret.y = cos( dispersionAngle ) * dispersion;
                // PlaySound( CHANNEL_SHOOT, STD_SHOOT_SOUND_PATH );
            }
            break;
        }
    }
    return ret;
} */

void DisposeBullet( int index ) {
    if ( bulletStorage[ index ] ) {
        delete bulletStorage[ index ];
        bulletStorage[ index ] = NULL;
    }
}

#define ENTITY_DAMAGED 1
#define ENTITY_NOT_DAMAGED 0

#define MIN_ENTITY_BLOOD_DROP 64//192
#define MAX_ENTITY_BLOOD_DROP 96//256

#define MIN_ENTITY_DAMAGED_DROP 0.15
#define MAX_ENTITY_DAMAGED_DROP 0.25
// Values just above are multipled by damage! Can get laggy if too much damaged!

#define STD_DEATH_PUSH 0.04

void MoveBullets() {
    for ( int i = 0; i < MAX_BULLETS; i++ ) { // every bullet
        if ( bulletStorage[ i ] ) { // if exists
            Bullet* bullet = bulletStorage[ i ];
            double moveDensity = 0.15;
            for ( double j = 0; j < bullet -> spd; j += moveDensity ) {
                if ( bullet -> Move( moveDensity ) ) {
                    if ( bullet -> Collide( MainMap ) ) {
                        PlaySound( CHANNEL_HIT, STD_HIT_SOUND_PATH );
                        SetVolumeDist( CHANNEL_HIT, Dist3D( player -> FixedPlayerSpot(), bullet -> position ), 0.25 );
                        DisposeBullet( i );
                        break;
                    }
                    int collideID = bullet -> Collide( monsterStorage, MAX_MONSTERS );
                    if ( collideID >= 0 ) {
                        monsterStorage[ collideID ] -> SetVar( VAR_HP, monsterStorage[ collideID ] -> GetVar( VAR_HP ) - bullet -> dmg );
                        monsterStorage[ collideID ] -> SetVar( VAR_ENTITY_DAMAGED, ENTITY_DAMAGED );
                        //ShatterBloodEntity( monsterStorage[ collideID ] );
                        bloodStorage -> AddPoints( AddPoint( monsterStorage[ collideID ] -> HotSpot(), Point( 0.0, 0.0, 0.0 ) ), Random( MIN_ENTITY_DAMAGED_DROP * bullet -> dmg, MAX_ENTITY_DAMAGED_DROP * bullet -> dmg ), bullet -> velocity, STD_DEATH_PUSH * bullet -> dmg );
                        // Well I will need some brutal hit sounds here.
                        // Dogs are mute while getting hits.
                        if ( monsterStorage[ collideID ] -> GetVar( VAR_HP ) <= 0.0 ) {
                            // monster death
                            //KillBloodEntity( monsterStorage[ collideID ] );
                            bloodStorage -> AddPoints( AddPoint( monsterStorage[ collideID ] -> HotSpot(), Point( 0.0, 0.0, 0.0 ) ), Random( MIN_ENTITY_BLOOD_DROP, MAX_ENTITY_BLOOD_DROP ), bullet -> velocity, STD_DEATH_PUSH * bullet -> dmg );
                            PlayCustomSound( CHANNEL_DEATH, monsterStorage[ collideID ] -> GetVar( VAR_DEATH_SOUND ) );
                            SetVolumeDist( CHANNEL_DEATH, Dist3D( player -> FixedPlayerSpot(), monsterStorage[ collideID ] -> HotSpot() ), 0.25 );
                            double fleshSize = Random( monsterStorage[ collideID ] -> GetVar( VAR_FLESHSIZE_MIN ), monsterStorage[ collideID ] -> GetVar( VAR_FLESHSIZE_MAX ) );
                            int fleshCount = Random( monsterStorage[ collideID ] -> GetVar( VAR_FLESHCOUNT_MIN ), monsterStorage[ collideID ] -> GetVar( VAR_FLESHCOUNT_MAX ) );
                            fleshStorage -> GenerateFlesh( bullet -> position, bullet -> velocity, monsterStorage[ collideID ] -> HotSpot(), bullet -> spd, fleshCount, fleshSize, 0.6 );
                            delete monsterStorage[ collideID ];
                            monsterStorage[ collideID ] = NULL;
                        }
                        DisposeBullet( i );
                        break;
                    }
                } else {
                    DisposeBullet( i );
                    break;
                }
            }
        }
    }
}

int monsterSpawnPending[ MAX_OBJECT ];

void MonsterSpawnCycle() {
    // just increment pending spawn count for now
    //monsterSpawnPending++;
    // WELL, it's not that easy now!
    for ( int i = 0; i < currentSpawnScheme -> SpawnNumber(); i++ ) {
        if ( currentSpawnScheme -> SpawnTime( i ) ) {
            monsterSpawnPending[ currentSpawnScheme -> SpawnID( i ) ]++;
        }
    }
}

/*void Model::Display( Point3D pos, Point3D posDir, double entitySpeedMul ) {
    double T = Timer::Current() * ( 2 * PI );
    Point3D posDiff = SubtractPoint( posDir, pos );
    double r, incl, azim;
    for ( int i = 0; i < blockNum; i++ ) {
        ModelBlock m = ( *mArray )[ i ];
        Point3D posDiffBlock;
        if ( int( m.var[ BOOL_MODEL_HEAD ] ) == 2 ) {
            posDiffBlock = SubtractPoint( posDiff, m.offset );
        } else {
            posDiffBlock = posDiff;
        }
        ctp( posDiffBlock.x, posDiffBlock.z, &r, &azim );
        ctp( r, posDiffBlock.y, NULL, &incl );
        double prCY = degr( azim );
        int textureUsed = ( int )( m.var[ TEXTURE_INDEX ] );
        if ( image[ textureUsed ] ) {
            if ( image[ textureUsed ] -> UseImage() ) {
                Point3D rotOffset = m.offset;
                double Cr, Ca, azimBlock;
                ctp( posDiff.x, posDiff.z, NULL, &azimBlock );
                ctp( rotOffset.x, rotOffset.z, &Cr, &Ca );
                Ca += azimBlock;
                ptc( Cr, Ca, &rotOffset.x, &rotOffset.z );
                if ( int( m.var[ BOOL_MODEL_HEAD ] ) == 1 ) {
                    double radXZ = sqrt( posDiff.x * posDiff.x + posDiff.z * posDiff.z );
                    double Ci, inclBlock, angleXZ;
                    ctp( posDiff.y, radXZ, NULL, &inclBlock );
                    ctp( rotOffset.x, rotOffset.z, &radXZ, &angleXZ );
                    ctp( rotOffset.y, radXZ, &Cr, &Ci );
                    double cMulIB = 1.0;
                    if ( m.offset.z < 0.0 ) {
                        cMulIB = -1.0;
                    }
                    Ci += inclBlock * cMulIB;
                    ptc( Cr, Ci, &rotOffset.y, &radXZ );
                    ptc( radXZ, angleXZ, &rotOffset.x, &rotOffset.z );
                }
                Point3D posBlock;
                posBlock = rotOffset;//AddPoint( pos, rotOffset );
                Block* block = m.block;
                if ( block ) {
                    //block -> RedefineHotSpot( posBlock.x, posBlock.y, posBlock.z );
                    double legAmp = sin( T * m.var[ SPD_AZIMUTH ] ) * m.var[ MAX_AZIMUTH ] * m.var[ DIR_AZIMUTH ] * entitySpeedMul;
                    double legAddInclination = sin( T * m.var[ SPD_INCLINATION ] ) * m.var[ MAX_INCLINATION ] * m.var[ DIR_INCLINATION ];
                    //block -> DrawRotated( &posBlock, Point( 0, 0, 0 ), m.var[ STD_AZIMUTH ] + legAmp + ( bool )( m.var[ BOOL_MODEL_HEAD ] ) * degr( incl + DEGR90 ), m.var[ STD_INCLINATION ] + prCY + legAddInclination, 0 );
                    //block -> DrawRotated_JoinModel( posBlock, m.var[ STD_AZIMUTH ] + legAmp + ( bool )( m.var[ BOOL_MODEL_HEAD ] ) * degr( incl + DEGR90 ), m.var[ STD_INCLINATION ] + prCY + legAddInclination, 0 );
                    block -> RedefineHotSpot( pos.x, pos.y, pos.z );
                    block -> Model_Start( m.var[ STD_AZIMUTH ] + legAmp + ( bool )( m.var[ BOOL_MODEL_HEAD ] ) * degr( incl + DEGR90 ), m.var[ STD_INCLINATION ] + prCY + legAddInclination, 0 );
                    //block -> DrawRotated_JoinModel( posBlock );
                    block -> Model_End();
                    // &m.hotspot
                }
            }
        }
    }
}*/

/*void Model::Display( Point3D pos, Point3D posDir, double entitySpeedMul ) {
    double T = Timer::Current() * ( 2 * PI );
    Point3D posDiff = SubtractPoint( posDir, pos );

}*/

#define STD_GRAVITY_POWER 0.5
#define STD_GRAVITY_INERTIA 0.98

StringLinker* audio_DamagePlayer = NULL;

void InitAudio_Player() {
    audio_DamagePlayer = new StringLinker( "data/audio/playerDamage.slf" );
    audio_ShootSounds = new StringLinker( "data/audio/weaponShoot.slf" );
}

void DeleteAudio_Player() {
    delete audio_DamagePlayer;
    //printf( "audio_ShootSounds: %p\n", audio_ShootSounds );
    delete audio_ShootSounds;
}

void DamagePlayer( double damage ) {
    // Sounds
    // NOTE: NEEDS MORE DAMAGE SOUNDS!
    // SUBNOTE: ok now, linked to external file.
    if ( audio_DamagePlayer ) {
        if ( audio_DamagePlayer -> GetSize() > 0 ) {
            int aIndex = rand() % audio_DamagePlayer -> GetSize();
            PlaySound( CHANNEL_PLAYER_DAMAGE, audio_DamagePlayer -> GetStringOf( aIndex ) );
        }
    }
    // /Sounds
    player -> SetVar( VAR_HP, player -> GetVar( VAR_HP ) - damage );
    for ( int i = 0; i < 3; i++ ) {
        MainColor[ i ] = 0.0;
    }
}

#include "Logic.h"

void MonsterControl() {
    bool kayley_Exists = false;
    for ( int i = 0; i < MAX_MONSTERS; i++ ) {
        if ( monsterStorage[ i ] ) {
            /*
            Point3D velocity = SubtractPoint( player -> HotSpot(), monsterStorage[ i ] -> HotSpot() );
            double dist = Dist3D( player -> HotSpot(), monsterStorage[ i ] -> HotSpot() );
            velocity.y = 0.0;
            if ( dist > 0.0 ) {
                velocity.x *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
                velocity.z *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
            }
            if ( dist < 0.75 ) { // damage player and die
                DamagePlayer( Random( monsterStorage[ i ] -> GetVar( VAR_DMG_MIN ), monsterStorage[ i ] -> GetVar( VAR_DMG_MAX ) ) );
                delete monsterStorage[ i ];
                monsterStorage[ i ] = NULL;
                continue;
            }
            for ( int j = 0; j < MAX_MONSTERS; j++ ) {
                if ( ( i != j ) && ( monsterStorage[ j ] ) ) {
                    monsterStorage[ i ] -> CollideEntity( monsterStorage[ j ], &velocity, 0.3 );
                }
            }
            monsterStorage[ i ] -> GravityAffect( MainMap, STD_GRAVITY_POWER, STD_GRAVITY_INERTIA, &velocity );
            monsterStorage[ i ] -> MoveAtMap( MainMap, velocity );
            if ( dist > 24.0 ) {
                delete monsterStorage[ i ];
                monsterStorage[ i ] = NULL;
            }
            // END HERE!
            */
            int current_Logic = monsterStorage[ i ] -> GetVar( VAR_LOGIC );
            switch ( current_Logic ) {
                case LOGIC_DOG: {
                    Dog_Logic( i );
                    break;
                }
                case LOGIC_KAYLEY: {
                    Kayley_Logic( i );
                    kayley_Exists = true;
                    break;
                }
                case LOGIC_HORSE: {
                    Horse_Logic( i );
                    break;
                }
                case LOGIC_FISH: {
                    Fish_Logic( i );
                    break;
                }
                case LOGIC_GUARDIAN: {
                    Guardian_Logic( i );
                    break;
                }
                default: {
                    break;
                }
            }
            //if ( monsterStorage[ i ] ) { // status post-update // UNUSED, external remove checking now
            monsterStorage[ i ] -> SetVar( VAR_ENTITY_DAMAGED, ENTITY_NOT_DAMAGED );
            //}
        }
    }
    if ( !kayley_Exists ) {
        StopSound( CHANNEL_KAYLEY );
    }
    lastShootOccuredPos = lastShootOccuredPos_NOSHOOT;
    CheckRemoveMonsters();
}

void DisplayBullets() {
    glDisable( GL_LIGHTING );
    glDisable( GL_TEXTURE_2D );
    glLineWidth( 4.25f );
    glBegin( GL_LINES );
    //if ( image[ TEXTURE_CURSOR ] -> UseImage() ) {
    for ( int i = 0; i < MAX_BULLETS; i++ ) {
        if ( bulletStorage[ i ] ) {
            bulletStorage[ i ] -> Display_Fast();
        }
    }
    //}
    glEnd();
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_LIGHTING );
}


