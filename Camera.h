#pragma once

#include <gl/gl.h>
#include <glut/glut.h>

#include <math.h>
#include "Entity.h"
#include "Coordinates.h"

#define YAWBORDER ( 89.99 * PI / 180.0 )
#define STD_RECOIL_MAX_ANGLE 10.0 * PI / 180.0
#define STD_RECOIL_INERTIA 0.992

#define FOV_ANXIETY_MAX_LEVEL 10.0
double FOV_ANXIETY_LEVEL = 0.0;

class Camera {
    public:
    Camera( double ix, double iy, double iz, double pA, double yA ) {
        x = ix;
        y = iy;
        z = iz;
        pitchAngle = pA;
        yawAngle = yA;
        rollAngle = 0.0;
        recoilGiven = 0.0;
        drv = false;
        CalcLookAngle();
        anxiety = NULL;
        qOffset = Point( 0.0, 0.0, 0.0 );
    }

    Camera( Entity* mainE, double pA, double yA ) {
        linkCam = mainE;
        pitchAngle = pA;
        yawAngle = yA;
        rollAngle = 0.0;
        recoilGiven = 0.0;
        drv = false;
        CalcLookAngle();
        anxiety = NULL;
        qOffset = Point( 0.0, 0.0, 0.0 );
    }

    double pitchAngle;
    double yawAngle;
    double yawAngle_Recoiled;
    double rollAngle;
    double recoilGiven;

    void CalcLookAngle() {
        yawAngle = Slap( yawAngle, -YAWBORDER, YAWBORDER );
        double yAngleRecoil = Slap( yawAngle + recoilGiven, -YAWBORDER, YAWBORDER );
        yawAngle_Recoiled = yAngleRecoil;
        stc( 1.0, yAngleRecoil - DEGR90, pitchAngle + DEGR90, &lx, &ly, &lz );
        if ( drv ) {
            double T = Timer::Current() - tStart;
            if ( T > tDuration ) {
                drv = false;
            } else {
                double pTime = T / tDuration; // expected to be [ 0..1 ]
                double pTimeRev = 1.0 - pTime;
                pitchAngle = mnp.x * pTimeRev + drva.x * pTime;
                yawAngle = mnp.y * pTimeRev + drva.y * pTime;
                rollAngle = mnp.z * pTimeRev + drva.z * pTime;
            }
        }
    }

    void RespawnAngles( Map* m ) {
        if ( m ) {
            pitchAngle = m -> GetSpawnAngleXZ();
            yawAngle = m -> GetSpawnAngleY();
        }
    }

    Point3D GetActorView() {
        if ( linkCam ) {
            return linkCam -> HotSpot();
        }
        Point3D buffer;
        buffer.x = x;
        buffer.y = y;
        buffer.z = z;
        return buffer;
    }

    Point3D NearestBlockLookingAt( Map* map, double scanrange, Point3D* latestBlockFree ) {
        Point3D ret = { -1.0, -1.0, -1.0 };
        if ( map ) {
            CalcLookAngle();
            Point3D position = GetActorView();
            position.y += pHeight;
            Point3D velocity = { lx, ly, lz };
            float density = 0.05;
            ScalePoint3D( &velocity, density );
            Point3D lBF = position;
            for ( float i = 0.0f; i < float( scanrange ); i += density ) {
                lBF = position;
                position = AddPoint( position, velocity );
                if ( map -> GetID( position.x, position.y, position.z ) > 0 ) {
                    if ( latestBlockFree ) {
                        ( *latestBlockFree ) = lBF;
                    }
                    ret = position;
                    //break;
                    return ret;
                }
            }
        }
        if ( latestBlockFree ) {
            ( *latestBlockFree ) = ret;
        }
        return ret;
    }

    Point3D GetView() {
        Point3D buffer;
        buffer.x = lx;
        buffer.y = ly;
        buffer.z = lz;
        return buffer;
    }

    void SetView( Point3D buffer ) { /// UPDATE: even used? If needed, then should be checked // does NOT work due to stc & cts errors
        lx = buffer.x;
        ly = buffer.y;
        lz = buffer.z;
        cts( lx, ly, lz, NULL, &yawAngle, &pitchAngle );
        yawAngle += DEGR90;
        pitchAngle -= DEGR90;
    }

    void SetRollAngle( double a ) {
        rollAngle = a;
    }
    void AddRollAngle( double a ) {
        rollAngle += a;
    }

    void LookAtAngles( bool overlay = false ) {
        double yA = ( yawAngle_Recoiled ) * 180.0 / PI;
        double pA = ( pitchAngle ) * 180.0 / PI;
        double zA = ( rollAngle ) * 180.0 / PI;
        if ( !overlay ) {
            glLoadIdentity();
        }
        if ( anxiety ) {
            AnxietyEffect();
        }
        //Point3D qOffset = Point( 0.0, 0.0, 0.0 );
        //printf( "qOffset: (%g, %g, %g)\n", qOffset.x, qOffset.y, qOffset.z );
        glRotated( zA + qOffset.z, 0.0, 0.0, 1.0 );
        glRotated( yA + qOffset.x, -1.0, 0.0, 0.0 );
        glRotated( pA + qOffset.y, 0.0, 1.0, 0.0 );
        if ( linkCam ) {
            Point3D l = linkCam -> FixedPlayerSpot();
            glTranslated( -l.x, -l.y, -l.z );
        } else {
            glTranslated( -x, -y, -z );
        }
        CalcLookAngle();
    }

    Point3D GetAngles() {
        Point3D buffer;
        buffer.x = pitchAngle;
        buffer.y = yawAngle;
        buffer.z = rollAngle;
        return buffer;
    }

    void SetAngles( Point3D angles ) {
        pitchAngle = angles.x;
        yawAngle = angles.y;
        rollAngle = angles.z;
    }

    Point3D GetDispersedView( double maxDisp ) {
        Point3D buffer = Point( 0.0, 0.0, 0.0 );
        double randDisp = ( Random( maxDisp ) - maxDisp / 2.0 ) * PI / 180.0;
        double randSphereSurfaceAngle = Random( 2 * PI );
        double TyawAngle = yawAngle + recoilGiven + sin( randSphereSurfaceAngle ) * randDisp;
        double TpitchAngle = pitchAngle + cos( randSphereSurfaceAngle ) * randDisp;
        stc( 1.0, TyawAngle - DEGR90, TpitchAngle + DEGR90, &buffer.x, &buffer.y, &buffer.z );
        return buffer;
    }

    /*void DisperseViewAngles( double maxDisp ) {
        double randDisp = ( Random( maxDisp ) - maxDisp / 2.0 ) * PI / 180.0;
        double randSphereSurfaceAngle = Random( 2 * PI );
        yawAngle += sin( randSphereSurfaceAngle ) * randDisp;
        pitchAngle += cos( randSphereSurfaceAngle ) * randDisp;
        CalcLookAngle();
    }*/

    void Recoil( double maxRecoil ) {
        recoilGiven += maxRecoil * PI / 180.0;
        //CalcLookAngle();
    }

    void RecoilAdaption() {
        if ( recoilGiven > STD_RECOIL_MAX_ANGLE ) {
            recoilGiven = STD_RECOIL_MAX_ANGLE;
        }
        if ( recoilGiven > 0.0 ) {
            recoilGiven -= STD_RECOIL_MAX_ANGLE * ( 1.0 - STD_RECOIL_INERTIA );
        }
        if ( recoilGiven < 0.0 ) {
            recoilGiven = 0.0;
        }
        CalcLookAngle();
    }

    void LinkAnxietyPointer( double* p ) {
        anxiety = p;
    }

    void AnxietyEffect() {
        double T = Timer::Current();
        glRotated( sin( T * 0.28 ) * ( *anxiety ) * 0.24, 1.0, 0.0, 1.0 );
        glRotated( sin( T * 0.60 ) * ( *anxiety ) * 0.16, 0.0, 1.0, 1.0 );
        glRotated( sin( T * 0.72 ) * ( *anxiety ) * 0.50, 0.0, 0.0, 1.0 );
        FOV_ANXIETY_LEVEL = max( -FOV_ANXIETY_MAX_LEVEL, min( sin( T * 1.73 ) * ( *anxiety ) * 0.15, FOV_ANXIETY_MAX_LEVEL ) );
        /* if ( ( *anxiety ) < 0.0 ) {
            ( *anxiety ) = 0.0;
        } else if ( ( *anxiety ) > 0.0 ) {
            ( *anxiety ) -= STD_ANXIETY_DECAY_LEVEL;
        } */
        ( *anxiety ) = max( 0.0, min( ( *anxiety ) - STD_ANXIETY_DECAY_LEVEL, STD_ANXIETY_MAX_LEVEL ) );
    }

    void AddAnxiety( double anx ) { // make sure for the result
        ( *anxiety ) += anx + STD_ANXIETY_DECAY_LEVEL;
    }

    bool DriveTo( Point3D a, double t ) { // angles in radians!
        if ( !drv ) {
            tStart = Timer::Current();
            tDuration = t;
            drva = a;
            mnp = GetAngles();
            if ( absf( mnp.x - drva.x ) > PI ) {
                if ( mnp.x > drva.x ) {
                    drva.x += 2 * PI;
                } else {
                    drva.x -= 2 * PI;
                }
            }
            drv = true;
            return true;
        }
        return false;
    }

    double x, y, z;
    double lx, ly, lz;
    Entity* linkCam = NULL;
    double* anxiety;
    Point3D qOffset; // quake angles in degrees
    private:
    Point3D drva, mnp;
    bool drv;
    double tStart, tDuration;
};

Camera* mainCamHandle = NULL;


class CamQuaker {
    public:
    CamQuaker( Camera** cam ) {
        Link( cam );
        SetQuake( -1.0, -1.0 );
        ___quaking = false;
    }
    ~CamQuaker() {
    }
    int QuakeTick() {
        if ( !___c ) {
            return -1;
        }
        Camera* c = *___c;
        if ( !c ) {
            return -2;
        }
        double t = Timer::Current();
        if ( ___quaking ) {
            if ( t < ___endQuakeTime ) {
                // quake
                double p2 = ___power / 2.0;
                double ranx = Random( ___power ) - p2;
                double rany = Random( ___power ) - p2;
                double ranz = Random( ___power ) - p2;
                c -> qOffset = Point( ranx, rany, ranz );
                return 1;
            } else {
                c -> qOffset = Point( 0.0, 0.0, 0.0 );
                ___quaking = false;
            }
        }
        return 0;
    }
    void SetQuake( double length, double power ) {
        ___startQuakeTime = Timer::Current();
        ___endQuakeTime = ___startQuakeTime + length;
        ___power = power;
        ___quaking = true;
    }
    int ResetQuake() {
        if ( !___c ) {
            return -1;
        }
        Camera* c = *___c;
        if ( !c ) {
            return -2;
        }
        if ( ___quaking ) {
            c -> qOffset = Point( 0.0, 0.0, 0.0 );
            ___quaking = false;
            return 1;
        }
        return 0;
    }
    void Link( Camera** cam ) {
        ___c = cam;
    }
    private:
    Camera** ___c;
    double ___startQuakeTime;
    double ___endQuakeTime;
    double ___power;
    bool ___quaking;
};

CamQuaker* mainCamQuaker = NULL;

