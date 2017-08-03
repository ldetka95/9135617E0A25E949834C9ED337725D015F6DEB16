#pragma once

#include <gl/gl.h>
#include <glut/glut.h>

#include "Stringlinker.h"
#include "Map.h"
#include "Blocks.h"
#include "TreeModel.h"

class ObjectModel {
    protected:
    ObjectModel() {
        ___joint = Point( 0.0, 0.0, 0.0 );
        ___size = Point( 0.0, 0.0, 0.0 );
        for ( int i = 0; i < MODELBLOCK_TOTAL_VAR; i++ ) {
            ___var[ i ] = 0.0;
        }
        //___tc_Init = Random( 2.0 * PI );
    }
    public:
    ~ObjectModel() {
        list< ObjectModel* >::iterator it = ___child.begin();
        while ( it != ___child.end() ) {
            delete ( *it );
            it++;
        }
    }
    static ObjectModel* fromFile( string path ) {
        FILE* handle = fopen( path.c_str(), "r" );
        if ( handle ) {
            return ___ParseModel( handle );
        }
        return NULL;
    }
    void display( Point3D position, Point3D faceDir ) {
        glPushMatrix();
        glTranslated( position.x, position.y, position.z );
        double a;
        Point3D dir = SubtractPoint( position, faceDir );
        ctp( dir.x, dir.z, NULL, &a );
        a = degr( a );
        glRotated( a, 0.0, 1.0, 0.0 );
        ___tc = Timer::Current() * PI;// + ___tc_Init;
        ___displayBlock( faceDir, Point( 0.0, a, 0.0 ), position );
        glPopMatrix();
    }
    double getVar( int varIndex ) {
        return ___var[ varIndex ];
    }
    void setVar( int varIndex, double value ) {
        ___var[ varIndex ] = value;
    }
    private:
    static ObjectModel* ___ParseModel( FILE* handle ) {
        ObjectModel* o = new ObjectModel();
        fscanf( handle, "%lf %lf %lf\n", &( o -> ___offset ).x, &( o -> ___offset ).y, &( o -> ___offset ).z );
        fscanf( handle, "%lf %lf %lf\n", &( o -> ___joint ).x, &( o -> ___joint ).y, &( o -> ___joint ).z );
        fscanf( handle, "%lf %lf %lf\n", &( o -> ___size ).x, &( o -> ___size ).y, &( o -> ___size ).z );
        fscanf( handle, "%lf %lf\n", &( o -> ___var[ TEXTURE_INDEX ] ), &( o -> ___var[ BOOL_MODEL_HEAD ] ) );
        fscanf( handle, "%lf %lf\n", &( o -> ___var[ STD_INCLINATION ] ), &( o -> ___var[ STD_AZIMUTH ] ) );
        fscanf( handle, "%lf %lf %lf\n", &( o -> ___var[ MAX_INCLINATION ] ), &( o -> ___var[ DIR_INCLINATION ] ), &( o -> ___var[ SPD_INCLINATION ] ) );
        fscanf( handle, "%lf %lf %lf\n", &( o -> ___var[ MAX_AZIMUTH ] ), &( o -> ___var[ DIR_AZIMUTH ] ), &( o -> ___var[ SPD_AZIMUTH ] ) );
        fscanf( handle, "%lf\n", &( o -> ___var[ COLLISION_RADIUS ] ) );
        int childCount = 0;
        fscanf( handle, "%d\n", &childCount );
        for ( int i = 0; i < childCount; i++ ) {
            o -> ___child.push_back( ___ParseModel( handle ) );
        }
        return o;
    }
    void ___displayBlock( Point3D faceDir, Point3D angles, Point3D offsum ) {
        glPushMatrix();
        int imgIndex = ( int )( ___var[ TEXTURE_INDEX ] );
        if ( image[ imgIndex ] ) {
            if ( !image[ imgIndex ] -> UseImage() ) {
                return;
            }
        } else {
            return;
        }
        glTranslated( ___offset.x, ___offset.y, ___offset.z );
        double incl = ___var[ STD_INCLINATION ] + sin( ___tc * ___var[ SPD_INCLINATION ] ) * ___var[ MAX_INCLINATION ] * ___var[ DIR_INCLINATION ];
        double azim = ___var[ STD_AZIMUTH ] + sin( ___tc * ___var[ SPD_AZIMUTH ] ) * ___var[ MAX_AZIMUTH ] * ___var[ DIR_AZIMUTH ];
        glRotated( incl, 0.0, 1.0, 0.0 );
        glRotated( azim, 1.0, 0.0, 0.0 );
        //angles.y += incl;
        //angles.x += azim;
        offsum = AddPoint( offsum, SubtractPoint( ___offset, ___joint ) );
        bool isHead = ( int )( ___var[ BOOL_MODEL_HEAD ] ) == 1;
        if ( isHead ) {
            glPushMatrix();
            //glRotated( -azim, 1.0, 0.0, 0.0 ); // angles.x
            //glRotated( -incl, 0.0, 1.0, 0.0 ); // angles.y
            // look angle
            Point3D dif = SubtractPoint( offsum, faceDir );
            double i;
            cts( dif.x, dif.y, dif.z, NULL, &i, NULL );
            //glRotated( - degr( a + DEGR90 ), 0.0, 1.0, 0.0 );
            glRotated( degr( i - DEGR90 ), 1.0, 0.0, 0.0 );
            // end of look angle
        }
        glTranslated( -___joint.x, -___joint.y, -___joint.z );
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
        list< ObjectModel* >::iterator it = ___child.begin();
        while ( it != ___child.end() ) {
            ( *it ) -> ___tc = ___tc;
            ( *it ) -> ___displayBlock( faceDir, angles, offsum );
            it++;
        }
        if ( isHead ) {
            glPopMatrix();
        }
        glPopMatrix();
    }
    double GetX( int a ) {
        int mul = ( int( ( a & 0x04 ) > 0 ) << 1 ) - 1;
        return ___size.x / 2.0 * mul;
    }
    double GetY( int a ) {
        int mul = ( int( ( a & 0x02 ) > 0 ) << 1 ) - 1;
        return ___size.y / 2.0 * mul;
    }
    double GetZ( int a ) {
        int mul = ( int( ( a & 0x01 ) > 0 ) << 1 ) - 1;
        return ___size.z / 2.0 * mul;
    }
    list< ObjectModel* > ___child;
    Point3D ___offset;
    Point3D ___joint;
    Point3D ___size;
    double ___var[ MODELBLOCK_TOTAL_VAR ];
    // helpers
    double ___tc;
};

ObjectModel* omTest = ObjectModel::fromFile( "data/models/objectmodel/test.dat" );

