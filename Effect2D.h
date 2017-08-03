#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include "Coordinates.h"
#include "Point3D.h"
#include "Textures.h"

Bitmap* ___effect_roar;

#define EFF_SIZE_X 320//640//512
#define EFF_SIZE_Y 240//480//512

void Effect_Init() {
    ___effect_roar = new Bitmap();
    ___effect_roar -> SetDim_Const( EFF_SIZE_X, EFF_SIZE_Y, 4 );
    void* ___roar_pixels = malloc( EFF_SIZE_X * EFF_SIZE_Y * 4 );
    ___effect_roar -> SetRaw( ___roar_pixels );
    for ( int iy = 0; iy < EFF_SIZE_Y; iy++ ) {
        for ( int ix = 0; ix < EFF_SIZE_X; ix++ ) {
            int adr = int( ___roar_pixels ) + ( ( iy * EFF_SIZE_X + ix ) << 2 );
            int gray = rand() & 0x1F;//( rand() % 0xFF );
            *( int* )( adr ) = 0xFF000000 + ( gray << 16 ) + ( gray << 8 ) + gray;
            *( int* )( adr ) -= ( ( rand() % 120 + 30 ) << 24 ); //& 0xFF
        }
    }
    ___effect_roar -> GL_Bitmap();
}


void Effect_Roar( double intensity ) { // intensity is expected to be from 0.0 to 1.0
    double asp = ( double )( EFF_SIZE_Y ) / ( double )( EFF_SIZE_X );
    double pixelSizeX = 1.0 / double( EFF_SIZE_X );
    double pixelSizeY = 1.0 / double( EFF_SIZE_Y );
    Point3D st = Point( Random( 0, EFF_SIZE_X >> 1 ) * pixelSizeX, Random( 0, EFF_SIZE_Y >> 1 ) * pixelSizeY, 0.0 );
    Point3D en = AddPoint( st, Point( 0.5, 0.5 / asp, 0.0 ) );
    ___effect_roar -> UseImage();
    glMatrixMode( GL_PROJECTION );
    glOrtho( 0, 1, 0, 1, 0.0, 100.0 );
    glMatrixMode( GL_MODELVIEW );
    //glDisable( GL_TEXTURE_2D );
    //glColor3f( 0.0, 0.5, 0.0 );
    glColor4f( 1.0, 1.0, 1.0, intensity );
    double a = 0;
    double b = 1.0;//EFF_SIZE_X;//0.5;
    glLoadIdentity();
    glBegin( GL_QUADS );
        glTexCoord2d( st.x, st.y );
        glVertex2f( a, a );
        glTexCoord2d( st.x, en.y );
        glVertex2f( a, b );
        glTexCoord2d( en.x, en.y );
        glVertex2f( b, b );
        glTexCoord2d( en.x, st.y );
        glVertex2f( b, a );
    glEnd();
    //glEnable( GL_TEXTURE_2D );
    /*int err = ( int )( glGetError() );
    if ( err ) {
        printf( "Error: %d\n", err );
    }*/
}

