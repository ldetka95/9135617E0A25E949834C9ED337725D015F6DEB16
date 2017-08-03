#pragma once

#include "Core.h"
#include <map>

#define STD_CHEAT_BUFFER 64

class CheatSystem {
    public:
    CheatSystem( double delay ) {
        for ( int i = 0; i < STD_CHEAT_BUFFER; i++ ) {
            ___lch[ i ] = 0;
        }
        ___pos = 0;
        ___t = new Timer( delay * 1000 );
        ___d = delay;
    }
    ~CheatSystem() {
        delete ___t;
    }
    void AppendChar( char c ) {
        ___pos--; // = ( ___pos - 1 ) % STD_CHEAT_BUFFER;
        if ( ___pos < 0 ) {
            ___pos += STD_CHEAT_BUFFER;
        }
        ___lch[ ___pos ] = c;
        ___t -> RestartTick();
    }
    bool DetectedInput( const char* input ) {
        //int p = ___pos;
        int s = strlen( input );
        for ( int i = min( s, STD_CHEAT_BUFFER ) - 1; i >= 0; i-- ) {
            /*int p = ___pos - i;
            if ( p < 0 ) {
                p += STD_CHEAT_BUFFER;
            }*/
            int p = ( ___pos + i ) % STD_CHEAT_BUFFER;
            if ( ___lch[ p ] != input[ s - i - 1 ] ) {
                return false;
            }
        }
        return true;
    }
    void Tick() {
        if ( ___t -> Tick() ) {
            AppendChar( 0 );
        }
        map < string, void* >::iterator i = ___m.begin();
        while ( i != ___m.end() ) {
            const char* kw = ( i -> first ).c_str();
            //printf( "Detecting %s\n", kw );
            if ( DetectedInput( kw ) ) {
                ( ( void (*)() )( i -> second ) )();
                AppendChar( 0 );
                PlaySound( CHANNEL_CHEAT, "cheat.ogg" );
                break;
            }
            i++;
        }
    }
    void AddDetection( const char* k, void ( *f )() ) {
        pair < string, void* > p;
        p.first = string( k );
        p.second = ( void* )( f );
        ___m.insert( p );
    }
    char BufferedNext( int i ) {
        int p = ___pos - i - 1;
        if ( p < 0 ) {
            p += STD_CHEAT_BUFFER ;
        }
        return ___lch[ p ];
    }
    private:
    char ___lch[ STD_CHEAT_BUFFER ];
    int ___pos;
    Timer* ___t;
    double ___d;
    map < string, void* > ___m;
};

CheatSystem* cheatSystem = NULL;

void CHEAT_Unlock_Weapons() {
    printf( "CHEAT: Unlocked weapons\n" );
    weapons_enabled = true;
    for ( int i = 0; i < weaponStash -> Quantity(); i++ ) {
        Weapon* w = weaponStash -> GetWeaponByIndex( i );
        if ( w ) {
            w -> SetVar( WEAPON_VAR_LOCKED, 0 );
        }
    }
}

void CHEAT_Infinity_Ammo() {
    printf( "CHEAT: Infinity ammo\n" );
    for ( int i = 0; i < ammoStash -> Quantity(); i++ ) {
        Ammo* a = ammoStash -> GetAmmoByIndex( i );
        if ( a ) {
            a -> SetInfinity();
        }
    }
}

extern bool forceFPS;

void CHEAT_Force_FPS() {
    forceFPS = !forceFPS;
}

void CheatInit() {
    cheatSystem = new CheatSystem( 3.0 );
    cheatSystem -> AddDetection( "stashed", CHEAT_Unlock_Weapons );
    cheatSystem -> AddDetection( "fracture", CHEAT_Infinity_Ammo );
    cheatSystem -> AddDetection( "fps", CHEAT_Force_FPS );
}
