#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include "Point3D.h"
#include "Coordinates.h"
#include "Map.h"

#define STD_BLOCK_POINT_NUMBER 8

Point3D ObtainAngles( Point3D p, Point3D c, double* radius ) {
    Point3D angles = { 0.0, 0.0, 0.0 };
    Point3D d = SubtractPoint( p, c );
    double sqdx = d.x * d.x;
    double sqdy = d.y * d.y;
    double sqdz = d.z * d.z;
    double sphereRadiusSQR = sqdx + sqdy + sqdz;
    ( *radius ) = sqrt( sphereRadiusSQR );
    if ( sphereRadiusSQR > 0.0 ) {
        double pitchAny = sqrt( sqdy + sqdz );
        double yawAny = sqrt( sqdx + sqdz );
        double rollAny = sqrt( sqdx + sqdy );
        angles.x = atan2( d.x, pitchAny ) * 180.0 / 3.1416; // pitch
        angles.y = atan2( d.y, yawAny ) * 180.0 / 3.1416; // yaw
        angles.z = atan2( d.z, rollAny ) * 180.0 / 3.1416; // roll
    }
    return angles;
}

Point3D DeclareAngles( Point3D a, Point3D c, double radius ) {
    Point3D cartPoint = { 0.0, 0.0, 0.0 };
    a.x *= 3.1416 / 180.0;
    a.y *= 3.1416 / 180.0;
    a.z *= 3.1416 / 180.0;
    cartPoint.y = sin( a.y );
    double ySA = 1.0 - absf( a.y );
    cartPoint.x = sin( a.x ) * ySA;
    cartPoint.z = sin( a.z ) * ySA;
    cartPoint.x = cartPoint.x * radius + c.x;
    cartPoint.y = cartPoint.y * radius + c.y;
    cartPoint.z = cartPoint.z * radius + c.z;
    return cartPoint;
}

// Point3D RotatePoint( Point3D rotated, Point3D rotationCenter, Point3D angle ) { // old declaration
Point3D RotatePoint( Point3D rotated, Point3D rotationCenter, double inclination, double azimuth ) {
    //double radius = 0.0;
    Point3D relativeCoordinates = SubtractPoint( rotated, rotationCenter );
    Point3D angles;
    cts( relativeCoordinates.x, relativeCoordinates.y, relativeCoordinates.z, &angles.x, &angles.y, &angles.z );
    //angles.x += angle.x; // x - radius - not changed!
    angles.y += inclination;
    angles.z += azimuth;
    Point3D rt;
    stc( angles.x, angles.y, angles.z, &rt.x, &rt.y, &rt.z );
    rt = AddPoint( rt, rotationCenter );
    return rt;
}

/*Point3D RotatePointQuaternion( Point3D point, Point3D rotationCenter, Point3D angles ) {
    Point3D rt = SubtractPoint( point, rotationCenter );
    return rt;
}*/

#define CURRENT_INCLINATION 0
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

#define MODELBLOCK_TOTAL_VAR 13

class Block;

struct ___ModelBlock {
    public:
    Block* block;
    Point3D offset, hotspot;
    double var[ MODELBLOCK_TOTAL_VAR ];
};

typedef struct ___ModelBlock ModelBlock;

class Block {
    public:
    Block( double sx, double sy, double sz ) {
        dimSize.x = sx;
        dimSize.y = sy;
        dimSize.z = sz;
        pointNum = STD_BLOCK_POINT_NUMBER;
        originalPoints = ( Point3D (*)[] )( malloc( STD_BLOCK_POINT_NUMBER * sizeof( Point3D ) ) );
        currentPoints = ( Point3D (*)[] )( calloc( STD_BLOCK_POINT_NUMBER, sizeof( Point3D ) ) );
        for ( int i = 0; i < STD_BLOCK_POINT_NUMBER; i++ ) {
            Point3D currentPoint;
            currentPoint.x = ( ( ( int )( ( i & 0x04 ) > 0 ) << 1 ) - 1 ) * sx / 2.0;
            currentPoint.y = ( ( ( int )( ( i & 0x02 ) > 0 ) << 1 ) - 1 ) * sy / 2.0; /// OUTDATED? // inverted hot spot, 100% correct!
            currentPoint.z = ( ( ( int )( ( i & 0x01 ) > 0 ) << 1 ) - 1 ) * sz / 2.0;
            ( *originalPoints )[ i ] = currentPoint;
        }
        RedefineHotSpot( 0.0, 0.0, 0.0 );
    }
    ~Block() {
        free( originalPoints );
        free( currentPoints );
    }
    void RedefineHotSpot( double x, double y, double z ) {
        hotspot.x = x;
        hotspot.y = y;
        hotspot.z = z;
    }
    void Rotate( Point3D* center, double inclination, double azimuth ) {
        Point3D rotCenter = hotspot;
        if ( center ) {
            rotCenter = ( *center );
        }
        Point3D diffPoint = SubtractPoint( hotspot, rotCenter );
        for ( int i = 0; i < STD_BLOCK_POINT_NUMBER; i++ ) {
            Point3D pickedPoint = SubtractPoint( ( *originalPoints )[ i ], diffPoint );
            ( *currentPoints )[ i ] = RotatePoint( pickedPoint, rotCenter, inclination, azimuth );
        }
    }
    double GetX( int index ) {
        return ( *originalPoints )[ index ].x;
    }
    double GetY( int index ) {
        return ( *originalPoints )[ index ].y;
    }
    double GetZ( int index ) {
        return ( *originalPoints )[ index ].z;
    }
    void RotateQuaternion( Point3D* center, double rX, double rY, double rZ ) {
        Point3D rotCenter = hotspot;
        if ( center ) {
            rotCenter = ( *center );
        }
        Point3D diffPoint = SubtractPoint( hotspot, rotCenter );
        for ( int i = 0; i < STD_BLOCK_POINT_NUMBER; i++ ) {
            Point3D pickedPoint = SubtractPoint( ( *originalPoints )[ i ], diffPoint );
            ( *currentPoints )[ i ] = pickedPoint;
        }
    }
    void SetDisp() {
        for ( int i = 0; i < STD_BLOCK_POINT_NUMBER; i++ ) {
            ( *currentPoints )[ i ] = ( *originalPoints )[ i ];
        }
    }
    /*void Model_Start( double rotatedX, double rotatedY, double rotatedZ ) {
        glPushMatrix();
        glLoadIdentity();
        glTranslated( hotspot.x, hotspot.y, hotspot.z );
        model_rx = rotatedX;
        model_ry = rotatedY;
        model_rz = rotatedZ;
        glColor3f( MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
    }
    void Model_End() {
        glPopMatrix();
    }*/
    void DrawRotated_JoinModel( ModelBlock parameters, Point3D rotation, Point3D lookAt, double entitySpeedMul ) {
        glPushMatrix();
        if ( parameters.var[ BOOL_MODEL_HEAD ] == 2 ) {
            glTranslated( 0, parameters.offset.y, 0 );
            double r_i, r_a;
            //Point3D l_r = lookAt;
            cts( lookAt.x, lookAt.y, lookAt.z, NULL, &r_i, &r_a );
            //glRotated( 90 - degr( r_a ), 0.0, 1.0, 0.0 );
            glRotated( rotation.y, 0.0, 1.0, 0.0 );
            glRotated( degr( r_i ) - 90, 1.0, 0.0, 0.0 );
            //glRotated( rotation.x, 0.0, 0.0, 1.0 );
            //glRotated( rotation.z, 0.0, 0.0, 1.0 );
            glTranslated( parameters.offset.x, 0, parameters.offset.z );
        } else {
            glRotated( rotation.y, 0.0, 1.0, 0.0 );
            if ( parameters.var[ BOOL_MODEL_HEAD ] ) {
                glRotated( rotation.x, 1.0, 0.0, 0.0 );
            }
            glRotated( rotation.z, 0.0, 0.0, 1.0 );
            glTranslated( parameters.offset.x, parameters.offset.y, parameters.offset.z );
        }
        glRotated( parameters.var[ STD_INCLINATION ], 0.0, 1.0, 0.0 );
        glRotated( parameters.var[ STD_AZIMUTH ], 1.0, 0.0, 0.0 );
        double T = Timer::Current() * ( 2 * PI );
        double tinc = T * parameters.var[ SPD_INCLINATION ];
        double tazi = T * parameters.var[ SPD_AZIMUTH ];
        double inc_rotation = parameters.var[ MAX_INCLINATION ] * sin( tinc ) * parameters.var[ DIR_INCLINATION ] * entitySpeedMul;
        double azi_rotation = parameters.var[ MAX_AZIMUTH ] * sin( tazi ) * parameters.var[ DIR_AZIMUTH ] * entitySpeedMul;
        //glRotated( parameters.var[ STD_INCLINATION ], 0.0, 1.0, 0.0 );
        //glRotated( parameters.var[ STD_AZIMUTH ], 1.0, 0.0, 0.0 );
        glRotated( inc_rotation, 0.0, 1.0, 0.0 );
        glRotated( azi_rotation, 1.0, 0.0, 0.0 );
        glBegin( GL_QUADS );

            // 1

            glNormal3f( 0.0, 0.0, 1.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 5 ), GetY( 5 ), GetZ( 5 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 7 ), GetY( 7 ), GetZ( 7 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 3 ), GetY( 3 ), GetZ( 3 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 1 ), GetY( 1 ), GetZ( 1 ) );

            // 2

            glNormal3f( -1.0, 0.0, 0.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 1 ), GetY( 1 ), GetZ( 1 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 3 ), GetY( 3 ), GetZ( 3 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 2 ), GetY( 2 ), GetZ( 2 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 0 ), GetY( 0 ), GetZ( 0 ) );

            // 3

            glNormal3f( 1.0, 0.0, 0.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 4 ), GetY( 4 ), GetZ( 4 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 6 ), GetY( 6 ), GetZ( 6 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 7 ), GetY( 7 ), GetZ( 7 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 5 ), GetY( 5 ), GetZ( 5 ) );

            // 4

            glNormal3f( 0.0, 0.0, -1.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 0 ), GetY( 0 ), GetZ( 0 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 2 ), GetY( 2 ), GetZ( 2 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 6 ), GetY( 6 ), GetZ( 6 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 4 ), GetY( 4 ), GetZ( 4 ) );

            // 5

            glNormal3f( 0.0, -1.0, 0.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 4 ), GetY( 4 ), GetZ( 4 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 5 ), GetY( 5 ), GetZ( 5 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 1 ), GetY( 1 ), GetZ( 1 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 0 ), GetY( 0 ), GetZ( 0 ) );

            // 6

            glNormal3f( 0.0, 1.0, 0.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 6 ), GetY( 6 ), GetZ( 6 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 7 ), GetY( 7 ), GetZ( 7 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 3 ), GetY( 3 ), GetZ( 3 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 2 ), GetY( 2 ), GetZ( 2 ) );

        glEnd();
        glPopMatrix();
    }
    void DrawRotated( Point3D* Cameracenter, Point3D playerPos, double rotatedX, double rotatedY, double rotatedZ ) {
        glPushMatrix();
        //glLoadIdentity();
        //glMatrixMode( GL_MODELVIEW );
        //glLoadIdentity();
        //gluLookAt( playerPos.x, playerPos.y, playerPos.z, playerLook.x, playerLook.y, playerLook.z, p.x, p.y, p.z );
        Point3D cc = { 0.0, 0.0, 0.0 };
        if ( Cameracenter ) {
            cc = ( *Cameracenter );
        }
        //glLoadIdentity();
        //printf( "Pos rotated: %lf %lf %lf\n", playerPos.x + hotspot.x, playerPos.y + hotspot.y, playerPos.z + hotspot.z );
        // order of rotation axis IS important!
        glTranslated( playerPos.x + hotspot.x, playerPos.y + hotspot.y, playerPos.z + hotspot.z ); // std translate
        glTranslated( cc.x, cc.y, cc.z );
        glRotated( rotatedY, 0.0, 1.0, 0.0 );
        glRotated( rotatedX, 1.0, 0.0, 0.0 );
        glRotated( rotatedZ, 0.0, 0.0, 1.0 );

        glColor3f( MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
        //glEnable( GL_TEXTURE_2D );
        SetDisp();
        glBegin( GL_QUADS );

            // 1

            glNormal3f( 0.0, 0.0, 1.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 5 ), GetY( 5 ), GetZ( 5 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 7 ), GetY( 7 ), GetZ( 7 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 3 ), GetY( 3 ), GetZ( 3 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 1 ), GetY( 1 ), GetZ( 1 ) );

        // glEnd();
        // glBegin( GL_QUADS );

            // 2

            glNormal3f( -1.0, 0.0, 0.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 1 ), GetY( 1 ), GetZ( 1 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 3 ), GetY( 3 ), GetZ( 3 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 2 ), GetY( 2 ), GetZ( 2 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 0 ), GetY( 0 ), GetZ( 0 ) );

        // glEnd();
        // glBegin( GL_QUADS );

            // 3

            glNormal3f( 1.0, 0.0, 0.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 4 ), GetY( 4 ), GetZ( 4 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 6 ), GetY( 6 ), GetZ( 6 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 7 ), GetY( 7 ), GetZ( 7 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 5 ), GetY( 5 ), GetZ( 5 ) );

        // glEnd();
        // glBegin( GL_QUADS );

            // 4

            glNormal3f( 0.0, 0.0, -1.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 0 ), GetY( 0 ), GetZ( 0 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 2 ), GetY( 2 ), GetZ( 2 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 6 ), GetY( 6 ), GetZ( 6 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 4 ), GetY( 4 ), GetZ( 4 ) );

        // glEnd();
        // glBegin( GL_QUADS );

            // 5

            glNormal3f( 0.0, -1.0, 0.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 4 ), GetY( 4 ), GetZ( 4 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 5 ), GetY( 5 ), GetZ( 5 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 1 ), GetY( 1 ), GetZ( 1 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 0 ), GetY( 0 ), GetZ( 0 ) );

        // glEnd();
        // glBegin( GL_QUADS );

            // 6

            glNormal3f( 0.0, 1.0, 0.0 );

            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( GetX( 6 ), GetY( 6 ), GetZ( 6 ) );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( GetX( 7 ), GetY( 7 ), GetZ( 7 ) );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( GetX( 3 ), GetY( 3 ), GetZ( 3 ) );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( GetX( 2 ), GetY( 2 ), GetZ( 2 ) );

        glEnd();
        //glDisable( GL_TEXTURE_2D );
        glPopMatrix();
    }
    double LowestY() {
        if ( pointNum <= 0 ) {
            return 0.0;
        }
        double ret = GetY( 0 );
        for ( int i = 1; i < pointNum; i++ ) {
            if ( GetY( i ) < ret ) {
                ret = GetY( i );
            }
        }
        return ret;
    }
    private:
    Point3D dimSize;
    Point3D hotspot;
    int pointNum;
    Point3D ( *originalPoints )[];
    Point3D ( *currentPoints )[];
    double model_rx, model_ry, model_rz;
};



