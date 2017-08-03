#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include "Coordinates.h"
#include "Point3D.h"
#include "Textures.h"
#include "TreeModel.h"

#define BILLBOARD_POINT_NUM 4

class BillBoardStorage {
    public:
    BillBoardStorage() {
        storage = new Stack < BillBoard >();
    }
    ~BillBoardStorage() {
        Clear();
        delete storage;
    }
    void Clear() {
        storage -> Head();
        do {
            BillBoard* billBoard = storage -> CurrentGet();
            if ( billBoard ) {
                delete billBoard;
            }
        } while ( storage -> Next() );
        storage -> Clear();
    }
    void Append( Point3D* quad, int textureID ) {
        //storage -> Append( &new BillBoard( quad, textureID ) );
    }
    void Append( Point3D center, double incl, double azim, double w, double h, int textureID ) {
        BillBoard* b = new BillBoard( center, Point( azim, incl, 0.0 ), w, h, textureID );
        storage -> Push( *b );
        //delete b;
    }
    void Display() {
        storage -> Head();
        do {
        //for ( int i = 0; i < storage -> Quantity(); i++ ) {
            BillBoard* billBoard = storage -> CurrentGet();
            if ( billBoard ) {
                billBoard -> Display_FromCenter();
            }
        //}
        } while ( storage -> Next() );
    }
    private:
    class BillBoard {
        public:
        BillBoard( Point3D* q, int tID ) {
            for ( int i = 0; i < BILLBOARD_POINT_NUM; i++ ) {
                Quad[ i ] = *( q + i );
            }
            id = tID;
            Recalc_Normal();
            texCoord = 0x2D;
        }
        BillBoard( Point3D center, Point3D angle, double width, double height, int tID ) {
            Center = center;
            Angle = angle;
            double w2 = double( width ) / 2.0;
            double h2 = double( height ) / 2.0;
            Quad[ 0 ] = Point( Center.x - w2, Center.y + h2, Center.z );
            Quad[ 1 ] = Point( Center.x + w2, Center.y + h2, Center.z );
            Quad[ 2 ] = Point( Center.x + w2, Center.y - h2, Center.z );
            Quad[ 3 ] = Point( Center.x - w2, Center.y - h2, Center.z );
            id = tID;
            Normal = Point( 0.0, 0.0, 1.0 );
            texCoord = 0x2D;
        }
        ~BillBoard() {
        }
        void Recalc_Normal() {
            /// TO-DO ( maybe )
        }
        void Display() {
            if ( !image[ id ] ) {
                return;
            }
            if ( image[ id ] -> UseImage() ) {
                glBegin( GL_QUADS );
                glNormal3f( Normal.x, Normal.y, Normal.z );
                for ( int i = 0; i < BILLBOARD_POINT_NUM; i++ ) {
                    glVertex3f( Quad[ i ].x, Quad[ i ].y, Quad[ i ].z );
                }
                glEnd();
            }
        }
        void Display_FromCenter() {
            if ( !image[ id ] ) {
                return;
            }
            if ( image[ id ] -> UseImage() ) {
                // prepare translation
                glPushMatrix();
                glTranslated( Center.x, Center.y, Center.z );
                glRotated( Angle.y, 0.0, 1.0, 0.0 );
                glRotated( Angle.x, 1.0, 0.0, 0.0 );
                glRotated( Angle.z, 0.0, 0.0, 1.0 );
                glTranslated( -Center.x, -Center.y, -Center.z );
                // display
                glBegin( GL_QUADS );
                glNormal3f( Normal.x, Normal.y, Normal.z );
                for ( int i = 0; i < BILLBOARD_POINT_NUM; i++ ) {
                    glTexCoord2d( ( texCoord >> ( ( i << 1 ) + 1 ) ) & 0x01, ( texCoord >> ( i << 1 ) ) & 0x01 );
                    glVertex3f( Quad[ i ].x, Quad[ i ].y, Quad[ i ].z );
                }
                glEnd();
                // previous transformation matrix
                glPopMatrix();
            }
        }
        Point3D Center;
        Point3D Angle;
        Point3D Normal;
        unsigned char texCoord;
        Point3D Quad[ BILLBOARD_POINT_NUM ];
        int id;
    };
    //ArrayList < BillBoard >* storage;
    Stack < BillBoard >* storage;
};

BillBoardStorage* MainBillBoardStorage = new BillBoardStorage();
