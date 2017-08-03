#pragma once

#include "Core.h"

#define MAX_MENU_IMG 256

Camera* menuCam;

Point3D ZeroPoint = Point( 0.0, 0.0, 0.0 );

#define MENU_GROUP_MAIN 1
#define MENU_GROUP_PAUSE 2
#define MENU_GROUP_LEVEL_SELECT 3

int menuButtonGroup = MENU_GROUP_MAIN;

class MenuClickableItem {
    public:
    MenuClickableItem( Point3D p, Point3D d, double w, double h, int tex, int tex2, bool c, int g ) {
        _pos = p;
        _dir = d;
        _tex = tex;
        _tex_cl = tex2;
        _w2 = w / 2.0;
        _h2 = h / 2.0;
        _c_on = c;
        _onclick = NULL;
        _group = g;
    }
    ~MenuClickableItem() {
    }
    void display( Point3D viewPos, Point3D viewDir, bool disp, int group ) {
        if ( ( group & _group ) == 0 ) {
            return;
        }
        Bitmap* b = _img[ _tex ];
        if ( disp ) {
            b = _img[ _tex_cl ];
        }
        if ( b ) {
            if ( b -> UseImage() ) {
                glPushMatrix();
                double a, i;
                cts( - _pos.x + _dir.x, - _pos.y + _dir.y, _pos.z - _dir.z, NULL, &i, &a );
                glTranslated( - _pos.x + viewPos.x, - _pos.y + viewPos.y, - _pos.z + viewPos.z );
                glRotated( a * 180.0 / PI - 90.0, 0.0, 1.0, 0.0 );
                glRotated( - i * 180.0 / PI + 90.0, 1.0, 0.0, 0.0 );
                glBegin( GL_QUADS );
                    glNormal3f( 0.0, 0.0, 1.0 );
                    glTexCoord2d( 0.0, 0.0 );
                    glVertex3f( -_w2, -_h2, 0.0 );
                    glTexCoord2d( 1.0, 0.0 );
                    glVertex3f( _w2, -_h2, 0.0 );
                    glTexCoord2d( 1.0, 1.0 );
                    glVertex3f( _w2, _h2, 0.0 );
                    glTexCoord2d( 0.0, 1.0 );
                    glVertex3f( -_w2, _h2, 0.0 );
                glEnd();
                glPopMatrix();
            }
        }
    }
    Point3D position() {
        return _pos;
    }
    void setClickAction( void ( *f )() ) {
        _onclick = f;
    }
    bool correctGroup( int g ) {
        return ( ( g & _group ) > 0 );
    }
    void clickAction() {
        if ( _onclick ) {
            _onclick();
        }
    }
    void getSphericalAnglesFrom( Point3D p, Point3D* r ) { // don't trust this method
        Point3D v[ 4 ] = {
            Point( -_w2, -_h2, 0.0 ),
            Point( _w2, -_h2, 0.0 ),
            Point( _w2, _h2, 0.0 ),
            Point( -_w2, _h2, 0.0 )
        };
        for ( int i = 0; i < 4; i++ ) {
            Point3D a = p;
            Point3D b = AddPoint( _pos, v[ i ] );
            a = SubtractPoint( a, b );
            cts( a.x, a.y, a.z, &r[ i ].x, &r[ i ].y, &r[ i ].z );
        }
    }
    double cursorRelDist( Point3D from, Point3D dir, double range ) {
        double q = 0.02;
        Point3D a = from;
        Point3D va = dir;
        ScalePoint3D( &va, -q / Dist3D( ZeroPoint, va ) );
        double minDist = Dist3D( ZeroPoint, _pos );
        for ( double i = 0; i < range; i += q ) {
            double d = Dist3D( _pos, a );
            if ( d < minDist ) {
                minDist = d;
            }
            //printf( "d = %g\n", d );
            a = AddPoint( a, va );
        }
        //printf( "\n\n" );
        return minDist;
    }
    static void getImages( string path ) {
        StringLinker* slinker = new StringLinker( path );
        for ( int i = 0; i < MAX_MENU_IMG; i++ ) {
            _img[ i ] = NULL;
        }
        for ( int i = 0; i < slinker -> GetSize(); i++ ) {
            _img[ slinker -> GetIdOf( i ) ] = new Bitmap( ( slinker -> GetStringOf( i ) ).c_str() );
            _img[ slinker -> GetIdOf( i ) ] -> GL_Bitmap();
        }
        delete slinker;
    }
    static void remImages() {
        for ( int i = 0; i < MAX_MENU_IMG; i++ ) {
            if ( _img[ i ] ) {
                delete _img[ i ];
            }
        }
    }
    static Bitmap* getUsedImage( int i ) {
        if ( _img[ i ] ) {
            if ( _img[ i ] -> UseImage() ) {
                return _img[ i ];
            }
        }
        return NULL;
    }
    private:
    static Bitmap* _img[ MAX_MENU_IMG ];
    Point3D _pos;
    Point3D _dir;
    int _tex, _tex_cl;
    double _w2, _h2;
    bool _c_on;
    void ( *_onclick )();
    int _group;
};

Bitmap* MenuClickableItem::_img[ MAX_MENU_IMG ];

#define STD_MENU_FOV 70

list< MenuClickableItem* > menuCI;

Point3D playerMenuPos = Point( 0.0, 0.0, 0.0 );

void Menu_Click_Play() {
    MIND_SAVED_STATE = MIND_STATE;
    MIND_STATE = STATE_LOADING;
    //printf( "Mind = %d, mind saved = %d\n", MIND_STATE, MIND_SAVED_STATE );
    //menuButtonGroup = MENU_GROUP_LEVEL_SELECT;
}

void Menu_Click_Exit() {
    MIND_STATE = STATE_QUIT;
    if ( connector ) {
        delete connector;
        connector = NULL;
    }
}

void Menu_Click_Resume() {
    MIND_STATE = STATE_ON_RESUME_GAME;
}

void Menu_Click_Back() {
    switch ( menuButtonGroup ) {
        case MENU_GROUP_PAUSE: {
            menuButtonGroup = MENU_GROUP_MAIN;
            MIND_STATE = STATE_ON_ABORT_GAME;
            if ( connector ) {
                delete connector;
                connector = NULL;
            }
            break;
        }
    }
}

void Menu_Click_Multiplayer_Join() {
    if ( !connector ) {
        FILE* handle = fopen( "data/serverinfo.txt", "r" );
        if ( handle ) {
            char IP[ 256 ];
            int port;
            if ( fscanf( handle, "%s %d", IP, &port ) == 2 ) {
                connector = new Connector( IP, port );
            }
            fclose( handle );
        }
        if ( !connector ) {
            connector = new Connector( "127.0.0.1", 33000 );
        }
        if ( connector -> isConnected() ) {
            MIND_SAVED_STATE = MIND_STATE;
            MIND_STATE = STATE_LOADING;
            //printf( "Mind = %d, mind saved = %d\n", MIND_STATE, MIND_SAVED_STATE );
        } else {
            delete connector;
            connector = NULL;
        }
    }
    //static int a = 0;
    //TestState state;
    //state.code = a;
    //a++;
    //Message* msg = new Message( sizeof( state ), MESSAGE_CODE_TEST, &state );
    //connector -> directSend( msg );
    //delete msg;
}

MenuClickableItem* nearestItem;
MenuClickableItem* nearestItem_clickOn;

void Menu_Init() {
    MenuClickableItem::getImages( "data/menudata/image.slf" );
    const int ITEMS = 5;
    MenuClickableItem* item[ ITEMS ] = {
        new MenuClickableItem(
            Point( 0.0, -0.1, 0.85 ),
            playerMenuPos,
            1.0, 1.0, 1, 2, true, MENU_GROUP_MAIN
        ),
        new MenuClickableItem(
            Point( 0.0, 1.0, 1.0 ),
            playerMenuPos,
            1.0, 1.0, 3, 4, true, MENU_GROUP_MAIN | MENU_GROUP_PAUSE
        ),
        new MenuClickableItem(
            Point( -0.5, -0.1, 1.0 ),
            playerMenuPos,
            1.0, 1.0, 5, 6, true, MENU_GROUP_PAUSE
        ),
        new MenuClickableItem(
            Point( 0.5, -0.1, 1.0 ),
            playerMenuPos,
            1.0, 1.0, 7, 8, true, MENU_GROUP_PAUSE
        ),
        new MenuClickableItem(
            Point( -1.0, 0.0, 0.7 ),
            playerMenuPos,
            1.0, 1.0, 9, 10, true, MENU_GROUP_MAIN
        )
    };
    item[ 0 ] -> setClickAction( Menu_Click_Play );
    item[ 1 ] -> setClickAction( Menu_Click_Exit );
    item[ 2 ] -> setClickAction( Menu_Click_Resume );
    item[ 3 ] -> setClickAction( Menu_Click_Back );
    item[ 4 ] -> setClickAction( Menu_Click_Multiplayer_Join );
    for ( int i = 0; i < ITEMS; i++ ) {
        menuCI.push_back( item[ i ] );
    }
    menuCam = new Camera( playerMenuPos.x, playerMenuPos.y, playerMenuPos.z, 0.0, 0.0 );
}

void Menu_Destroy() {
    MenuClickableItem::remImages();
    delete menuCam;
}

void Menu_MousePressed( int button, int state, int x, int y ) {
    if ( button == GLUT_LEFT_BUTTON ) {
        if ( state == GLUT_DOWN ) {
            nearestItem_clickOn = nearestItem;
        } else if ( state == GLUT_UP ) {
            if ( ( nearestItem ) && ( nearestItem == nearestItem_clickOn ) ) {
                nearestItem -> clickAction();
            }
        }
    }
}

#define MENU_ITEM_CATCH_DIST 10.0
#define MENU_ITEM_CATCH_RADIUS 0.5

void Menu_Motion( int x, int y ) {
    double difX = double( x - lastMouseX ) * 0.0035 * mSensTable[ mSensitivityI ][ 0 ];
    double difY = double( y - lastMouseY ) * 0.0035 * mSensTable[ mSensitivityI ][ 1 ] * invertMouse;
    Uni_Motion( x, y );
    if ( menuCam ) {
        menuCam -> pitchAngle += difX;
        double aDGR = degr( menuCam -> pitchAngle );
        if ( aDGR > 360.0 ) {
            menuCam -> pitchAngle -= 2 * PI;
        } else if ( aDGR < 0.0 ) {
            menuCam -> pitchAngle += 2 * PI;
        }
        menuCam -> yawAngle += difY;
    }
    if ( menuCam ) {
        menuCam -> CalcLookAngle();
    }
    // menu item selection
    nearestItem = NULL;
    list< MenuClickableItem* >::iterator it = menuCI.begin();
    double md = INFINITY;
    while ( it != menuCI.end() ) {
        double d = ( *it ) -> cursorRelDist( menuCam -> GetActorView(), menuCam -> GetView(), MENU_ITEM_CATCH_DIST );
        if ( ( d < MENU_ITEM_CATCH_RADIUS ) && ( d < md ) && ( *it ) -> correctGroup( menuButtonGroup ) ) {
            nearestItem = ( *it );
            md = d;
        }
        it++;
    }
}

double menuBGtexCount = 16.0;
double menuBGtexDist = 70.0;
int menuBGsectorLength = 20;

Point3D menuVeinCylinderRotation = Point( 1.2, 0.7, 0.25 );

double MENU_loading_start_time = -1.0;
double PLAYER_FALLING_SPEED = 0.0;
double PLAYER_FALLING_THRESHOLD = 2.75;
double PLAYER_FALLING_OFFSET = 0.0;

void DrawMenuBG() {
    Bitmap* bg;
    if ( ( bg = MenuClickableItem::getUsedImage( 0 ) ) != NULL ) {
        glPushMatrix();
        glRotated( Timer::Current() * menuVeinCylinderRotation.z, 0.0, 0.0, 1.0 );
        glRotated( Timer::Current() * menuVeinCylinderRotation.x, 0.0, 1.0, 0.0 );
        glRotated( Timer::Current() * menuVeinCylinderRotation.y, 1.0, 0.0, 0.0 );
        if ( MENU_loading_start_time >= 0.0 ) { // Probably loading?
            PLAYER_FALLING_OFFSET += log( 1.0 + ( Timer::Current() - MENU_loading_start_time ) * PLAYER_FALLING_THRESHOLD ) * PLAYER_FALLING_SPEED;
            glTranslated( 0.0, PLAYER_FALLING_OFFSET, 0.0 );
        } else {
            PLAYER_FALLING_OFFSET = 0.0;
        }
        double ajmp = 2.0 * M_PI / menuBGtexCount;
        double fullSectorLength = ( 2.0 * M_PI ) * menuBGtexDist - 2.9;
        double fullHalfLength = double( menuBGsectorLength ) * ( fullSectorLength / 2.0 );
        for ( int j = 0; j < menuBGsectorLength; j++ ) {
            double hx = double( j ) * fullSectorLength - fullHalfLength;
            for ( double i = 0; i < menuBGtexCount; i += 1.0 ) {
                double x1, y1, x2, y2;
                double t1 = ajmp * i;
                double t2 = ajmp * ( i + 1 );
                double tx1 = t1 / ( 2.0 * PI );
                double tx2 = t2 / ( 2.0 * PI );
                ptc( menuBGtexDist, t1, &x1, &y1 );
                ptc( menuBGtexDist, t2, &x2, &y2 );
                double dx = x2 - x1;
                double dy = y2 - y1;
                double h = sqrt( dx * dx + dy * dy ) / 2.0 * menuBGtexCount;
                glBegin( GL_QUADS );
                    glTexCoord2d( tx1, 0.0 );
                    glVertex3d( x1, hx - h, y1 );
                    glTexCoord2d( tx2, 0.0 );
                    glVertex3d( x2, hx - h, y2 );
                    glTexCoord2d( tx2, 1.0 );
                    glVertex3d( x2, hx + h, y2 );
                    glTexCoord2d( tx1, 1.0 );
                    glVertex3d( x1, hx + h, y1 );
                glEnd();
            }
        }
        glPopMatrix();
    }
}

double MENU_FOG_START = 0.0;
double MENU_FOG_END = 1000.0;

GLfloat menufogcolor[ 3 ] = { 0.1, 0.0, 0.0 };

void Menu_Display() {
    // TO-DO: Menu
    glClearColor( menufogcolor[ 0 ], menufogcolor[ 1 ], menufogcolor[ 2 ], 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glColor3f( 0.5, 0.5, 0.5 );
    glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( STD_MENU_FOV, double( wX ) / double ( wY ), 0.1, MAX_DRAW_DIST );
    glMatrixMode( GL_MODELVIEW );
    //glEnable( GL_LIGHTING );
    glDisable( GL_LIGHTING );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_DEPTH_TEST );
    glFogf(	GL_FOG_START, MENU_FOG_START );
    glFogf(	GL_FOG_END, MENU_FOG_END );
    glFogi( GL_FOG_MODE, GL_LINEAR );
    glHint( GL_FOG_HINT, GL_FASTEST );
    glFogfv( GL_FOG_COLOR, menufogcolor );
    glEnable( GL_FOG );
    menuCam -> LookAtAngles();
    glTranslated( 0.0, PLAYER_FALLING_OFFSET, 0.0 );
    DrawMenuBG();
    list< MenuClickableItem* >::iterator it = menuCI.begin();
    while ( it != menuCI.end() ) {
        ( *it ) -> display( playerMenuPos, menuCam -> GetView(), nearestItem == ( *it ), menuButtonGroup );
        it++;
    }
    glFlush();
    glutSwapBuffers();
}

void Menu_Reshape() {
    Menu_Display();
}

void Menu_Main() {
    Menu_Display();
}

void MenuResetCam() {
    menuCam -> pitchAngle = 0.0;
    menuCam -> yawAngle = 0.0;
    lastMouseX = wX / 2.0;
    lastMouseY = wY / 2.0;
    glutWarpPointer( lastMouseX, lastMouseY );
}

template < class V >
    class LevelSelector {
        public:
        LevelSelector( int group ) {
            ___g = group;
        }
        ~LevelSelector() {
            /*map < MenuClickableItem*, V >::iterator it = ___select.begin();
            while ( it != ___select.end() ) {

                it++;
            }*/
        }
        void add( MenuClickableItem* item, V v ) {
            ___blist.push_back( item );
            ___select[ item ] = v;
        }
        bool valueFor( MenuClickableItem* item, V* v ) {
            if ( ___select.find( item ) == ___select.end() ) {
                return false;
            }
            if ( v ) {
                ( *v ) = ___select[ item ];
            }
            return true;
        }
        void remove( MenuClickableItem* item ) {
            ___select.erase( item );
            list < MenuClickableItem* >::iterator it = ___blist.begin();
            while ( it != ___blist.end() ) {
                MenuClickableItem* i = ( *it );
                if ( i == item ) {
                    ___blist.erase( it );
                    delete i;
                    break;
                }
                it++;
            }
        }
        private:
        int ___g;
        list < MenuClickableItem* > ___blist;
        map < MenuClickableItem*, V > ___select;
    };

LevelSelector< int >* levelSelector = NULL;

void InitSelectors() {
    int level_selector_buttons_max_amp = 60.0;
    levelSelector = new LevelSelector< int >( MENU_GROUP_LEVEL_SELECT );

}

