#pragma once

#include "Point3D.h"

#define WATER_SURFACE_KC 8

class Water {
    public:
    class Properties {
        protected:
        Properties() {
            level = 6.25;
            color = Point( 0.15, 0.25, 0.45 );
        }
        Properties( FILE* handle ) { // handle must be opened
            fscanf( handle, "%lf\n", &level );
            fscanf( handle, "%lf %lf %lf\n", &color.x, &color.y, &color.z );
            fclose( handle );
        }
        public:
        static Properties* Create( string path ) {
            FILE* handle = fopen( path.c_str(), "r" );
            if ( handle ) {
                return new Properties( handle );
            }
            return NULL;
        }
        static Properties* CreateNull() {
            return new Properties();
        }
        ~Properties() {
        }
        void trySave( string path ) {
            FILE* handle = fopen( path.c_str(), "w" );
            if ( handle ) {
                fprintf( handle, "%lf\n", level );
                fprintf( handle, "%lf %lf %lf\n", color.x, color.y, color.z );
                fclose( handle );
            }
        }
        /// public fields - it makes no sense to mark them private and create getter and setter for each own.
        double level;
        Point3D color;
    };
    protected:
    class ControlMatrix {
        public:
        ControlMatrix( int x, int z, int d, int offset, GLfloat constValue ) {
            ___x = x;
            ___z = z;
            ___offset = offset;
            ___kc = x << 1; // temporary fix!
            ___k = ( GLfloat* )( malloc( ___kc * sizeof( GLfloat ) ) );
            ___density = d;
            ___density2 = d * d;
            ___matrix = ( GLfloat* )( malloc( x * z * sizeof( GLfloat ) * ___density2 * ___offset ) );
            reset( constValue );
            fillKnots();
        }
        ~ControlMatrix() {
            free( ___k );
            free( ___matrix );
        }
        GLfloat get( int x, int z, int o ) {
            int index = ___calcIndex( x, z, o );
            return ___matrix[ index ];
        }
        void set( int x, int z, int o, GLfloat v ) {
            int index = ___calcIndex( x, z, o );
            ___matrix[ index ] = v;
        }
        GLfloat* getPtr( int x, int z ) {
            return ___matrix + ___calcIndex( x, z, 0 );
        }
        void reset( GLfloat constValue ) {
            float d = ___density;
            for ( int ix = 0; ix < ___x * d; ix++ ) {
                for ( int iz = 0; iz < ___z * d; iz++ ) {
                    set( ix, iz, 0, float( ix ) / d );
                    set( ix, iz, 1, constValue );
                    set( ix, iz, 2, float( iz ) / d );
                }
            }
        }
        void fillKnots() {
            int kc2 = ___kc >> 1;
            for ( int i = 0; i < kc2; i++ ) {
                ___k[ i ] = 0.0;
            }
            for ( int i = kc2; i < ___kc; i++ ) {
                ___k[ i ] = 1.0;
            }
        }
        GLfloat* getMatrix() {
            return ___matrix;
        }
        int getX() {
            return ___x;
        }
        int getZ() {
            return ___z;
        }
        /*GLfloat* getKnotX() {
            return ___kx;
        }
        GLfloat* getKnotZ() {
            return ___kz;
        }*/
        GLfloat* getKnot() {
            return ___k;
        }
        int getKnotCount() {
            return ___kc;
        }
        void drawPoints() {
            glDisable( GL_LIGHTING );
            glColor3f( 1.0, 1.0, 1.0 );
            glPointSize( 5.0 );
            glBegin( GL_POINTS );
                for ( int ix = 0; ix < ___x; ix++ ) {
                    for ( int iz = 0; iz < ___z; iz++ ) {
                        glVertex3f( get( ix, iz, 0 ), get( ix, iz, 1 ), get( ix, iz, 2 ) );
                    }
                }
            glEnd();
            glEnable( GL_LIGHTING );
        }
        int getDensity() {
            return ___density;
        }
        private:
        int ___calcIndex( int x, int z, int o ) {
            return ( z * ___x * ___density + x ) * ___offset + o;
        }
        GLfloat* ___matrix;
        int ___x, ___z, ___offset;
        GLfloat* ___k;
        int ___kc;
        int ___density;
        int ___density2;
        //GLfloat* ___kx;
        //GLfloat* ___kz;
    };
    Water( Properties* prop ) { // properties not null
        ___properties = prop;
        ___nurbs = gluNewNurbsRenderer();
        ___controlMatrix = new ControlMatrix( 256, 256, 8, 3, prop -> level );
    }
    public:
    static Water* Create( string path ) {
        Properties* prop = Properties::Create( path );
        if ( prop ) {
            return new Water( prop );
        }
        return NULL;
    }
    static Water* CreateNull() {
        Properties* prop = Properties::CreateNull();
        return new Water( prop );
    }
    ~Water() {
        delete ___properties;
        gluDeleteNurbsRenderer( ___nurbs );
    }
    Properties* getProperties() {
        return ___properties;
    }
    void trySave( string propPath ) {
        ___properties -> trySave( propPath );
    }
    void display( Point3D pos, int range ) {
        // TODO: display
        //printf( "[Water displaying intensifies]\n" );
        // in
        glDisable( GL_TEXTURE_2D );
        // init drawing
        Point3D color = ___properties -> color;
        glColor3f( color.x, color.y, color.z );
        // surface draw
        int d = ___controlMatrix -> getDensity();
        int x = pos.x * d;//___controlMatrix -> getX() * d;
        int z = pos.z * d;//___controlMatrix -> getZ() * d;
        int xmax = ___controlMatrix -> getX() * d;
        int zmax = ___controlMatrix -> getZ() * d;
        int r = range * d;
        int ax = x - r;
        if ( ax < 1 ) {
            ax = 1;
        }
        int bx = x + r;
        if ( bx > xmax ) {
            bx = xmax;
        }
        int az = z - r;
        if ( az < 1 ) {
            az = 1;
        }
        int bz = z + r;
        if ( bz > zmax ) {
            bz = zmax;
        }
        glBegin( GL_QUADS );
            for ( int ix = ax; ix < bx; ix++ ) {
                for ( int iz = az; iz < bz; iz++ ) {
                    glVertex3fv( ___controlMatrix -> getPtr( ix - 1, iz - 1 ) );
                    glVertex3fv( ___controlMatrix -> getPtr( ix, iz - 1 ) );
                    glVertex3fv( ___controlMatrix -> getPtr( ix, iz ) );
                    glVertex3fv( ___controlMatrix -> getPtr( ix - 1, iz ) );
                }
            }
        glEnd();
        /*gluBeginSurface( ___nurbs );
        //printf( "%d\n", glGetError() );
        gluNurbsSurface(
            ___nurbs,
            //WATER_SURFACE_KC, ___KNOTS,
            //WATER_SURFACE_KC, ___KNOTS,
            ___controlMatrix -> getKnotCount(), ___controlMatrix -> getKnot(),
            ___controlMatrix -> getKnotCount(), ___controlMatrix -> getKnot(),
            ___controlMatrix -> getX() * 3, 3,
            ___controlMatrix -> getMatrix(),
            ___controlMatrix -> getX(),
            ___controlMatrix -> getZ(),
            GL_MAP2_VERTEX_3
        );
        //printf( " > %d\n", glGetError() );
        gluEndSurface( ___nurbs );*/
        // debug - point test draw
        //___controlMatrix -> drawPoints();
        // out
        glEnable( GL_TEXTURE_2D );
    }
    private:
    Properties* ___properties;
    GLUnurbs* ___nurbs;
    ControlMatrix* ___controlMatrix;
    //static GLfloat ___KNOTS[ WATER_SURFACE_KC ];
};


//GLfloat Water::___KNOTS[ WATER_SURFACE_KC ] = { 0.1, 0.1, 0.1, 0.1, 1.0, 1.0, 1.0, 1.0 };
