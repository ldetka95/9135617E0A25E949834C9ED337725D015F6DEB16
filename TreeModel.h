#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include "Point3D.h"
#include "Coordinates.h"
#include "Map.h"

//Point3D COLLISION_POINT[] = { Point( 0, 1, 0 ), Point( 0, -1, 0 ), Point( 1, 0, 0 ), Point( -1, 0, 0 ), Point( 0, 0, 1 ), Point( 0, 0, -1 ) };

//#define BLOOD_MOVE_QUANT 0.1

class BloodStorage {
    public:
    BloodStorage( double sizeMin, double sizeMax, int msMin, int msMax ) {
        stack = new BloodStack();//Stack < BloodStorageElement > ();
        minSize = sizeMin;
        maxSize = sizeMax;
        minMS = msMin;
        maxMS = msMax;
    }
    ~BloodStorage() {
        delete stack;
    }
    void AddPoints( Point3D pos, int number, Point3D vel, double push ) {
        int T = ( int )( Timer::Current() * 1000.0 );
        ScalePoint3D( &vel, push );
        int msDif = maxMS - minMS + 1;
        for ( int i = 0; i < number; i++ ) {
            Point3D mvec = Balance( RandomVector( 0.2 + Random( 0.15 ) ), vel, 0.1 + Random( 0.8 ) );
            //mvec.y = -mvec.y;
            //mvec.y += 0.1;
            //ScalePoint3D( &mvec, push );
            double size = Random( maxSize - minSize ) + minSize;
            int timeout = T + rand() % msDif + minMS;
            ___Register( pos, mvec, size, timeout );
        }
    }
    void Process( Point3D gravityVector, double gravityPower, double gravityInertia ) {
        int T = ( int )( Timer::Current() * 1000.0 );
        stack -> Head();
        ScalePoint3D( &gravityVector, gravityPower );
        glPushAttrib( GL_LIGHTING_BIT );
        float redMaterial = MainColor[ 0 ] * 0.25;
        float bufferMaterialVA[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
        float bufferMaterialVD[ 4 ] = { redMaterial, 0.0, 0.0, 1.0 };
        float bufferMaterialVS[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
        float bufferMaterialVE[ 4 ] = { -0.1, -0.7, -0.7, 1.0 };
        glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, bufferMaterialVA );
        glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, bufferMaterialVD );
        glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, bufferMaterialVS );
        glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, bufferMaterialVE );
        glEnable( GL_POINT_SMOOTH );
        do {
            bool rem = false;
            do {
                rem = false;
                BloodStorageElement* element = stack -> CurrentGet();
                if ( element ) {
                    rem = element -> Process( T, gravityVector, gravityInertia, cam );
                }
                if ( rem ) {
                    stack -> CurrentRem();
                }
            } while ( rem );
        } while ( stack -> Next() );
        glDisable( GL_POINT_SMOOTH );
        glPopAttrib();
    }
    void RegisterCam( Point3D c ) {
        cam = c;
    }
    private:
    class BloodStorageElement {
        public:
        BloodStorageElement( Point3D p, Point3D mov, double size, int timeOut ) {
            to = timeOut;
            moveVec = mov;
            pos = p;
            dropsize = size;
            moving = true;
            next = NULL;
        }
        ~BloodStorageElement() {
        }
        bool Process( int t, Point3D gravityVector, double gravityInertia, Point3D cam ) {
            if ( t > to ) {
                return true;
            }
            if ( moving ) {
                moveVec = Balance( moveVec, gravityVector, gravityInertia );
                double cSize = dropsize * 0.001;
                Point3D pos2 = AddPoint( pos, moveVec );
                char cf = Collide( pos, pos2 );
                if ( cf ) {
                    moveVec = Point( 0.0, 0.0, 0.0 );
                    moving = false;
                    Point3D ic = IntChange( pos, pos2 );
                    if ( cf & 0x01 ) { // x
                        if ( pos.x < pos2.x ) {
                            pos.x = ic.x - cSize;
                        } else {
                            pos.x = ic.x + cSize;
                        }
                    }
                    if ( cf & 0x02 ) { // y
                        if ( pos.y < pos2.y ) {
                            pos.y = ic.y - cSize;
                        } else {
                            pos.y = ic.y + cSize;
                        }
                    }
                    if ( cf & 0x04 ) { // z
                        if ( pos.z < pos2.z ) {
                            pos.z = ic.z - cSize;
                        } else {
                            pos.z = ic.z + cSize;
                        }
                    }
                    if ( cf & 0x01 ) { // x, add pos
                        double dif = absf( pos2.x - ic.x ) / absf( pos2.x - pos.x );
                        pos.y += ( pos2.y - pos.y ) * dif;
                        pos.z += ( pos2.z - pos.z ) * dif;
                    }
                    if ( cf & 0x02 ) { // y, add pos
                        double dif = absf( pos2.y - ic.y ) / absf( pos2.y - pos.y );
                        pos.x += ( pos2.x - pos.x ) * dif;
                        pos.z += ( pos2.z - pos.z ) * dif;
                    }
                    if ( cf & 0x04 ) { // z, add pos
                        double dif = absf( pos2.z - ic.z ) / absf( pos2.z - pos.z );
                        pos.x += ( pos2.x - pos.x ) * dif;
                        pos.y += ( pos2.y - pos.y ) * dif;
                    }
                } else {
                    pos = pos2;
                }
            }
            // display
            double camScale = 1.25 / Dist3D( pos, cam );
            glPointSize( dropsize * camScale );
            Point3D normalVec = SubtractPoint( cam, pos );
            ScalePoint3D( &normalVec, camScale );
            glBegin( GL_POINTS );
                glNormal3f( normalVec.x, normalVec.y, normalVec.z );
                glVertex3f( pos.x, pos.y, pos.z );
            glEnd();
            return false;
        }
        inline char Collide( Point3D a, Point3D b ) {
            char flag = 0;
            if ( MainMap -> GetID( b.x, a.y, a.z ) ) {
                flag = flag | 0x01; // x
            }
            if ( MainMap -> GetID( a.x, b.y, a.z ) ) {
                flag = flag | 0x02; // y
            }
            if ( MainMap -> GetID( a.x, a.y, b.z ) ) {
                flag = flag | 0x04; // z
            }
            return flag;
        }
        inline Point3D IntChange( Point3D a, Point3D b ) {
            Point3D r = Point( -1.0, -1.0, -1.0 );
            if ( floor( a.x ) != floor( b.x ) ) {
                r.x = round( ( b.x + a.x ) / 2.0 );
            }
            if ( floor( a.y ) != floor( b.y ) ) {
                r.y = round( ( b.y + a.y ) / 2.0 );
            }
            if ( floor( a.z ) != floor( b.z ) ) {
                r.z = round( ( b.z + a.z ) / 2.0 );
            }
            return r;
        }
        /*inline Point3D FirstMapEdge( Point3D a, Point3D b ) {
            Point3D ret = b;//Point3D( -1.0, -1.0, -1.0 );
            double distAB = Dist3D( a, b );
            Point3D c = IntChange( a, b );
            Point3D dif = SubtractPoint( b, a );
            //Point3D p = Point3D( 0.0, 0.0, 0.0 );
            bool p[ 3 ] = { false, false, false };
            Point3D F[ 3 ];
            if ( c.x >= 0 ) {
                //ret.x = c.x;
                double de_factor = c.x / dif.x;
                F[ 0 ] = Point( c.x, dif.y * de_factor, dif.z * de_factor );
                p[ 0 ] = true;
            }
            if ( c.y >= 0 ) {
                //ret.y = c.y;
                double de_factor = c.y / dif.y;
                F[ 1 ] = Point( dif.x * de_factor, c.y, dif.z * de_factor );
                p[ 1 ] = true;
            }
            if ( c.z >= 0 ) {
                //ret.z = c.z;
                double de_factor = c.z / dif.z;
                F[ 2 ] = Point( dif.x * de_factor, dif.y * de_factor, c.z );
                p[ 2 ] = true;
            }
            for ( int i = 0; i < 3; i++ ) {
                if ( p[ i ] ) {
                    double d = Dist3D( a, F[ i ] );
                    if ( d < distAB ) {
                        distAB = d;
                        ret = F[ i ];
                    }
                }
            }
            return ret;
        }*/
        /*inline bool Collide( Point3D pos, Map* m ) {
            if ( MainMap -> GetID( pos.x, pos.y, pos.z ) ) {
                moving = false;
                return true;
            }
            return false;
        }*/
        Point3D moveVec;
        Point3D pos;
        double dropsize;
        int to;
        BloodStorageElement* next;
        bool moving;
    };
    Point3D RandomVector( double r ) {
        Point3D ret;
        double i = Random( 2 * PI );
        double a = Random( PI );
        stc( r, i, a, &( ret.x ), &( ret.y ), &( ret.z ) );
        return ret;
    }
    void ___Register( Point3D p, Point3D m, double size, int timeOut ) {
        BloodStorageElement* el = new BloodStorageElement( p, m, size, timeOut );
        stack -> Push( el );
    }
    class BloodStack {
        public:
        BloodStack() {
            head = NULL;
            current = NULL;
            currentPrev = NULL;
        }
        ~BloodStack() {
            Clear();
        }
        void Push( BloodStorageElement* e ) {
            e -> next = head;
            head = e;
        }
        void Clear() {
            BloodStorageElement* seeker = head;
            while ( seeker ) {
                BloodStorageElement* grinder = seeker;
                seeker = seeker -> next;
                delete grinder;
            }
            head = NULL;
        }
        void Head() {
            current = head;
            currentPrev = NULL;
        }
        bool Next() {
            if ( current ) {
                currentPrev = current;
                current = current -> next;
                return true;
            }
            return false;
        }
        BloodStorageElement* CurrentGet() {
            return current;
        }
        void CurrentRem() {
            if ( current ) {
                BloodStorageElement* grinder = current;
                current = current -> next;
                if ( currentPrev ) {
                    currentPrev -> next = current;
                } else {
                    head = current;
                }
                grinder -> next = NULL;
                delete grinder;
            }
        }
        private:
        BloodStorageElement* head;
        BloodStorageElement* current;
        BloodStorageElement* currentPrev;
    };
    BloodStack* stack;
    double minSize, maxSize;
    int minMS, maxMS;
    Point3D cam;
};

#define BLOOD_SIZE_MIN 20.0
#define BLOOD_SIZE_MAX 100.0

#define BLOOD_TIME_DECAY_MIN 6000
#define BLOOD_TIME_DECAY_MAX 10000

BloodStorage* bloodStorage = new BloodStorage( BLOOD_SIZE_MIN, BLOOD_SIZE_MAX, BLOOD_TIME_DECAY_MIN, BLOOD_TIME_DECAY_MAX );


#define STD_QUAD_WALL_POINT 4
#define STD_CUBE_VERTEX 8
#define STD_CUBE_WALL_COUNT 6

Point3D OFFSET_CALC_MASK[ STD_CUBE_WALL_COUNT ] = {
    Point( 1.0, 0.0, 0.0 ),
    Point( -1.0, 0.0, 0.0 ),
    Point( 0.0, 1.0, 0.0 ),
    Point( 0.0, -1.0, 0.0 ),
    Point( 0.0, 0.0, 1.0 ),
    Point( 0.0, 0.0, -1.0 )
};

int MAPPED_DEFORMED_CUBE_POINTS[ STD_CUBE_VERTEX ][ 3 ][ 2 ] = {
    { { 0, 1 }, { 3, 2 }, { 4, 0 } }, // 0
    { { 0, 2 }, { 1, 1 }, { 4, 1 } }, // 1
    { { 0, 0 }, { 3, 3 }, { 5, 3 } }, // 2
    { { 0, 3 }, { 1, 0 }, { 5, 0 } }, // 3
    { { 2, 2 }, { 3, 1 }, { 4, 3 } }, // 4
    { { 1, 2 }, { 2, 1 }, { 4, 2 } }, // 5
    { { 2, 3 }, { 3, 0 }, { 5, 2 } }, // 6
    { { 1, 3 }, { 2, 0 }, { 5, 1 } }  // 7
};

double FLESH_WALL_TEXCOORD_MAP[ STD_QUAD_WALL_POINT ][ 2 ] = {
    { 1.0, 0.0 }, { 0.0, 0.0 }, { 0.0, 1.0 }, { 1.0, 1.0 }
};

#define STD_FLESH_INERTIA 0.95
#define STD_FLESH_BOUNCE_FACTOR 0.6

class Flesh {
    class QuadWall {
        public:
        QuadWall() {
        }
        ~QuadWall() {
        }
        inline void Draw_Fast() {
            for ( int i = 0; i < STD_QUAD_WALL_POINT; i++ ) {
                glNormal3f( n[ i ].x, n[ i ].y, n[ i ].z );
                glVertex3f( p[ i ].x, p[ i ].y, p[ i ].z );
            }
        }
        inline void Draw_Fast_Textured( Bitmap* tstorage, double rsxStart, double rsxJump ) {//int tquantity ) {
            for ( int i = 0; i < STD_QUAD_WALL_POINT; i++ ) {
                glTexCoord2d( rsxStart + FLESH_WALL_TEXCOORD_MAP[ i ][ 0 ] * rsxJump, FLESH_WALL_TEXCOORD_MAP[ i ][ 1 ] );
                glNormal3f( n[ i ].x, n[ i ].y, n[ i ].z );
                glVertex3f( p[ i ].x, p[ i ].y, p[ i ].z );
            }
        }
        Point3D p[ STD_QUAD_WALL_POINT ];
        Point3D n[ STD_QUAD_WALL_POINT ];
    };
    public:
    Flesh( Point3D pos, Point3D rot, Point3D pos_vec, Point3D rot_vec, double size, double timeOut, int bloodTextureID, int bloodTextureQuantity ) {
        ___pos = pos;
        ___rot = rot;
        ___pvec = pos_vec;
        ___rvec = rot_vec;
        int n = 6;
        ___wall = ( QuadWall* )( calloc( sizeof( QuadWall ), n ) );
        ___wallCount = n;
        Set_Random_Walls_2( size );
        ___out = new Timer( timeOut * 1000.0 );
        if ( bloodTextureQuantity > 0 ) {
            ___rsxStart = double( bloodTextureID ) / double( bloodTextureQuantity );
        }
    }
    ~Flesh() {
        delete ___out;
    }
    void Display() {
        glPushMatrix();
        glTranslated( ___pos.x, ___pos.y, ___pos.z );
        glRotated( ___rot.z, 0.0, 0.0, 1.0 );
        glRotated( ___rot.y, 0.0, 1.0, 0.0 );
        glRotated( ___rot.x, 1.0, 0.0, 0.0 );
        glBegin( GL_QUADS );
        for ( int i = 0; i < ___wallCount; i++ ) {
            ___wall[ i ].Draw_Fast();
        }
        glEnd();
        glPopMatrix();
    }
    void Display_Textured( Bitmap* t, double j ) {
        glPushMatrix();
        glTranslated( ___pos.x, ___pos.y, ___pos.z );
        glRotated( ___rot.z, 0.0, 0.0, 1.0 );
        glRotated( ___rot.y, 0.0, 1.0, 0.0 );
        glRotated( ___rot.x, 1.0, 0.0, 0.0 );
        glBegin( GL_QUADS );
        for ( int i = 0; i < ___wallCount; i++ ) {
            ___wall[ i ].Draw_Fast_Textured( t, ___rsxStart, j );
        }
        glEnd();
        glPopMatrix();
    }
    bool Process() { // returns TRUE if should be still maintained
        if ( ___out -> Tick() ) {
            return false;
        }
        Point3D pzero = Point( 0.0, 0.0, 0.0 );
        ___pvec = Balance( grndVector, ___pvec, STD_FLESH_INERTIA );
        Point3D nextPos = Floor( AddPoint( ___pos, ___pvec ) );
        Point3D cIntPos = Floor( ___pos );
        if ( MainMap -> GetID( nextPos.x, nextPos.y, nextPos.z ) > 0 ) {
            if ( cIntPos.x != nextPos.x ) {
                ___pvec.x *= -STD_FLESH_BOUNCE_FACTOR;
            }
            if ( cIntPos.y != nextPos.y ) {
                ___pvec.y *= -STD_FLESH_BOUNCE_FACTOR;
            }
            if ( cIntPos.z != nextPos.z ) {
                ___pvec.z *= -STD_FLESH_BOUNCE_FACTOR;
            }
        }
        ScalePoint3D( &___rvec, STD_FLESH_INERTIA );
        ___pos = AddPoint( ___pos, ___pvec );
        ___rot = AddPoint( ___rot, ___rvec );
        if ( Dist3D( ___pvec, pzero ) < 0.002 ) {
            ___pvec = pzero;
        }
        if ( Dist3D( ___rvec, pzero ) < 0.002 ) {
            ___rvec = pzero;
        }
        return true;
    }
    void Set_Random_Walls( double size ) {
        Point3D v[ STD_CUBE_VERTEX ];
        double s2 = size / 2.0;
        for ( int i = 0; i < STD_CUBE_VERTEX; i++ ) {
            v[ i ] = Point( Random( -s2, s2 ), Random( -s2, s2 ), Random( -s2, s2 ) );
        }
        for ( int i = 0; i < ___wallCount; i++ ) {
            int iMostOffset[ STD_QUAD_WALL_POINT ] = { 0, 0, 0, 0 };
            ___Sort_By_Point_Array( iMostOffset, v, OFFSET_CALC_MASK[ i ], STD_CUBE_VERTEX, STD_QUAD_WALL_POINT );
            for ( int j = 0; j < STD_QUAD_WALL_POINT; j++ ) {
                ___wall[ i ].p[ j ] = v[ iMostOffset[ j ] ];
            }
        }
    }
    void Set_Random_Walls_2( double size ) {
        // step 1: generation
        Point3D v[ STD_CUBE_VERTEX ];
        for ( int i = 0; i < STD_CUBE_VERTEX; i++ ) { // binary encoding, ZYX order
            double x = ( double )( i & 0x01 ) - 0.5;
            double y = ( double )( ( i >> 1 ) & 0x01 ) - 0.5;
            double z = ( double )( ( i >> 2 ) & 0x01 ) - 0.5;
            v[ i ] = Point( x, y, z );
        }
        // step 2: deformation
        for ( int i = 0; i < STD_CUBE_VERTEX; i++ ) {
            double inclination, azimuth;
            cts( v[ i ].x, v[ i ].y, v[ i ].z, NULL, &inclination, &azimuth );
            double r = Random( size );
            stc( r, inclination, azimuth, &v[ i ].x, &v[ i ].y, &v[ i ].z );
        }
        // step 3: initialization
        Point3D pzero = Point( 0.0, 0.0, 0.0 );
        for ( int i = 0; i < STD_CUBE_VERTEX; i++ ) {
            for ( int j = 0; j < 3; j++ ) {
                int wid = MAPPED_DEFORMED_CUBE_POINTS[ i ][ j ][ 0 ];
                int qid = MAPPED_DEFORMED_CUBE_POINTS[ i ][ j ][ 1 ];
                ___wall[ wid ].p[ qid ] = v[ i ];
                Point3D n = v[ i ];
                ScalePoint3D( &n, 1.0 / Dist3D( pzero, n ) );
                ___wall[ wid ].n[ qid ] = n;
            }
        }
    }
    private:
    void ___Sort_By_Point_Array( int* ai, Point3D* ap, Point3D mask, int len, int sel ) {
        char* checked = ( char* )( calloc( 1, len ) );
        Point3D pzero = Point( 0.0, 0.0, 0.0 );
        double mx = ( mask.x + mask.y + mask.z ) / Dist3D( pzero, mask );
        for ( int i = 0; i < sel; i++ ) {
            int id = 0;
            double d = -1.0 * mx;
            double dinit = d;
            for ( int j = 0; j < len; j++ ) {
                if ( !checked[ j ] ) {
                    double cdist = Dist3D( pzero, Multiply3D( ap[ j ], mask ) ) * mx;
                    if ( mx >= 0.0 ) {
                        if ( cdist > d ) {
                            id = j;
                            d = cdist;
                        }
                    } else {
                        if ( cdist < d ) {
                            id = j;
                            d = cdist;
                        }
                    }
                }
            }
            if ( d != dinit ) {
                ai[ i ] = id;
                checked[ id ] = 1;
            }
        }
        free( checked );
    }
    Timer* ___out;
    QuadWall* ___wall;
    Point3D ___pos, ___rot;
    Point3D ___pvec, ___rvec;
    int ___wallCount;
    double ___rsxStart;
};

extern int std_Texture_Width, std_Texture_Height;

class FleshStorage {
    public:
    FleshStorage() {
        // textures
        ___fleshTexture = ___JoinedTex( "data/textures/blood.slf" );
    }
    ~FleshStorage() {
        list < Flesh* >::iterator it = ___f.begin();
        while ( it != ___f.end() ) {
            delete ( *it );
            it++;
        }
    }
    void GenerateFlesh( Point3D shoot_pos, Point3D shoot_vec, Point3D ent_pos, double shoot_power, int count, double size, double disp ) {
        //printf( "Shoot pos = ( %g, %g, %g )\n", shoot_pos.x, shoot_pos.y, shoot_pos.z );
        //double disp = 0.6;
        double rnd_rot = 90.0;
        double rota = 6.0;
        Point3D pzero = Point( 0.0, 0.0, 0.0 );
        //shoot_pos = SubtractPoint( shoot_pos, shoot_vec );
        for ( int i = 0; i < count; i++ ) {
            Point3D ap = Point( Random( -disp, disp ), Random( -disp, disp ), Random( -disp, disp ) );
            ap = AddPoint( ent_pos, ap );
            Point3D ar = Point( Random( -rnd_rot, rnd_rot ), Random( -rnd_rot, rnd_rot ), Random( -rnd_rot, rnd_rot ) );
            Point3D apv = SubtractPoint( ap, shoot_pos );
            ScalePoint3D( &apv, shoot_power / Dist3D( pzero, apv ) );
            apv = Balance( apv, shoot_vec, 0.5 );
            ScalePoint3D( &apv, 0.4 );
            Point3D arv = Point( Random( -rota, rota ), Random( -rota, rota ), Random( -rota, rota ) );
            Flesh* f = new Flesh( ap, ar, apv, arv, Random( size / 2.0, size ), Random( 6.0, 10.0 ), Random( 0, ___textureQ ), ___textureQ );
            ___f.push_back( f );
        }
    }
    void ProcessAll() {
        for ( UINT32 i = 0; i < ___f.size(); i++ ) {
            Flesh* f = ___f.front();
            ___f.pop_front();
            if ( f -> Process() ) {
                ___f.push_back( f );
            } else {
                delete f;
            }
        }
    }
    void DisplayAll() {
        if ( ___fleshTexture ) {
            ___fleshTexture -> UseImage();
        } else {
            glDisable( GL_TEXTURE_2D );
        }
        glPushAttrib( GL_LIGHTING_BIT );
        float redMaterial = MainColor[ 0 ] * 0.6;
        float redMaterial2 = MainColor[ 0 ] * 0.2;
        float redMaterial3 = MainColor[ 0 ] * 0.8;
        float bufferMaterialVA[ 4 ] = { redMaterial2, 0.0, 0.0, 1.0 };
        float bufferMaterialVD[ 4 ] = { redMaterial, 0.0, 0.0, 1.0 };
        float bufferMaterialVS[ 4 ] = { redMaterial3, 0.0, 0.0, 1.0 };
        float bufferMaterialVE[ 4 ] = { -0.1, -0.7, -0.7, 1.0 };
        glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, bufferMaterialVA );
        glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, bufferMaterialVD );
        glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, bufferMaterialVS );
        glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, bufferMaterialVE );
        double rsxJump = 1.0 / double( ___textureQ );
        //glColor3f( 0.8, 0.0, 0.0 );
        //glBegin( GL_QUADS );
            for ( UINT32 i = 0; i < ___f.size(); i++ ) {
                Flesh* f = ___f.front();
                ___f.pop_front();
                f -> Display_Textured( ___fleshTexture, rsxJump );
                ___f.push_back( f );
            }
        //glEnd();
        glPopAttrib();
        if ( !___fleshTexture ) {
            glEnable( GL_TEXTURE_2D );
        }
    }
    private:
    Bitmap* ___JoinedTex( string path ) {
        StringLinker* slinker = new StringLinker( path );
        Bitmap* bmp = NULL;
        int size = slinker -> GetSize();
        if ( size > 0 ) {
            bmp = new Bitmap();
            bmp -> SetDim( std_Texture_Width * size, std_Texture_Height, 4 );
            for ( int i = 0; i < size; i++ ) {
                Bitmap* buffer = new Bitmap( ( slinker -> GetStringOf( i ) ).c_str() );
                bmp -> CopyFrom_32( buffer, 0, 0, i * std_Texture_Width, 0, std_Texture_Width, std_Texture_Height );
                delete buffer;
            }
            bmp -> GL_Bitmap();
        }
        delete slinker;
        ___textureQ = size;
        return bmp;
    }
    Bitmap* ___fleshTexture;
    int ___textureQ;
    list < Flesh* > ___f;
};

FleshStorage* fleshStorage = new FleshStorage();

template < class T >
    class ArrayList { // implementation of array list: slow construction, very fast access
        public:
        ArrayList() {
            arrayList = NULL;
            quantity = 0;
        }
        ~ArrayList() {
            Clear();
        }
        void Append( T* item ) { // appends item to the list. Fast for small lists.
            ___Append( CreateRefElement( item, true ) );
        }
        void AppendReference( T* item ) { // appends only reference to item to the list. Fast for small lists.
            ___Append( CreateRefElement( item, false ) );
        }
        void Overwrite( T* item, int position ) { // overwrite EXISTING item at position. Very fast.
            DestroyRefElement( ( *arrayList )[ position ] );
            ( *arrayList )[ position ] = CreateRefElement( item, true );
        }
        void OverwriteReference( T* item, int position ) { // overwrite EXISTING reference at position. Very fast.
            DestroyRefElement( ( *arrayList )[ position ] );
            ( *arrayList )[ position ] = CreateRefElement( item, false );
        }
        void Insert( T* item, int position ) { // insert NEW item in position. Slow for bigger lists.
            struct RefElement* re = CreateRefElement( item, true );
            ___Resize( quantity + 1 );
            ___Move( position, quantity - 1, 1 );
            ( *arrayList )[ position ] = re;
        }
        void InsertReference( T* item, int position ) { // insert NEW reference in position. Slow for bigger lists.
            struct RefElement* re = CreateRefElement( item, false );
            ___Resize( quantity + 1 );
            ___Move( position, quantity - 1, 1 );
            ( *arrayList )[ position ] = re;
        }
        void Delete( int position ) { // delete specified item or reference. Slow for bigger lists.
            struct RefElement* grinder = ( *arrayList )[ position ];
            ___Move( position, quantity - 1, -1 );
            ___Resize( quantity - 1 );
            DestroyRefElement( grinder );
        }
        void Clear() { // delete all items in list. Automatically called on list destruction. Fast.
            if ( arrayList ) {
                for ( int i = 0; i < quantity; i++ ) {
                    DestroyRefElement( &( *arrayList )[ i ] );
                }
                free( arrayList );
                arrayList = NULL;
            }
            quantity = 0;
        }
        inline int Quantity() { // returns count of items in list, or its quantity. Ultra fast.
            return quantity;
        }
        inline T* Get( int position ) { // returns item on position, or NULL if item not present or position out of bounds. Super fast.
            if ( ( position < 0 ) || ( position >= quantity ) ) {
                return NULL;
            }
            struct RefElement* re = ( *arrayList )[ position ];
            if ( !re ) {
                return NULL;
            }
            return re -> el;
        }
        inline bool IsAllocated( int position ) { // returns true if item is separately allocated, false otherwise or if error. Super fast.
            // NOTE: use OR'd with IsReference() to determine if item exists.
            if ( ( position < 0 ) || ( position >= quantity ) ) {
                return false;
            }
            struct RefElement* re = ( *arrayList )[ position ];
            if ( !re ) {
                return false;
            }
            return re -> alc;
        }
        inline bool IsReference( int position ) { // returns true if item is a reference, false otherwise or if error. Super fast.
            // NOTE: use OR'd with IsAllocated() to determine if item exists.
            if ( ( position < 0 ) || ( position >= quantity ) ) {
                return false;
            }
            struct RefElement* re = ( *arrayList )[ position ];
            if ( !re ) {
                return false;
            }
            return !( re -> alc );
        }
        private:
        struct RefElement {
            public:
            bool alc;
            T* el;
        };
        struct RefElement* CreateRefElement( T* item, bool alloc ) {
            struct RefElement* ret = ( struct RefElement* )( malloc( sizeof( struct RefElement ) ) );
            if ( alloc ) {
                ( *( ret -> el ) ) = ( T* )( malloc( sizeof( T ) ) );
                ( *( ret -> el ) ) = ( *item );
            } else {
                ret -> el = item;
            }
            ret -> alc = alloc;
            return ret;
        }
        void DestroyRefElement( struct RefElement* ref ) {
            if ( ref ) {
                if ( ref -> alc ) {
                    free( ref -> el );
                }
            }
        }
        inline void ___Resize( int newLen ) {
            for ( int i = newLen; i < quantity; i++ ) {
                delete ( *arrayList )[ i ];
            }
            arrayList = ( RefElement* (*)[] )( realloc( arrayList, newLen * sizeof( struct RefElement ) ) );
            for ( int i = quantity; i < newLen; i++ ) {
                ( *arrayList )[ i ] = NULL;
            }
            quantity = newLen;
        }
        inline void ___Append( RefElement* rel ) {
            ___Resize( quantity + 1 );
            ( *arrayList )[ quantity - 1 ] = rel;
        }
        inline void ___Move( int a, int b, int c ) { // switch elements in range [a; b] by [c] fields.
            if ( c < 0 ) {
                for ( int i = a; i <= b; i++ ) {
                    ( *arrayList )[ i + c ] = ( *arrayList )[ i ];
                }
            } else if ( c > 0 ) {
                for ( int i = b; i >= a; i-- ) {
                    ( *arrayList )[ i + c ] = ( *arrayList )[ i ];
                }
            }
        }
        struct RefElement* ( *arrayList )[];
        int quantity;
    };


