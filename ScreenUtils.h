#pragma once

#include <gl/gl.h>
#include <glut/glut.h>

#include <stdio.h>
#include <stdlib.h>

#include <string>

using namespace std;

string str( double value ) {
    char buffer[ 128 ];
    sprintf( buffer, "%lf", value );
    string s_ret = "";
    s_ret.assign( buffer );
    return s_ret;
}

class ConsolePrinter {
    public:
    ConsolePrinter( int X, int Y, int wX, int wY ) {
        colOffset = ( float )( wX ) / ( float )( X ) / 573.0;//256.0;
        rowOffset = ( float )( wY ) / ( float )( Y ) / 430.0;//256.0;
        ir = 0.99;
        ig = 0.99;
        ib = 0.99;
        cFont = GLUT_BITMAP_HELVETICA_12;//GLUT_BITMAP_TIMES_ROMAN_10;
        //mul = 1.0;
    }
    ~ConsolePrinter() {
    }
    void stdPrint( int x, int y, string stext, double aspX = 1.0 ) {
        glLoadIdentity();
        glTranslatef( -0.875, 0.685, -1.0 );
        glPushAttrib( GL_LIGHTING_BIT );
        for ( int i = 0; i < 8; i++ ) {
            glDisable( GL_LIGHT0 + i );
        }
        const char* text = stext.c_str();
        glColor3f( ir, ig, ib );
        int slen = stext.length();
        for ( int i = 0; i < slen; i++ ) {
            glRasterPos2f( ( x + i ) * colOffset, -y * rowOffset );
            //glVertex2f( ( x + i ) * colOffset, -y * rowOffset );
            //printf( "%c", text[ i ] );
            glutBitmapCharacter( cFont, text[ i ] );
        }
        glPopAttrib();
        //printf( "\n" );
    }
    void stdPrint( int x, int y, char c, double aspX = 1.0 ) {
        char C[ 2 ];
        C[ 0 ] = c;
        C[ 1 ] = 0;
        string s = C;
        stdPrint( x, y, s, aspX );
    }
    void stdColor( float r, float g, float b ) {
        ir = r;
        ig = g;
        ib = b;
    }
    void stdFont( void* font ) {
        cFont = font;
    }
    //void SetOmul( float m ) {
        //mul = m;
    //}
    private:
    float colOffset, rowOffset;
    //float mul;
    float ir, ig, ib;
    void* cFont;
};

ConsolePrinter* stdCon = NULL;

void InitSTDConsole( int wX, int wY ) {
    stdCon = new ConsolePrinter( 128, 80, wX, wY );
}

