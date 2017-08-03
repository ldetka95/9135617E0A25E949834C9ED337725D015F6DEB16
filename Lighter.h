#pragma once

#include <gl/gl.h>
#include <glut/glut.h>

#include "Point3D.h"

void FillVec4f( float v[ 4 ], Point3D p, int sFrom ) {
    v[ sFrom ] = p.x;
    v[ sFrom + 1 ] = p.y;
    v[ sFrom + 2 ] = p.z;
}

void TurnOver4f( float v[ 4 ] ) {
    for ( int i = 0; i < 3; i++ ) {
        v[ i ] = -v[ i ];
    }
}

class Lighter {
    public:
    Lighter( char lK ) {
        lKey = lK;
        glEnable( GL_LIGHT0 + lKey );
        FlashPrepare();
    }
    ~Lighter() {
    }
    void FlashPrepare( Point3D src, Point3D destRel, Point3D color, float radius ) {
        GLfloat vecUsed[ 4 ] = { GLfloat( src.x ), GLfloat( src.y ), GLfloat( src.z ), 1.0 };
        GLfloat vecUsedDst[ 4 ] = { GLfloat( destRel.x ), GLfloat( destRel.y ), GLfloat( destRel.z ), 1.0 };
        GLfloat vecColor[ 4 ] = { GLfloat( color.x ), GLfloat( color.y ), GLfloat( color.z ), 1.0 };
        //FillVec4f( vecUsed, color, 0 );
        //vecUsed[ 3 ] = 1.0;
        //glLightfv( GL_LIGHT0 + lKey, GL_DIFFUSE, vecUsed );
        /*FillVec4f( vecUsed, src, 0 );
        for ( int i = 0; i < 4; i++ ) {
            printf( "%lf ", vecUsed[ i ] );
        }
        printf( "\n" );*/
        //FillVec4f( vecUsed, Point( 0, 0, 0 ), 0 );
        //vecUsed[ 3 ] = 1.0;
        glLightfv( GL_LIGHT0 + lKey, GL_DIFFUSE, vecColor );
        glLightfv( GL_LIGHT0 + lKey, GL_POSITION, vecUsed );
        glLightfv( GL_LIGHT0 + lKey, GL_SPOT_DIRECTION, vecUsedDst );
        glLightf( GL_LIGHT0 + lKey, GL_SPOT_CUTOFF, radius );
        glLightf( GL_LIGHT0 + lKey, GL_SPOT_EXPONENT, 16.0 );
        // glLightf( GL_LIGHT0 + lKey, GL_CONSTANT_ATTENUATION, 0.0 );
        // glLightf( GL_LIGHT0 + lKey, GL_LINEAR_ATTENUATION, 0.075 );
        //glPushMatrix();
    }
    void CorrectDir_DEBUG( Point3D p ) {
        GLfloat vecUsedDst[ 4 ] = { GLfloat( p.x ), GLfloat( p.y ), GLfloat( p.z ), 1.0 };
        glLightfv( GL_LIGHT0 + lKey, GL_SPOT_DIRECTION, vecUsedDst );
    }
    void FlashPrepare() {
        GLfloat fv[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
        glLightfv( GL_LIGHT0 + lKey, GL_POSITION, fv );
        glLightfv( GL_LIGHT0 + lKey, GL_DIFFUSE, fv );
    }
    void FlashScene() {
        //glPopMatrix();
    }
    private:
    char lKey;
};

Lighter* playerLighter;

