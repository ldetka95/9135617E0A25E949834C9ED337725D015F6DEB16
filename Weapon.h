#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include "Object.h"

#define MAP_INVALID_KEY -1

class Ammo {
    public:
    Ammo( string path ) {
        box = 0;
        FILE* handle = fopen( path.c_str(), "r" );
        ac = Point( 1.0, 1.0, 1.0 );
        if ( handle ) {
            // model link read
            //char pathBuffer[ STD_BUFFER_SIZE ];
            //fscanf( handle, "%[^\n]", pathBuffer );
            //box -> FromFile( pathBuffer );
            // data
            fscanf( handle, "%lf %lf %lf\n", &sx, &sy ,&sz );
            fscanf( handle, "%d %d %d\n", &box, &inf, &stdGive );
            fscanf( handle, "%lf %lf %lf\n", &ac.x, &ac.y, &ac.z );
            // close handle
            fclose( handle );
        }
        bullets = 0;
        name = path;
    }
    ~Ammo() {
        /*if ( box ) {
            delete box;
        }*/
    }
    int GetBox() {
        return box;
    }
    bool ShotIsGiven( int amount ) {
        if ( inf ) {
            return true;
        }
        int newBullets = bullets - amount;
        if ( newBullets >= 0 ) {
            bullets = newBullets;
            return true;
        }
        return false;
    }
    double GetAmountPickup() {
        return stdGive;
    }
    double GetBulletCount() {
        return bullets;
    }
    bool IsInfinity() {
        return ( inf > 0 );
    }
    void SetInfinity() {
        inf = 1;
    }
    double SizeX() {
        return sx;
    }
    double SizeY() {
        return sy;
    }
    double SizeZ() {
        return sz;
    }
    void PickedSomeAmmo() {
        bullets += stdGive;
    }
    string GetName() {
        return name;
    }
    Point3D BulletColor() {
        return ac;
    }
    private:
    // backpack info
    int bullets; // reamining bullet count in backpack
    int inf; // is infinity?
    // box pickup info
    int box; // texture ID only
    int stdGive; // how much player will obtain after pickup
    double sx, sy, sz; // box dimensions
    // others
    string name; // path loaded from
    Point3D ac; // bullet color
};

class AmmoStash {
    public:
    AmmoStash( string path ) {
        ammoArray = NULL;
        Resize( 0 );
        StringLinker* wpnLinker = new StringLinker( path );
        for ( int i = 0; i < ( wpnLinker -> GetSize() ); i++ ) {
            Append( wpnLinker -> GetIdOf( i ), new Ammo( wpnLinker -> GetStringOf( i ) ) );
        }
        delete wpnLinker;
    }
    ~AmmoStash() {
        Resize( 0 );
    }
    void Append( int k, Ammo* wpn ) {
        Resize( ammoCount + 1 );
        ( *ammoArray )[ ammoCount - 1 ].key = k;
        ( *ammoArray )[ ammoCount - 1 ].a = wpn;
    }
    void Resize( int size ) {
        /*for ( int i = ammoCount - 1; i >= size; i-- ) {
            if ( ( *ammoArray )[ i ].a ) {
                delete ( *ammoArray )[ i ].a;
            }
        }*/
        if ( size > 0 ) {
            ammoArray = ( AmmoStashElement (*)[] )( realloc( ammoArray, sizeof( AmmoStashElement ) * size ) );
            for ( int i = ammoCount; i < size; i++ ) {
                ( *ammoArray )[ i ].key = MAP_INVALID_KEY;
                ( *ammoArray )[ i ].a = NULL;
            }
        } else if ( ammoArray ) {
            free( ammoArray );
            ammoArray = NULL;
        }
        ammoCount = size;
    }
    Ammo* GetAmmo( int key ) {
        for ( int i = 0; i < ammoCount; i++ ) {
            if ( ( *ammoArray )[ i ].key == key ) {
                return ( *ammoArray )[ i ].a;
            }
        }
        return NULL;
    }
    int GetAmmoKey( Ammo* amm ) {
        for ( int i = 0; i < ammoCount; i++ ) {
            if ( ( *ammoArray )[ i ].a == amm ) {
                return ( *ammoArray )[ i ].key;
            }
        }
        return MAP_INVALID_KEY;
    }
    Ammo* GetAmmoByIndex( int i ) {
        return ( *ammoArray )[ i ].a;
    }
    int GetAmmoKeyByIndex( int i ) {
        return ( *ammoArray )[ i ].key;
    }
    int Quantity() {
        return ammoCount;
    }
    private:
    class AmmoStashElement {
        public:
        int key;
        Ammo* a;
    };
    AmmoStashElement ( *ammoArray )[];
    int ammoCount;
};

AmmoStash* ammoStash;
//Block* mainBlockDisplayer;

#define STD_PICKUP_RANGE 0.75
#define STD_PICK_ROTATION_SPEED 0.2 * PI // rotate once for 10 sec
#define STD_PICK_PULSE_AMP 0.15

class AmmoBox {
    public:
    AmmoBox( Ammo* ammo, double amount, double x, double y, double z ) {
        pos = Point( x, y, z );
        ammoGiven = ammo;
        numGiven = amount;
        rotationSpeed = STD_PICK_ROTATION_SPEED;
        pulseAmp = STD_PICK_PULSE_AMP;
        spawner = NULL;
    }
    ~AmmoBox() {
    }
    int Display( Point3D posP ) {
        if ( ammoGiven ) {
            /*Block* boxM = ammoGiven -> GetBox();
            if ( boxM ) {
                boxM -> Display( pos );
            }*/
            int imageID = ammoGiven -> GetBox();
            if ( image[ imageID ] ) {
                if ( image[ imageID ] -> UseImage() ) {
                    //printf( "Drawing attempt\n" );
                    double curTime = Timer::Current() * rotationSpeed;
                    Block* block = new Block( ammoGiven -> SizeX(), ammoGiven -> SizeY(), ammoGiven -> SizeZ() );
                    //printf( "Draw size: %lf %lf %lf\n", ammoGiven -> SizeX(), ammoGiven -> SizeY(), ammoGiven -> SizeZ() );
                    double ax = sin( curTime );
                    double ay = cos( curTime );
                    double angleRotate = 0.0;
                    ctp( ax, ay, NULL, &angleRotate );
                    //printf( "Current angle: %lf\n", angleRotate * 180.0 / PI );
                    Point3D posCurrent = pos;
                    posCurrent.y += sin( curTime ) * pulseAmp;
                    block -> DrawRotated( NULL, posCurrent, 0.0, angleRotate * 180.0 / PI, 0.0 );
                    //printf( "Pos drawing: %lf %lf %lf\n", posCurrent.x, posCurrent.y, posCurrent.z );
                    delete block;
                    return 1;
                }
            }
        }
        return 0;
    }
    void* GetSpawner() {
        return spawner;
    }
    void SetSpawner( void* s ) {
        spawner = s;
    }
    bool IsCloseTo( Point3D posP ) {
        double dist = Dist3D( pos, posP );
        if ( dist < STD_PICKUP_RANGE ) {
            return true;
        }
        return false;
    }
    Ammo* GetAmmoType() {
        return ammoGiven;
    }
    private:
    Ammo* ammoGiven;
    double numGiven;
    Point3D pos;
    double rotationSpeed, pulseAmp;
    void* spawner;
};

class AmmoBoxStack { // stack of in-game pickups
    public:
    AmmoBoxStack() {
        stack = NULL;
    }
    ~AmmoBoxStack() {
        Flush();
    }
    void Flush() {
        AmmoBoxStackElement* seeker = stack;
        while ( seeker ) {
            AmmoBoxStackElement* grinder = seeker;
            seeker = seeker -> next;
            delete grinder;
        }
        stack = NULL;
    }
    void Push( AmmoBox* b ) {
        AmmoBoxStackElement* tmp = new AmmoBoxStackElement();
        tmp -> box = b;
        tmp -> next = stack;
        stack = tmp;
    }
    /*void RemoveNull() {
        AmmoBoxStackElement* seeker = stack;
        AmmoBoxStackElement* sshadow = stack;
        while ( seeker ) {
            if ( !( seeker -> box ) ) {
                // remove

            }
            sshadow = seeker;
            seeker = seeker -> next;
        }
    }*/
    void TryPickUp( Point3D posP );
    void DisplayAll( Point3D posP ) {
        AmmoBoxStackElement* seeker = stack;
        //int cDisp = 0;
        //int tDisp = 0;
        while ( seeker ) {
            if ( seeker -> box ) {
                seeker -> box -> Display( posP );
                //cDisp +=
            }
            seeker = seeker -> next;
            //tDisp++;
        }
        //printf( "cDisp = %d, tDisp = %d\n", cDisp, tDisp );
    }
    private:
    class AmmoBoxStackElement {
        public:
        AmmoBoxStackElement* next;
        AmmoBox* box;
    };
    AmmoBoxStackElement* stack;
};

AmmoBoxStack* ammoBoxStack = NULL;

class AmmoSpawn { // places spawning in-game pickups
    // Spawns a box of ammo, then waits for pick up. Box should wake spawner while picked.
    public:
    AmmoSpawn( Ammo* spawning, double x, double y, double z, double timeInterval ) {
        if ( timeInterval < 0 ) {
            timer = new Timer( INFINITY );
        } else {
            timer = new Timer( timeInterval );
        }
        ammo = spawning;
        spawnPos = Point( x, y, z );
        timer -> ForceTick();
    }
    ~AmmoSpawn() {
        delete timer;
    }
    void Spawn( AmmoBoxStack* abstack ) {
        if ( abstack ) {
            AmmoBox* newBox = Spawn();
            if ( newBox ) {
                abstack -> Push( newBox );
            }
        }
    }
    AmmoBox* Spawn() {
        if ( !( timer -> Tick() ) ) { // can't spawn now, or spawned already
            return NULL;
        }
        Sleep();
        AmmoBox* box = new AmmoBox( ammo, ammo -> GetAmountPickup(), spawnPos.x, spawnPos.y, spawnPos.z );
        box -> SetSpawner( ( void* )( this ) );
        return box;
    }
    void Sleep() {
        if ( !( timer -> IsPaused() ) ) {
            timer -> Pause();
        }
    }
    void Awake() { // should be called by an external object
        if ( timer -> IsPaused() ) {
            timer -> Resume();
        }
    }
    Point3D GetSpawnPosition() {
        return spawnPos;
    }
    Ammo* SpawningAmmo() {
        return ammo;
    }
    double TimeInterval() {
        return timer -> GetInterval();
    }
    private:
    Ammo* ammo;
    Point3D spawnPos;
    Timer* timer;
};

class AmmoSpawnStack {
    public:
    AmmoSpawnStack() {
        stack = NULL;
    }
    ~AmmoSpawnStack() {
        Flush();
    }
    void Flush() {
        AmmoSpawnStackElement* seeker = stack;
        while ( seeker ) {
            AmmoSpawnStackElement* grinder = seeker;
            seeker = seeker -> next;
            delete grinder;
        }
        stack = NULL;
    }
    void Add( AmmoSpawn* ammoSpawn ) {
        AmmoSpawnStackElement* tmp = new AmmoSpawnStackElement();
        tmp -> spawn = ammoSpawn;
        tmp -> next = stack;
        stack = tmp;
    }
    void TryRemove( Point3D pos, double inRadius ) {
        double smallestRadius = inRadius;
        AmmoSpawnStackElement* toRemove = NULL;
        AmmoSpawnStackElement* toRemovePrev = NULL;
        AmmoSpawnStackElement* seeker = stack;
        AmmoSpawnStackElement* sshadow = NULL;
        while ( seeker ) { // get smallest distance
            if ( seeker -> spawn ) {
                double cDist = Dist3D( pos, seeker -> spawn -> GetSpawnPosition() );
                if ( cDist < smallestRadius ) {
                    toRemove = seeker;
                    toRemovePrev = sshadow;
                    smallestRadius = cDist;
                }
            }
            sshadow = seeker;
            seeker = seeker -> next;
        }
        if ( toRemove ) { // remove
            if ( toRemovePrev ) {
                toRemovePrev -> next = toRemove -> next;
            } else {
                stack = toRemove -> next;
            }
            delete toRemove;
        }
    }
    double NearestDist( Point3D pos ) {
        double sdist = INFINITY;
        AmmoSpawnStackElement* seeker = stack;
        while ( seeker ) { // get smallest distance
            if ( seeker -> spawn ) {
                double cDist = Dist3D( pos, seeker -> spawn -> GetSpawnPosition() );
                if ( cDist < sdist ) {
                    sdist = cDist;
                }
            }
            seeker = seeker -> next;
        }
        return sdist;
    }
    void DisplayAll() {
        Block block( 1.0, 1.0, 1.0 );
        AmmoSpawnStackElement* seeker = stack;
        while ( seeker ) {
            if ( seeker -> spawn ) {
                Ammo* ammoSpawned = seeker -> spawn -> SpawningAmmo();
                if ( ammoSpawned ) {
                    int box = ammoSpawned -> GetBox();
                    if ( image[ box ] ) {
                        if ( image[ box ] -> UseImage() ) {
                            block.DrawRotated( NULL, seeker -> spawn -> GetSpawnPosition(), 0.0, 0.0, 0.0 );
                        }
                    }
                }
            }
            seeker = seeker -> next;
        }
    }
    void SpawnAmmoCycle() {
        AmmoSpawnStackElement* seeker = stack;
        while ( seeker ) {
            if ( seeker -> spawn ) {
                seeker -> spawn -> Spawn( ammoBoxStack );
            }
            seeker = seeker -> next;
        }
    }
    void Load( string pathR ) {
        string path = pathR + "spawnammo.dat";
        FILE* handle = fopen( path.c_str(), "r" );
        if ( handle ) {
            Point3D pos;
            double tInterval;
            int ammoSpawnKey;
            while ( fscanf( handle, "%lf %lf %lf %d %lf\n", &pos.x, &pos.y, &pos.z, &ammoSpawnKey, &tInterval ) > 0 ) {
                AmmoSpawn* push = new AmmoSpawn( ammoStash -> GetAmmo( ammoSpawnKey ), pos.x, pos.y, pos.z, tInterval );
                Add( push );
            }
            fclose( handle );
        }
    }
    void Save( string pathR ) {
        string path = pathR + "spawnammo.dat";
        FILE* handle = fopen( path.c_str(), "w" );
        if ( handle ) {
            AmmoSpawnStackElement* seeker = stack;
            while ( seeker ) {
                AmmoSpawn* pop = seeker -> spawn;
                Point3D pos = pop -> GetSpawnPosition();
                int ammoSpawnKey = ammoStash -> GetAmmoKey( pop -> SpawningAmmo() );
                double tInterval = pop -> TimeInterval();
                fprintf( handle, "%lf %lf %lf %d %lf\n", pos.x, pos.y, pos.z, ammoSpawnKey, tInterval );
                seeker = seeker -> next;
            }
            fclose( handle );
        }
    }
    private:
    class AmmoSpawnStackElement {
        public:
        AmmoSpawn* spawn;
        AmmoSpawnStackElement* next;
    };
    AmmoSpawnStackElement* stack;
};

AmmoSpawnStack* ammoSpawnStack = NULL;

void AmmoBoxStack::TryPickUp( Point3D posP ) {
    AmmoBoxStackElement* seeker = stack;
    AmmoBoxStackElement* sshadow = NULL;
    while ( seeker ) {
        bool switchedToNext = false;
        if ( seeker -> box ) {
            bool canPickUp = seeker -> box -> IsCloseTo( posP );
            if ( canPickUp ) {
                // inform spawn that this ammo is picked up
                AmmoSpawn* spawn = ( AmmoSpawn* )( seeker -> box -> GetSpawner() );
                if ( spawn ) {
                    spawn -> Awake();
                }
                // give benefits
                Ammo* ammo = seeker -> box -> GetAmmoType();
                if ( ammo ) {
                    ammo -> PickedSomeAmmo();
                }
                PlaySound( CHANNEL_PICKUP, "pickUp.ogg" );
                // remove
                if ( seeker == stack ) {
                    stack = seeker -> next;
                } else {
                    sshadow -> next = seeker -> next;
                }
                AmmoBoxStackElement* grinder = seeker;
                switchedToNext = true;
                seeker = seeker -> next;
                delete grinder;
            }
        }
        if ( !switchedToNext ) {
            sshadow = seeker;
            seeker = seeker -> next;
        }
    }
}


#define WEAPON_TOTAL_VAR 64

#define WEAPON_VAR_AMMOUSEDKEY 0
#define WEAPON_VAR_SHOOTINGDELAY 1
#define WEAPON_VAR_MINDAMAGE 2
#define WEAPON_VAR_MAXDAMAGE 3
#define WEAPON_VAR_RECOIL 4
#define WEAPON_VAR_BULLETSHOOTCOUNT 5
#define WEAPON_VAR_BULLETDISPERSION 6
#define WEAPON_VAR_RECOIL_AMPLITUDE 7
#define WEAPON_VAR_BULLETAMOUNTUSED 8
#define WEAPON_VAR_BULLETVEL 9
#define WEAPON_VAR_BULLETSPEED 10
#define WEAPON_VAR_SHOOTSOUNDID 32
#define WEAPON_VAR_MODELID 33
#define WEAPON_VAR_SHOOTOFFSET_X 34
#define WEAPON_VAR_SHOOTOFFSET_Y 35
#define WEAPON_VAR_SHOOTOFFSET_Z 36
#define WEAPON_VAR_FOVSCOPE 37
#define WEAPON_VAR_SCOPINGTIME 38
#define WEAPON_VAR_LOCKED 63

#define SHOOT_CORRECT 0
#define SHOOT_NOAMMO 1
#define SHOOT_BADTIME 2
#define SHOOT_INCORRECT 3

#define MAX_RECOIL_AMPLITUDE 70.0

StringLinker* audio_ShootSounds = NULL;

class Weapon {
    public:
    Weapon( string path ) {
        for ( int i = 0; i < WEAPON_TOTAL_VAR; i++ ) {
            var[ i ] = 0.0;
        }
        FILE* handle = fopen( path.c_str(), "r" );
        if ( handle ) {
            for ( int i = 0; i < WEAPON_TOTAL_VAR; i++ ) {
                fscanf( handle, "%lf ", &( var[ i ] ) );
            }
            fclose( handle );
        }
        lastShotTime = 0.0;
        name = path;
    }
    ~Weapon() {
    }
    double GetVar( int index ) {
        return var[ index ];
    }
    void SetVar( int index, double value ) {
        var[ index ] = value;
    }
    // individual methods
    int TryShoot() {
        Ammo* ammo = ammoStash -> GetAmmo( ( int )( var[ WEAPON_VAR_AMMOUSEDKEY ] ) );
        double curTime = Timer::Current();
        if ( ( ammo ) && ( curTime >= ( lastShotTime + var[ WEAPON_VAR_SHOOTINGDELAY ] ) ) ) {
            lastShotTime = curTime;
            int ret = ammo -> ShotIsGiven( var[ WEAPON_VAR_BULLETAMOUNTUSED ] );
            if ( ( ret ) && ( audio_ShootSounds ) ) {
                int givenID = ( int )( var[ WEAPON_VAR_SHOOTSOUNDID ] );
                //printf( "givenID = %d\n", givenID );
                PlaySound( CHANNEL_SHOOT, audio_ShootSounds -> GetString( givenID ) );
            } else if ( !ret ) {
                PlaySound( CHANNEL_SHOOT, "weapon/empty.ogg" );
            }
            badTried = ret;
            return ( int )( !ret );
        }
        return SHOOT_INCORRECT;
    }
    double GetRandomDamage() {
        return Random( var[ WEAPON_VAR_MINDAMAGE ], var[ WEAPON_VAR_MAXDAMAGE ] );
    }
    void Display( double x, double y, double z, double angle ) {
        int modelID = var[ WEAPON_VAR_MODELID ];
        Point3D pos = Point( x, y, z );
        double dx, dz;
        ptc( 1, angle, &dx, &dz );
        Point3D posDir = Point( x + dx, y, z + dz );
        if ( mainModel[ modelID ] ) {
            mainModel[ modelID ] -> Display( pos, posDir, 1.0 );
        }
    }
    void Display( double x, double y, double z, double dx, double dy, double dz ) {
        int modelID = var[ WEAPON_VAR_MODELID ];
        Point3D pos = Point( x, y, z );
        double radius, angle;
        ctp( dy, dz, &radius, &angle );
        angle -= GetCurrentRecoilAngle() * PI / 180.0;
        ptc( radius, angle, &dy, &dz );
        Point3D posDir = Point( dx, dy, dz );
        if ( mainModel[ modelID ] ) {
            mainModel[ modelID ] -> Display( pos, posDir, 1.0 );
        }
    }
    double GetCurrentRecoilAngle() {
        double curTime = Timer::Current();
        return ( int )( badTried ) * ( 1.0 - Slap( ( curTime - lastShotTime ) / var[ WEAPON_VAR_SHOOTINGDELAY ], 0.0, 1.0 ) ) * var[ WEAPON_VAR_RECOIL_AMPLITUDE ];//MAX_RECOIL_AMPLITUDE;
    }
    string GetName() {
        return name;
    }
    Ammo* UsedAmmo() {
        return ammoStash -> GetAmmo( var[ WEAPON_VAR_AMMOUSEDKEY ] );
    }
    private:
    double var[ WEAPON_TOTAL_VAR ];
    double lastShotTime;
    bool badTried;
    string name;
};

class WeaponStash {
    public:
    WeaponStash() {
        wseArray = NULL;
        Resize( 0 );
    }
    WeaponStash( string path ) {
        wseArray = NULL;
        Resize( 0 );
        StringLinker* wpnLinker = new StringLinker( path );
        for ( int i = 0; i < ( wpnLinker -> GetSize() ); i++ ) {
            Append( wpnLinker -> GetIdOf( i ), new Weapon( wpnLinker -> GetStringOf( i ) ) );
        }
        delete wpnLinker;
    }
    ~WeaponStash() {
        Resize( 0 );
    }
    void Append( int k, Weapon* wpn ) {
        Resize( wseCount + 1 );
        ( *wseArray )[ wseCount - 1 ].key = k;
        ( *wseArray )[ wseCount - 1 ].w = wpn;
    }
    void Resize( int size ) {
        /*for ( int i = wseCount - 1; i >= size; i-- ) {
            if ( ( *wseArray )[ i ].w ) {
                delete ( *wseArray )[ i ].w;
            }
        }*/
        if ( size > 0 ) {
            wseArray = ( WeaponStashElement (*)[] )( realloc( wseArray, sizeof( WeaponStashElement ) * size ) );
            for ( int i = wseCount; i < size; i++ ) {
                ( *wseArray )[ i ].key = MAP_INVALID_KEY;
                ( *wseArray )[ i ].w = NULL;
            }
        } else if ( wseArray ) {
            free( wseArray );
            wseArray = NULL;
        }
        wseCount = size;
    }
    Weapon* GetWeapon( int key ) {
        for ( int i = 0; i < wseCount; i++ ) {
            if ( ( *wseArray )[ i ].key == key ) {
                return ( *wseArray )[ i ].w;
            }
        }
        return NULL;
    }
    int GetWeaponKey( Weapon* wpn ) {
        for ( int i = 0; i < wseCount; i++ ) {
            if ( ( *wseArray )[ i ].w == wpn ) {
                return ( *wseArray )[ i ].key;
            }
        }
        return MAP_INVALID_KEY;
    }
    Weapon* GetWeaponByIndex( int i ) {
        return ( *wseArray )[ i ].w;
    }
    int GetWeaponKeyByIndex( int i ) {
        return ( *wseArray )[ i ].key;
    }
    /* // CHECK IF USEFUL
    void Delete( int key ) {
    }
    void Delete( Weapon* wpn ) {
    }
    */
    int Quantity() {
        return wseCount;
    }
    private:
    class WeaponStashElement {
        public:
        int key;
        Weapon* w;
    };
    WeaponStashElement ( *wseArray )[];
    int wseCount;
};

WeaponStash* weaponStash;

/// UPDATE 7.05.2016

class WeaponSpawn {
    public:
    WeaponSpawn( Weapon* w, Point3D pos ) {
        spawnPos = pos;
        weapon = w;
        pickCount = 1;
    }
    ~WeaponSpawn() {
    }
    void Display( bool editMode ) {
        if ( weapon ) {
            if ( ( editMode ) || ( pickCount > 0 ) ) {
                double t = Timer::Current();
                double angle = t * STD_PICK_ROTATION_SPEED;
                double pulse = sin( angle ) * STD_PICK_PULSE_AMP;
                weapon -> Display( spawnPos.x, spawnPos.y + pulse, spawnPos.z, angle );
            }
        }
    }
    double DistTo( Point3D pos ) {
        return Dist3D( pos, spawnPos );
    }
    Weapon* GetWeapon() {
        return weapon;
    }
    Point3D SpawnPosition() {
        return spawnPos;
    }
    bool PickUp() {
        if ( pickCount > 0 ) {
            pickCount--;
            return true;
        }
        return false;
    }
    private:
    Point3D spawnPos;
    Weapon* weapon;
    int pickCount;
};

class WeaponSpawnStack {
    public:
    WeaponSpawnStack() {
    }
    ~WeaponSpawnStack() {
        Flush();
    }
    void Add( WeaponSpawn* spawn ) {
        lst.push_front( spawn );
    }
    bool TryRemove( Point3D pos, double catchRadius ) {
        double sdist = catchRadius;
        int sws_index = -1;
        for ( UINT32 i = 0; i < lst.size(); i++ ) {
            WeaponSpawn* ws = lst.front();
            lst.pop_front();
            double cdist = ws -> DistTo( pos );
            if ( cdist < sdist ) {
                sdist = cdist;
                sws_index = ( int )( i );
            }
            lst.push_back( ws );
        }
        if ( sws_index >= 0 ) {
            for ( UINT32 i = 0; i < lst.size(); i++ ) {
                WeaponSpawn* ws = lst.front();
                lst.pop_front();
                if ( ( int )( i ) == sws_index ) {
                    delete ws;
                    return true;
                }
                lst.push_back( ws );
            }
        }
        return false;
    }
    Weapon* GetNearest( Point3D pos, double catchRadius, bool pick = false ) {
        WeaponSpawn* nws = NULL;
        double sdist = catchRadius;
        for ( UINT32 i = 0; i < lst.size(); i++ ) {
            WeaponSpawn* ws = lst.front();
            lst.pop_front();
            double cdist = ws -> DistTo( pos );
            if ( cdist < sdist ) {
                sdist = cdist;
                nws = ws;
            }
            lst.push_back( ws );
        }
        Weapon* w = NULL;
        if ( nws ) {
            w = nws -> GetWeapon();
            if ( pick ) {
                if ( !nws -> PickUp() ) {
                    return NULL;
                }
            }
        }
        return w;
    }
    void Load( string pathR ) {
        string path = pathR + "spawnweapon.dat";
        FILE* handle = fopen( path.c_str(), "r" );
        if ( handle ) {
            Point3D pos;
            int wid;
            while ( fscanf( handle, "%lf %lf %lf %d\n", &pos.x, &pos.y, &pos.z, &wid ) > 0 ) {
                WeaponSpawn* push = new WeaponSpawn( weaponStash -> GetWeapon( wid ), pos );
                Add( push );
            }
            fclose( handle );
        }
    }
    void Save( string pathR ) {
        string path = pathR + "spawnweapon.dat";
        FILE* handle = fopen( path.c_str(), "w" );
        if ( handle ) {
            for ( UINT32 i = 0; i < lst.size(); i++ ) {
                WeaponSpawn* ws = lst.front();
                if ( ws ) {
                    Weapon* w = ws -> GetWeapon();
                    Point3D pos = ws -> SpawnPosition();
                    fprintf( handle, "%lf %lf %lf %d\n", pos.x, pos.y, pos.z, weaponStash -> GetWeaponKey( w ) );
                }
                lst.pop_front();
                lst.push_back( ws );
            }
            fclose( handle );
        }
    }
    void DisplayAll( bool editMode ) {
        for ( UINT32 i = 0; i < lst.size(); i++ ) {
            WeaponSpawn* ws = lst.front();
            if ( ws ) {
                ws -> Display( editMode );
            }
            lst.pop_front();
            lst.push_back( ws );
        }
    }
    Weapon* TryPickUp( Point3D pos, double catchRadius ) {
        Weapon* nearest = GetNearest( pos, catchRadius, true );
        if ( nearest ) {
            nearest -> SetVar( WEAPON_VAR_LOCKED, 0 ); // unlock me
        }
        return nearest;
    }
    void Flush() {
        int lsts = lst.size();
        for ( int i = 0; i < lsts; i++ ) {
            WeaponSpawn* ws = lst.front();
            lst.pop_front();
            delete ws;
        }
    }
    private:
    list < WeaponSpawn* > lst;
};

WeaponSpawnStack* weaponSpawnStack = NULL;

void InitWeapons( string pathWpnStash, string pathAmmoStash ) {
    weaponStash = new WeaponStash( pathWpnStash );
    ammoStash = new AmmoStash( pathAmmoStash );
    ammoBoxStack = new AmmoBoxStack();
    ammoSpawnStack = new AmmoSpawnStack();
}

void RemoveWeapons() {
    //ammoSpawnStack -> Save( MainMap -> GetMapPath() );
    delete ammoSpawnStack;
    delete ammoBoxStack;
    delete ammoStash;
    delete weaponStash;
}

// weapon keys

#define MAX_WEAPON_KEYS 256
int currentWeaponKey = 0;

void PrevWeapon() {
    if ( weaponStash ) {
        Weapon* w = NULL;
        do {
            for ( int i = 0; i < weaponStash -> Quantity(); i++ ) {
                currentWeaponKey--;
                if ( currentWeaponKey < 0 ) {
                    currentWeaponKey = MAX_WEAPON_KEYS - 1;
                }
                w = weaponStash -> GetWeapon( currentWeaponKey );
                if ( w ) {
                    if ( !( w -> GetVar( WEAPON_VAR_LOCKED ) ) ) {
                        break;
                    } else {
                        w = NULL;
                    }
                }
            }
        } while ( !w );
    }
}

void NextWeapon() {
    if ( weaponStash ) {
        Weapon* w = NULL;
        do {
            for ( int i = 0; i < weaponStash -> Quantity(); i++ ) {
                currentWeaponKey++;
                if ( currentWeaponKey >= MAX_WEAPON_KEYS ) {
                    currentWeaponKey = 0;
                }
                w = weaponStash -> GetWeapon( currentWeaponKey );
                if ( w ) {
                    if ( !( w -> GetVar( WEAPON_VAR_LOCKED ) ) ) {
                        break;
                    } else {
                        w = NULL;
                    }
                }
            }
        } while ( !w );
    }
}

void ResetStacks() { // called when loading level
    ammoSpawnStack -> Flush();
    ammoBoxStack -> Flush();
}

