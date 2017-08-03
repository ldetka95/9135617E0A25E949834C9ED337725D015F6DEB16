#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#define BITMAP_HEADER_LENGTH 54

#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif

#include "ScreenUtils.h"
#include <time.h>

const int TEX_ALPHA_MASK = 0xFF000000;
const int TEX_R_MASK = 0x00FF0000;
const int TEX_G_MASK = 0x0000FF00;
const int TEX_B_MASK = 0x000000FF;

class Bitmap {
    public:
    Bitmap() {
        data = NULL;
        dataPos = 0;
        width = 0;
        height = 0;
        for ( int i = 0; i < BITMAP_HEADER_LENGTH; i++ ) {
            header[ i ] = 0;
        }
        imageSize = 0;
    }
    Bitmap( const char* imagepath ) {
        data = NULL;
        dataPos = 0;
        width = 0;
        height = 0;
        FILE* handle = fopen( imagepath, "rb" );
        if ( handle ) {
            bool correctBitmap = true;
            if ( fread( header, 1, BITMAP_HEADER_LENGTH, handle ) != BITMAP_HEADER_LENGTH ) {
                correctBitmap = false;
            }
            if ( header[ 0 ] != 'B' || header[ 1 ] != 'M' ) {
                correctBitmap = false;
            }
            if ( correctBitmap ) {
                dataPos = *( int* )( &header[ 0x0A ] );
                //imageSize = *( int* )( &header[ 0x22 ] );
                width = *( unsigned int* )( &header[ 0x12 ] );
                height = *( unsigned int* )( &header[ 0x16 ] );
                //printf( "w = %d, h = %d\n", width, height );
                imageSize = 0;
                if ( !imageSize ) {
                    imageSize = width * height;
                }
                if ( !dataPos ) {
                    dataPos = 54;
                }
                unsigned char* TempData = ( unsigned char* )( malloc( imageSize * 3 ) );
                data = ( unsigned char* )( malloc( imageSize * 4 ) );
                fread( TempData, 1, imageSize * 3, handle );
                for ( unsigned int i = 0; i < imageSize; i++ ) {
                    unsigned int* lRead = ( unsigned int* )( int( TempData ) + i * 3 );
                    unsigned int lColPixel = ( *lRead ) & 0x00FFFFFF;
                    lColPixel = TEX_ALPHA_MASK + ( ( lColPixel & TEX_R_MASK ) >> 16 ) + ( ( lColPixel & TEX_G_MASK ) ) + ( ( lColPixel & TEX_B_MASK ) << 16 ); // revert colors
                    if ( lColPixel == 0xFFFFFFFF ) {
                        lColPixel = 0x00000000;
                    }
                    // tmp invert
                    //lColPixel = ( lColPixel & 0x00FFFFFF ) + 0xFF000000;
                    //int index = int( data ) + i * 4;
                    *( unsigned int* )( int( data ) + i * 4 ) = lColPixel;
                    /*if ( index < 0 ) {
                        printf( "INDEX %08X, VALUE = %d\n", index, lColPixel );
                    }*/
                }
                free( TempData );
            } else {
                printf( "INCORRECT BITMAP: %s\n", imagepath );
            }
            fclose( handle );
        } else {
            printf( "CANNOT OPEN: %s\n", imagepath );
        }
    }
    ~Bitmap() {
        if ( data ) {
            free( data );
        }
    }
    GLuint GL_Bitmap() {
        //GLuint ret;
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
        //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
        //printf( "GL_Bitmap call for: %p\n", this );
        UseImage();
        return texture;
    }
    void CopyFrom_32( Bitmap* bmp, int x1, int y1, int x2, int y2, unsigned int w, unsigned int h ) {
        // copy raw data from bitmap bmp, from rectangle (x1, y1, w, h) to rectangle (x2, y2, w, h)
        // scaling or rotating is impossible
        if ( bmp ) {
            //int srcbpp = bmp -> Bpp();
            //int dstbpp = Bpp();
            int cw = min( min( w, bmp -> Width() ), width );
            int ch = min( min( h, bmp -> Height() ), height );
            for ( int y = 0; y < ch; y++ ) {
                int* srcstart = ( int* )( bmp -> RawData() ) + ( ( y1 + y ) * bmp -> Width() + x1 );// * srcbpp;
                int* dststart = ( int* )( data ) + ( ( y2 + y ) * width + x2 );// * dstbpp;
                for ( int x = 0; x < cw; x++ ) { // x < w * dstbpp; x += dstbpp
                    ( *( dststart + x ) ) = ( * ( srcstart + x ) );
                }
            }
        }
    }
    bool UseImage() {
        if ( data ) {
            //glPixelStoref(
            //glBindTexture( GL_TEXTURE_2D, texture );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
            /*int err = glGetError();
            if ( err > 0 ) {
                printf( "Error = %d\n", err );
                printf( "   width = %d\n", width );
                printf( "   height = %d\n", height );
            }*/
            return true;
        }
        return false;
    }
    void makeTransparent32( double f ) {
        unsigned int tv = ( 1.0 - f ) * 256;
        unsigned int tvmask = tv << 24;
        unsigned int* pixels = ( unsigned int* )( data );
        for ( unsigned int i = 0; i < imageSize; i++ ) {
            pixels[ i ] = ( pixels[ i ] & 0x00FFFFFF ) + ( tvmask & 0xFF000000 );
        }
    }
    void Debug_Info() {
        static int x = 0;
        printf( "At x %d: Width = %d, Height = %d\n", x, width, height );
        x++;
    }
    void* RawData() {
        return ( void* )( data );
    }
    void SetRaw( void* ndata ) {
        data = ( unsigned char* )( ndata );
    }
    void SetDim( int w, int h, int bpp ) {
        width = w;
        height = h;
        imageSize = w * h * bpp;
        data = ( unsigned char* )( realloc( data, imageSize ) );
        for ( unsigned int i = 0; i < imageSize; i++ ) {
            ( *( data + i ) ) = 0;
        }
    }
    void SetDim_Const( int w, int h, int bpp ) {
        width = w;
        height = h;
        imageSize = w * h * bpp;
    }
    unsigned int Width() {
        return width;
    }
    unsigned int Height() {
        return height;
    }
    unsigned int Bpp() {
        return imageSize / width / height;
    }
    void SaveBitmap( string dirAuto ) {
        char pDate[ 256 ];
        sprintf( pDate, "image-%ld.bmp", time( NULL ) );
        string path = dirAuto + pDate;
        FILE* handle = fopen( path.c_str(), "wb" );
        if ( handle ) {
            /// header prepare
                // main header
            *( unsigned short int* )( &header[ 0x00 ] ) = 0x4D42;
            *( unsigned int* )( &header[ 0x02 ] ) = imageSize + BITMAP_HEADER_LENGTH;
            *( unsigned short int* )( &header[ 0x0A ] ) = BITMAP_HEADER_LENGTH;
                // DIB - windows style, BITMAPINFOHEADER
            *( unsigned int* )( &header[ 14 ] ) = 40;
            *( unsigned int* )( &header[ 18 ] ) = width;
            *( unsigned int* )( &header[ 22 ] ) = height;
            *( unsigned int* )( &header[ 26 ] ) = 0x00180001;
            *( unsigned int* )( &header[ 34 ] ) = imageSize;
            *( unsigned int* )( &header[ 38 ] ) = 2835;
            *( unsigned int* )( &header[ 42 ] ) = 2835;
            /*for ( int i = 0; i < BITMAP_HEADER_LENGTH; i++ ) {
                printf( "%d: %02X    | %c\n", i, header[ i ], header[ i ] );
            }*/ /// K!
            /// saving
            fwrite( header, BITMAP_HEADER_LENGTH, 1, handle );
            fwrite( data, imageSize, 1, handle );
            fclose( handle );
        }
    }
    private:
    void ___HeaderShortDeploy( int pos, unsigned short int number ) {
        header[ pos ] = ( number >> 8 ) & 0x00FF;
        header[ pos + 1 ] = number & 0x00FF;
    }
    void ___HeaderIntDeploy( int pos, unsigned int number ) {
        header[ pos ] = ( number >> 24 ) & 0x000000FF;
        header[ pos + 1 ] = ( number >> 16 ) & 0x000000FF;
        header[ pos + 2 ] = ( number >> 8 ) & 0x000000FF;
        header[ pos + 3 ] = number & 0x000000FF;
    }
    unsigned char header[ BITMAP_HEADER_LENGTH ];
    unsigned int dataPos;
    unsigned int width, height;
    unsigned int imageSize;
    unsigned char* data;
    GLuint texture;
};

const int MAX_TEXTURES = 256;
Bitmap* image[ MAX_TEXTURES ];

void SquareWall( double x, double y, double z, double size ) {
    glBegin( GL_QUADS );
    glEnd();
}

void StartFastDrawing() {
    glPushMatrix();
    glMatrixMode( GL_MODELVIEW );
    //glTranslatef( x, y, z );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
    glEnable( GL_TEXTURE_2D );
    glBegin( GL_QUADS );
}

void EndFastDrawing() {
    glEnd();
    glDisable( GL_TEXTURE_2D );
    glPopMatrix();
}

void TranslateMap( double x, double y, double z ) {
    glTranslatef( x, y, z );
}

inline void Cube( double x, double y, double z, double Size, double TextureID ) { /// UNUSED SINCE DRAWING IS OPTIMIZED ENOUGH
    glPushMatrix();
    //glMatrixMode( GL_MODELVIEW );
    //glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    //glEnable( GL_COLOR_MATERIAL );
    glTranslatef( x, y, z ); // std translate
    //glColor3f( 1.0, 1.0, 1.0 );
    //glColor3f( MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
    double Wall = Size / 2.0;
    //glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
    //glEnable( GL_TEXTURE_2D );

        // 1

    glBegin( GL_QUADS );
        glTexCoord2d( 1.0, 1.0 );
        glVertex3f( Wall, Wall, Wall );
        glTexCoord2d( 0.0, 1.0 );
        glVertex3f( - Wall, Wall, Wall );
        glTexCoord2d( 0.0, 0.0 );
        glVertex3f( - Wall, - Wall, Wall );
        glTexCoord2d( 1.0, 0.0 );
        glVertex3f( Wall, - Wall, Wall );

        // 2

        glTexCoord2d( 1.0, 1.0 );
        glVertex3f( Wall, Wall, Wall );
        glTexCoord2d( 0.0, 1.0 );
        glVertex3f( Wall, Wall, - Wall );
        glTexCoord2d( 0.0, 0.0 );
        glVertex3f( Wall, - Wall, - Wall );
        glTexCoord2d( 1.0, 0.0 );
        glVertex3f( Wall, - Wall, Wall );

        // 3

        glTexCoord2d( 1.0, 1.0 );
        glVertex3f( Wall, Wall, Wall );
        glTexCoord2d( 0.0, 1.0 );
        glVertex3f( Wall, Wall, - Wall );
        glTexCoord2d( 0.0, 0.0 );
        glVertex3f( - Wall, Wall, - Wall );
        glTexCoord2d( 1.0, 0.0 );
        glVertex3f( - Wall, Wall, Wall );
    //glEnd();

        // 4


    //glBegin( GL_QUADS );
        glTexCoord2d( 0.0, 0.0 );
        glVertex3f( - Wall, - Wall, - Wall );
        glTexCoord2d( 1.0, 0.0 );
        glVertex3f( Wall, - Wall, - Wall );
        glTexCoord2d( 1.0, 1.0 );
        glVertex3f( Wall, Wall, - Wall );
        glTexCoord2d( 0.0, 1.0 );
        glVertex3f( - Wall, Wall, - Wall );

        // 5

        glTexCoord2d( 0.0, 0.0 );
        glVertex3f( - Wall, - Wall, - Wall );
        glTexCoord2d( 1.0, 0.0 );
        glVertex3f( - Wall, - Wall, Wall );
        glTexCoord2d( 1.0, 1.0 );
        glVertex3f( - Wall, Wall, Wall );
        glTexCoord2d( 0.0, 1.0 );
        glVertex3f( - Wall, Wall, - Wall );

        // 6

        glTexCoord2d( 0.0, 0.0 );
        glVertex3f( - Wall, - Wall, - Wall );
        glTexCoord2d( 1.0, 0.0 );
        glVertex3f( - Wall, - Wall, Wall );
        glTexCoord2d( 1.0, 1.0 );
        glVertex3f( Wall, - Wall, Wall );
        glTexCoord2d( 0.0, 1.0 );
        glVertex3f( Wall, - Wall, - Wall );
    glEnd();

    //glDisable( GL_TEXTURE_2D );
    glPopMatrix();
}

inline void SideCube( double x, double y, double z, double Size, double TextureID, char sideFlag ) { /// UNUSED SINCE DRAWING IS OPTIMIZED ENOUGH
    // well I won't use drawing per-cube anymore. It's even slower than PKP trains.
    if ( sideFlag == 0 ) {
        return;
    }
    glPushMatrix();
    glTranslatef( x, y, z ); // std translate
    double Wall = Size / 2.0;
    glBegin( GL_QUADS );
        if ( sideFlag & 0x20 ) { // 1
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( Wall, Wall, Wall );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( - Wall, Wall, Wall );
            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( - Wall, - Wall, Wall );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( Wall, - Wall, Wall );
        }

        if ( sideFlag & 0x02 ) { // 2
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( Wall, Wall, Wall );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( Wall, Wall, - Wall );
            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( Wall, - Wall, - Wall );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( Wall, - Wall, Wall );
        }

        if ( sideFlag & 0x08 ) { // 3
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( Wall, Wall, Wall );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( Wall, Wall, - Wall );
            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( - Wall, Wall, - Wall );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( - Wall, Wall, Wall );
        }

        if ( sideFlag & 0x10 ) { // 4
            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( - Wall, - Wall, - Wall );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( Wall, - Wall, - Wall );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( Wall, Wall, - Wall );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( - Wall, Wall, - Wall );
        }

        if ( sideFlag & 0x01 ) { // 5
            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( - Wall, - Wall, - Wall );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( - Wall, - Wall, Wall );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( - Wall, Wall, Wall );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( - Wall, Wall, - Wall );
        }

        if ( sideFlag & 0x04 ) { // 6
            glTexCoord2d( 0.0, 0.0 );
            glVertex3f( - Wall, - Wall, - Wall );
            glTexCoord2d( 1.0, 0.0 );
            glVertex3f( - Wall, - Wall, Wall );
            glTexCoord2d( 1.0, 1.0 );
            glVertex3f( Wall, - Wall, Wall );
            glTexCoord2d( 0.0, 1.0 );
            glVertex3f( Wall, - Wall, - Wall );
        }
    glEnd();
    glPopMatrix();
}

void Sky( double x, double y, double z, double SkyH, double Horiz, double TextureID ) {
    glPushMatrix();
    //glMatrixMode( GL_MODELVIEW );
    // glTranslatef( x, y, z ); // std translate
    //glColor3f( 1.0, 1.0, 1.0 );
    //glColor3f( MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
    double Wall = Horiz / 2.0;
    //glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
    //glEnable( GL_TEXTURE_2D );
    glBegin( GL_QUADS );

        glNormal3f( 0.0, -1.0, 0.0 );

        glTexCoord2d( 0.0, 1.0 );
        glVertex3f( Wall + x, SkyH + y, Wall + z );
        glTexCoord2d( 0.0, 0.0 );
        glVertex3f( Wall + x, SkyH + y, - Wall + z );
        glTexCoord2d( 1.0, 0.0 );
        glVertex3f( - Wall + x, SkyH + y, - Wall + z );
        glTexCoord2d( 1.0, 1.0 );
        glVertex3f( - Wall + x, SkyH + y, Wall + z );

    glEnd();
    //glDisable( GL_TEXTURE_2D );
    glPopMatrix();
}

int forbiddenIDArrayLength = 0;
int ( *forbiddenIDs )[] = NULL;

void LoadForbidden( string path ) {
    FILE* handle = fopen( path.c_str(), "r" );
    if ( handle ) {
        int scanID = 0;
        while ( fscanf( handle, "%d", &scanID ) > 0 ) {
            forbiddenIDArrayLength++;
            forbiddenIDs = ( int (*)[] )( realloc( forbiddenIDs, forbiddenIDArrayLength * sizeof( int ) ) );
            ( *forbiddenIDs )[ forbiddenIDArrayLength - 1 ] = scanID;
        }
        fclose( handle );
    }
}

void RemoveForbidden() {
    if ( forbiddenIDArrayLength > 0 ) {
        free( forbiddenIDs );
        forbiddenIDs = NULL;
        forbiddenIDArrayLength = 0;
    }
}

bool ForbiddenID( int id ) {
    for ( int i = 0; i < forbiddenIDArrayLength; i++ ) {
        if ( ( *forbiddenIDs )[ i ] == id ) {
            return true;
        }
    }
    return false;
}

