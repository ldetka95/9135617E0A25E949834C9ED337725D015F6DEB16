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
#include "Camera.h"

double spawnDist = FOG_END_DIST + 2.0;

void RecalcSpawnDist() {
    spawnDist = FOG_END_DIST + 2.0;
}

#define LOGIC_DOG 1
#define LOGIC_HORSE 2
#define LOGIC_FISH 3

#define LOGIC_GUARDIAN 4
#define LOGIC_SOLDIER 5
#define LOGIC_ARCHANGEL 6

#define LOGIC_KAYLEY 24

#define MAX_ALLOWED_MONSTER_DIST 48.0

int trySpawnTimes = 32; // not too much, it can get laggy

double SpawnAutoAngleStepMin = PI / 15.3333;
double SpawnAutoAngleStepMax = PI / 7.3333;

bool NothingBetween( Point3D p1, Point3D p2 ) {
    Point3D v = SubtractPoint( p2, p1 );
    double dist = Dist3D( p1, p2 );
    if ( dist > 0.0 ) {
        ScalePoint3D( &v, 1.0 / dist ); // scale to vector of length 1.0
        for ( int i = 0; i < dist; i++ ) {
            if ( MainMap -> GetID( p1.x, p1.y, p1.z ) > 0 ) {
                return false; // sth blocks
            }
            p1 = AddPoint( p1, v );
        }
        return true; // way free
    } else { // dist is 0.0, nothing on the way though
        return true;
    }
}

bool NothingBetween( Model* m, Point3D base, Point3D p2 ) {
    if ( !m ) {
        return true; // act as a ghost
    }
    for ( int i = 0; i < m -> Length(); i++ ) {
        ModelBlock block = m -> Get( i );
        Point3D p1 = AddPoint( block.offset, base );
        Point3D v = SubtractPoint( p2, p1 );
        double dist = Dist3D( p1, p2 );
        if ( dist > 0.0 ) {
            ScalePoint3D( &v, 1.0 / dist );
            for ( int i = 0; i < dist; i++ ) {
                if ( MainMap -> GetID( p1.x, p1.y, p1.z ) > 0 ) {
                    return false;
                }
                p1 = AddPoint( p1, v );
            }
        }
    }
    return true;
}

bool FirstSolidBetween( Point3D p1, Point3D p2, Point3D* ret ) {
    Point3D v = SubtractPoint( p2, p1 );
    double dist = ceil( Dist3D( p1, p2 ) );
    if ( ( dist > 0.0 ) && ( ret ) ) {
        ScalePoint3D( &v, 1.0 / dist );
        for ( int i = 0; i <= dist; i++ ) {
            if ( MainMap -> GetID( p1.x, p1.y, p1.z ) > 0 ) {
                ( *ret ) = p1; // blocking place on main map
                return true; // sth blocks
            }
            p1 = AddPoint( p1, v );
        }
    }
    return false; // nothing blocks
}

bool toRemove[ MAX_MONSTERS ];

void CheckRemoveMonsters() {
    for ( int i = 0; i < MAX_MONSTERS; i++ ) {
        if ( toRemove[ i ] ) {
            if ( monsterStorage[ i ] ) {
                delete monsterStorage[ i ];
                monsterStorage[ i ] = NULL;
            }
            toRemove[ i ] = false;
        }
    }
}

bool SameIntegerPoint( Point3D p1, Point3D p2, Point3D mask ) {
    if ( mask.x != 0.0 ) {
        if ( floor( p1.x ) != floor( p2.x ) ) {
            return false;
        }
    }
    if ( mask.y != 0.0 ) {
        if ( floor( p1.y ) != floor( p2.y ) ) {
            return false;
        }
    }
    if ( mask.z != 0.0 ) {
        if ( floor( p1.z ) != floor( p2.z ) ) {
            return false;
        }
    }
    return true;
}

/*
    Basic Living Logic
        Check collisions, affect gravity if should, and try to move, if needed.
        Damages player on radius, when too close.
        Every Logic that does not belong to Kayley automatically dissapears when player is too far.
*/

#define PLAYER_REACTION_TIME 0.11

void Living_Logic( int i, Point3D* velocity, double playerDistance, bool canJump, bool gravityAffects = true, bool moved = true ) {
    for ( int j = 0; j < MAX_MONSTERS; j++ ) {
        if ( ( i != j ) && ( monsterStorage[ j ] ) ) {
            monsterStorage[ i ] -> CollideEntity( monsterStorage[ j ], velocity, 0.3 );
        }
    }
    if ( gravityAffects ) {
        monsterStorage[ i ] -> GravityAffect( MainMap, STD_GRAVITY_POWER, STD_GRAVITY_INERTIA, velocity );
    }
    Point3D oldPos = monsterStorage[ i ] -> HotSpot();
    if ( moved ) {
        monsterStorage[ i ] -> MoveAtMap( MainMap, ( *velocity ) );
    }
    Point3D newPos = monsterStorage[ i ] -> HotSpot();
    double velDist = Dist3D( oldPos, newPos );
    if ( ( velDist < ( monsterStorage[ i ] -> GetVar( VAR_SPEED ) * 0.5 ) ) && ( canJump ) ) { // entity is probably blocked by something
        // check if it makes sence to jump now
        Point3D facing = monsterStorage[ i ] -> Facing();
        Point3D solid = facing;
        facing.y = oldPos.y;
        if ( FirstSolidBetween( oldPos, facing, &solid ) || ( SameIntegerPoint( oldPos, facing, Point( 1.0, 0.0, 1.0 ) ) ) ) {
            solid.y += 1.0;
            if ( !( MainMap -> GetID( solid.x, solid.y, solid.z ) ) ) { // it makes sence
                // Try to Jump Yourself, as Komprex said.
                monsterStorage[ i ] -> TryJump( MainMap, 0.18 ); // JUMP! *electric guitar*
            }
        }
    }
    /*
    int seen = monsterStorage[ i ] -> GetVar( VAR_ENTITY_SEEN );
    double alert_range = ENTITY_ALERT_RANGE_DEFAULT + monsterStorage[ i ] -> GetVar( VAR_ENTITY_ALERT_RANGE );
    double ei, ea;
    Point3D p = player -> FixedPlayerSpot();
    Point3D e = monsterStorage[ i ] -> HotSpot();
    cts( e.x - p.x, e.y - p.y, e.z - p.z, NULL, &ei, &ea );
    if ( ( absf( ei - mainCamHandle -> pitchAngle ) > ( PI / 2.0 ) ) || ( absf( ea - mainCamHandle -> yawAngle ) > ( PI / 4.0 ) ) ) {
        seen = 1;
    }
    if ( !seen ) {
        if ( playerDistance < alert_range ) {
            mainCamHandle -> DriveTo( Point( ei, ea, 0.0 ), PLAYER_REACTION_TIME );
        }
    }
    monsterStorage[ i ] -> SetVar( VAR_ENTITY_SEEN, seen );
    */
    monsterStorage[ i ] -> SetVar( VAR_ENTITY_MOVE_SPEED_XZ, Dist3D( Point( oldPos.x, newPos.y, oldPos.z ), newPos ) );
    if ( playerDistance <= monsterStorage[ i ] -> GetVar( VAR_COLLISION_RADIUS ) ) { // damage player and die
        DamagePlayer( Random( monsterStorage[ i ] -> GetVar( VAR_DMG_MIN ), monsterStorage[ i ] -> GetVar( VAR_DMG_MAX ) ) );
        toRemove[ i ] = true;
        return;
    }
    int logic = monsterStorage[ i ] -> GetVar( VAR_LOGIC );
    if ( ( playerDistance > MAX_ALLOWED_MONSTER_DIST ) && ( logic != LOGIC_KAYLEY ) ) { // Kayley can NOT be removed!
        toRemove[ i ] = true;
        return;
    }
}



/*
    Fixed monster position
*/

Point3D Monster_ReCalcSpot( int i ) {
    Point3D mobSpot = monsterStorage[ i ] -> HotSpot();
    Object* mother = monsterStorage[ i ] -> GetMother();
    if ( mother ) {
        Model* model = mother -> GetModel();
        if ( model ) {
            mobSpot.y += model -> GetLowestY();
        }
    }
    return mobSpot;
}

/*
    Dog Logic
        Basic logic.
        Goes straight to the player, and cannot avoid the wall.
        Jumps if meets blockade.
        Affected by gravity.
*/

void Dog_Logic( int i ) {
    monsterStorage[ i ] -> FaceTo( player -> FixedPlayerSpot() ); // where do I look?
    Point3D monsterPos = Monster_ReCalcSpot( i );
    Point3D velocity = SubtractPoint( player -> HotSpot(), monsterPos );
    velocity.y = 0.0;
    Point3D playerPos = player -> HotSpot();
    double dist = Dist3D( playerPos, Point( monsterPos.x, playerPos.y, monsterPos.z ) );
    double dist3d = Dist3D( playerPos, monsterPos );
    if ( dist > 0.0 ) {
        velocity.x *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
        velocity.z *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
    }
    Living_Logic( i, &velocity, dist3d, true );
}

/*
    KAYLEY SPECIALS
*/

#define KAYLEY_ATTACK 0
#define KAYLEY_RUNAWAY 1
#define KAYLEY_WAIT 2

#define KAYLEY_DISTSTATE_OUTSIDE 0
#define KAYLEY_DISTSTATE_INSIDE 1

//Timer* kayley_Interval = NULL;
#define KAYLEY_TIMER_CHANGE_STATE 0

/*
    Basic Follower Logic
        Follows player only.
        Uses raw Kayley Logic on KAYLEY_ATTACK state.
        Does not implement gravity affection.
*/

void Follower_Logic( int i, Entity* followed, int* distState, double dist, bool* moved, Point3D* velocity, Point3D hspot_M ) {
    // For now, it uses Kayley special Vars, which is not a bug, because this logic is used by Kayley too.
    // Followed Entity should be Player. Otherwise, it will cause minor bugs.
    monsterStorage[ i ] -> FaceTo( followed -> FixedPlayerSpot() );
    double dist_A = monsterStorage[ i ] -> GetVar( VAR_KAYLEY_DISTMIN );
    double dist_B = monsterStorage[ i ] -> GetVar( VAR_KAYLEY_DISTMAX );
    if ( ( ( *distState ) == KAYLEY_DISTSTATE_OUTSIDE ) && ( dist > 0.0 ) ) {
        ( *moved ) = true;
        velocity -> x *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
        velocity -> z *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
        double nDist = Dist3D( followed -> HotSpot(), AddPoint( hspot_M, ( *velocity ) ) );
        if ( nDist <= dist_A ) {
            ( *distState ) = KAYLEY_DISTSTATE_INSIDE;
        }
    } else if ( ( dist > dist_B ) && ( ( *distState ) == KAYLEY_DISTSTATE_INSIDE ) && ( dist > 0.0 ) ) {
        ( *distState ) = KAYLEY_DISTSTATE_OUTSIDE;
    }
}

/*
    Kayley Logic
        Special logic.
        Goes straight to the player, but can't damage him as usual.
        While in KAYLEY_ATTACK state, screams and follows the player.
        While in KAYLEY_RUNAWAY state, runs away from player straight to the fog; changes state when fog reached.
        While in KAYLEY_WAIT state, does nothing - just a debug state.
        Changes its state overall on timer ( between KAYLEY_ATTACK and KAYLEY_RUNAWAY ).
        Does not jump at all, so the final battle map should be super flat.
        Affected by gravity.
*/

#define KAYLEY_ANXIETY_ADD 0.08

#define STD_ANXIETY_DECAY_LEVEL 0.01667 // 1.0 every second
#define STD_ANXIETY_MAX_LEVEL 40.0

double ___AnxietyExternLevel = 0.0;

void Kayley_Logic( int i ) {
    Point3D hspot_M = monsterStorage[ i ] -> HotSpot();
    double lYP = monsterStorage[ i ] -> GetLowestYPoint();
    hspot_M.y += lYP;
    Point3D velocity = SubtractPoint( player -> HotSpot(), hspot_M );
    velocity.y = 0.0;
    bool moved = false;
    double dist = Dist3D( player -> HotSpot(), hspot_M );
    for ( int j = 0; j < MAX_MONSTERS; j++ ) {
        if ( ( i != j ) && ( monsterStorage[ j ] ) ) {
            monsterStorage[ i ] -> CollideEntity( monsterStorage[ j ], &velocity, 0.3 );
        }
    }
    // main Kayley logic
    int distState = monsterStorage[ i ] -> GetVar( VAR_ENTITY_KAYLEY_DISTSTATE );
    int state = monsterStorage[ i ] -> GetVar( VAR_ENTITY_KAYLEY_STATE );
    int oldState = state;
    double timeMin = monsterStorage[ i ] -> GetVar( VAR_KAYLEY_STATECHANGETIMEMIN );
    double timeMax = monsterStorage[ i ] -> GetVar( VAR_KAYLEY_STATECHANGETIMEMAX );
    bool created = false;
    Timer* kayley_Interval = monsterStorage[ i ] -> Apply_Timer( KAYLEY_TIMER_CHANGE_STATE, timeMin * 1000.0, timeMax * 1000.0, &created );
    if ( created ) {
        oldState = 1 - state;
    }
    if ( kayley_Interval -> Tick() ) {
        state = 1 - state;
    }
    switch ( state ) {
        case KAYLEY_ATTACK: {
            Follower_Logic( i, player, &distState, dist, &moved, &velocity, hspot_M );
            if ( dist < 1.0 ) {
                ___AnxietyExternLevel += KAYLEY_ANXIETY_ADD + STD_ANXIETY_DECAY_LEVEL;//mainCamHandle -> AddAnxiety( KAYLEY_ANXIETY_ADD );
            }
            break;
        }
        case KAYLEY_RUNAWAY: {
            Point3D hspot_N = monsterStorage[ i ] -> HotSpot();
            hspot_N.y -= lYP;
            Point3D escapeLookPoint = AddPoint( SubtractPoint( hspot_N, player -> HotSpot() ), hspot_N );
            monsterStorage[ i ] -> FaceTo( escapeLookPoint );
            if ( dist < FOG_END_DIST + 1.0 ) {
                moved = true;
                velocity.x *= -monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
                velocity.z *= -monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
            } else {
                // random position change
                int chTries = 32; // not too much
                double baseAngle = Random( 2.0 * PI ); // all around
                double seekDirMultiplier = ( double )( ( ( rand() & 0x01 ) << 1 ) - 1 ); // { -1, 1 }
                double currentStep = seekDirMultiplier * Random( SpawnAutoAngleStepMin, SpawnAutoAngleStepMax );
                for ( int j = 0; j < chTries; j++ ) {
                    Point3D triedPlace = { 0.0, 1.0, 0.0 };
                    ptc( spawnDist, baseAngle + double( j ) * currentStep, &triedPlace.x, &triedPlace.z );
                    triedPlace = AddPoint( player -> HotSpot(), triedPlace );
                    if ( NothingBetween( player -> HotSpot(), triedPlace ) ) {
                        monsterStorage[ i ] -> UpdateHotSpot( triedPlace );
                        break;
                    }
                }
                state = KAYLEY_ATTACK;
            }
            break;
        }
        case KAYLEY_WAIT: {
            break;
        }
    }
    monsterStorage[ i ] -> SetVar( VAR_ENTITY_KAYLEY_DISTSTATE, distState );
    monsterStorage[ i ] -> SetVar( VAR_ENTITY_KAYLEY_STATE, state );
    Living_Logic( i, &velocity, dist, false, true, moved );
    if ( state != oldState ) {
        switch ( state ) {
            case KAYLEY_ATTACK: {
                StopSound( CHANNEL_KAYLEY );
                LoopCustomSound( CHANNEL_KAYLEY, SOUND_KAYLEY_ATTACK );
                break;
            }
            case KAYLEY_RUNAWAY: {
                StopSound( CHANNEL_KAYLEY );
                LoopCustomSound( CHANNEL_KAYLEY, SOUND_KAYLEY_BREATH );
                break;
            }
        }
    }
    SetVolumeDist( CHANNEL_KAYLEY, dist, 0.25 );
}

/*
    Horse Logic
        Custom logic.
        While in HORSE_LISTEN state, just stands and waits for one of following conditions to become true:
             - player shot within first hearing radius,
             - player ran within second hearing radius,
             - player moved within collision ( third hearing ) radius,
             - player damaged horse.
        If at least one of these conditions is true, changes state to HORSE_ATTACK.
        While in HORSE_ATTACK state, uses Dog Logic, and cannot switch back to HORSE_LISTEN state.
        Affected by gravity.
*/

#define HORSE_LISTEN 0
#define HORSE_ATTACK 1

Point3D lastShootOccuredPos_NOSHOOT = { INFINITY, INFINITY, INFINITY };
Point3D lastShootOccuredPos = lastShootOccuredPos_NOSHOOT;
bool player_Running = false;

void Horse_Logic( int i ) {
    int state = monsterStorage[ i ] -> GetVar( VAR_ENTITY_HORSE_STATE );
    switch ( state ) {
        case HORSE_LISTEN: {
            Point3D monsterPos = Monster_ReCalcSpot( i );
            double lastShoot_Dist = Dist3D( lastShootOccuredPos, monsterPos );
            double allowedShoot_Dist = monsterStorage[ i ] -> GetVar( VAR_HEAR_DIST );
            double player_Dist = Dist3D( player -> HotSpot(), monsterPos );
            double allowed_Dist = 1.75;
            if ( player_Running ) {
                allowed_Dist = monsterStorage[ i ] -> GetVar( VAR_HEAR_RUN_DIST );
            }
            int damaged_State = monsterStorage[ i ] -> GetVar( VAR_ENTITY_DAMAGED );
            if ( ( lastShoot_Dist < allowedShoot_Dist ) || ( damaged_State == ENTITY_DAMAGED ) || ( player_Dist < allowed_Dist ) ) {
                // heared a thing
                PlayCustomSound( CHANNEL_CUSTOM, SOUND_HORSE_ATTACK );
                state = HORSE_ATTACK;
            }
            Point3D velocity = Point( 0.0, 0.0, 0.0 );
            Living_Logic( i, &velocity, player_Dist, false );
            monsterStorage[ i ] -> SetVar( VAR_ENTITY_HORSE_STATE, state );
            break;
        }
        case HORSE_ATTACK: {
            Dog_Logic( i );
            break;
        }
    }
}

/*
    Fish Logic
        Custom logic.
        Basically acts exactly as Dog Logic, but it isn't affected by gravity.
*/

void Fish_Logic( int i ) {
    monsterStorage[ i ] -> FaceTo( player -> FixedPlayerSpot() ); // where do I look?
    Point3D monsterPos = Monster_ReCalcSpot( i );
    Point3D playerPos = player -> FixedPlayerSpot();
    Point3D velocity = SubtractPoint( playerPos, monsterPos );
    double dist = Dist3D( playerPos, monsterPos );
    if ( dist > 0.0 ) {
        velocity.x *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
        velocity.y *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
        velocity.z *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / dist;
    }
    Living_Logic( i, &velocity, dist, false, false );
}

/*
    Guardian Logic
        Custom logic.
        While in GUARDIAN_RELAX state, moves around the point within move radius.
        While in GUARDIAN_ALERT state, it follows and observes the player until get scared or relax.
        While in GUARDIAN_ATTACK state, uses Dog Logic, and cannot switch back to any state.
        Changes state to GUARDIAN_ALERT if state was GUARDIAN_RELAX and player moved inside the move radius.
        Changes state to GUARDIAN_ATTACK if player shot and state was GUARDIAN_ALERT or player was close enough,
            or if entity is damaged by player.
        Changes state to GUARDIAN_RELAX if state was GUARDIAN_ALERT and player moved outside the move radius.
        Affected by gravity.
*/

#define GUARDIAN_RELAX 0
#define GUARDIAN_ALERT 1
#define GUARDIAN_ATTACK 2

#define GUARDIAN_TIMER_WALK 0

void Guardian_Logic( int i ) {
    int state = monsterStorage[ i ] -> GetVar( VAR_ENTITY_HORSE_STATE );
    Point3D monsterPos = Monster_ReCalcSpot( i );//monsterStorage[ i ] -> HotSpot();// Monster_ReCalcSpot( i ); //
    double playerDist = Dist3D( player -> HotSpot(), monsterPos );
    Point3D spawnPos = monsterStorage[ i ] -> SpawnedAt();
    double playerDist_FromEntitySpawn = Dist3D( player -> HotSpot(), spawnPos );
    //double collisionRadius = monsterStorage[ i ] -> GetVar( VAR_COLLISION_RADIUS );
    //printf( "Player dist = %lf\n", playerDist );
    //printf( "Collision radius = %lf\n", collisionRadius );
    if ( ( monsterStorage[ i ] -> GetVar( VAR_ENTITY_DAMAGED ) == ENTITY_DAMAGED ) || ( Dist3D( monsterPos, lastShootOccuredPos ) < monsterStorage[ i ] -> GetVar( VAR_HEAR_DIST ) ) ) {
        // if damaged or heared shoot, attacks immediately.
        state = GUARDIAN_ATTACK;
    }
    double walk_Radius = monsterStorage[ i ] -> GetVar( VAR_WALKER_ALLOWEDRADIUS );
    switch ( state ) {
        case GUARDIAN_RELAX: {
            // walks around the point and waits for player walking inside move radius.
            Point3D facing = monsterStorage[ i ] -> Facing();
            double facingDist = Dist3D( facing, monsterPos );
            Point3D velocity = Point( 0.0, 0.0, 0.0 );
            if ( facingDist > 1.0 ) { // move to the point
                velocity = SubtractPoint( facing, monsterPos );
                velocity.x *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / facingDist;
                velocity.y = 0.0;
                velocity.z *= monsterStorage[ i ] -> GetVar( VAR_SPEED ) / facingDist;
            }
            double maxTimerRand = monsterStorage[ i ] -> GetVar( VAR_WALKER_ANXIETY );
            Timer* walk_Timer = monsterStorage[ i ] -> Apply_Timer( GUARDIAN_TIMER_WALK, 0.0, maxTimerRand * 1000.0 );
            if ( walk_Timer -> Tick() )  { // ticks
                double Radius = Random( 0.0, walk_Radius );
                double Angle = Random( 0.0, 2 * PI );
                double x, z;
                ptc( Radius, Angle, &x, &z );
                monsterStorage[ i ] -> FaceTo( AddPoint( Point( x, 0.0, z ), spawnPos ) );
                monsterStorage[ i ] -> Remove_Timer( GUARDIAN_TIMER_WALK );
            }
            if ( playerDist_FromEntitySpawn <= walk_Radius ) {
                state = GUARDIAN_ALERT;
            }
            Living_Logic( i, &velocity, playerDist, false );
            break;
        }
        case GUARDIAN_ALERT: {
            if ( playerDist_FromEntitySpawn > walk_Radius ) {
                state = GUARDIAN_RELAX;
            }
            int distState = monsterStorage[ i ] -> GetVar( VAR_ENTITY_KAYLEY_DISTSTATE );
            bool moved = false;
            Point3D velocity = SubtractPoint( player -> HotSpot(), monsterPos );
            Follower_Logic( i, player, &distState, playerDist, &moved, &velocity, monsterPos );
            monsterStorage[ i ] -> SetVar( VAR_ENTITY_KAYLEY_DISTSTATE, distState );
            Living_Logic( i, &velocity, playerDist, true, moved );
            break;
        }
        case GUARDIAN_ATTACK: {
            Dog_Logic( i );
            break;
        }
    }
    monsterStorage[ i ] -> SetVar( VAR_ENTITY_HORSE_STATE, state );
}


