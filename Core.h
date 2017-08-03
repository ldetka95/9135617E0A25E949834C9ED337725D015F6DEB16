#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <gl/gl.h>
#include <glut/glut.h>

#include <math.h>

#include <time.h>

#include "Camera.h"
#include "Textures.h"
#include "Map.h"
#include "Audio.h"
#include "Entity.h"
#include "Blocks.h"
#include "Coordinates.h"
#include "Stringlinker.h"
#include "Lighter.h"
#include "Billboard.h"

#include "MPConnector.h"

#include "ScreenUtils.h"
//#include "Radio.h"
#include "VorbisRadio.h"

#include "DelayedThread.h"

#define SCREENX 800.0
#define SCREENY 600.0

GLuint imageGL = 0; // unused?

#define MAX_DRAW_DIST 2048.0

#define EDITMODE_VISIBLITY_RANGE 16.0

#define SKY_TEXTURE_MAIN 255

#define TEXTURE_DEBUG 200
#define TEXTURE_CURSOR 201
#define TEXTURE_MONSTER_TEST 202
#define TEXTURE_MONSTER_DOG 203
#define TEXTURE_MARK 254

#define CHUNK_RANGE_MIN 3
#define CHUNK_RANGE_INIT 5
#define CHUNK_RANGE_MAX 20

int chunk_Range = CHUNK_RANGE_INIT;

double playerSpeed_EditMode_Init[ 2 ] = { 0.05, 0.20 };
int playerSpeed_EditMode_ArrayID = 0;
double playerSpeed_EditMode = 0.05;
bool weapons_enabled = true;

#include "pse2_CustomInterpreter.h"
#include "RWGame.h"
#include "Effect2D.h"

#include "Cheat.h"
#include "ObjectModel.h"

#define STATE_UNDEFINED -1
#define STATE_GAME 0
#define STATE_MENU 1
#define STATE_LOADING 2
#define STATE_GAME_MP 3
#define STATE_INIT 98
#define STATE_QUIT 99

#define STATE_ON_PAUSE_GAME 1000
#define STATE_ON_RESUME_GAME 1001
#define STATE_ON_MENU_INIT_GAME 1002
#define STATE_ON_ABORT_GAME 1003

#define STATE_ON_KILLED 9999

/*#define MIND_NOEVENT 0
#define MIND_EVENT_EXIT 1
int MIND_EVENT = MIND_NOEVENT;
*/

int MIND_STATE = STATE_INIT;
int MIND_SAVED_STATE = STATE_UNDEFINED;

double BlockSize = 1.0;
double hBlockSize = BlockSize / 2.0;

double minDamage = 15.0;
double maxDamage = 25.0;

/*double Random( double a, double b ) {
    return Random( b - a ) + a;
}*/

int mapX = 128;
int mapY = 32;
int mapZ = 128;

int visiblityRange = 0; // loaded
double FOV_SCOPE = 0.0;
double ROLLAMP = 0.0;

Block* block;
Block* tmpCube;
double cubeBlockSize = 0.3;

int SnapShotVRange = 256;

int currentLevel = 0;

Point3D StdShootPoint = Point( 0.24, -0.45, -0.8 );
Point3D BasicShootPoint = StdShootPoint;
Point3D StdRunAmplitudePoint = Point( 0.11, -0.18, -0.72 );
Point3D StdRunAmplitudeTargetPoint = AddPoint( StdRunAmplitudePoint, Point( 0.1, -0.15, -1.0 ) );
Point3D StdScopePoint = Point( 0.0, -0.3, -0.8 );
Point3D backupShootPoint = StdShootPoint;

void RespawnPlayer() {
    ROLLAMP = 0.0;
    player -> Respawn( MainMap );
    player -> SetVar( VAR_HP, player -> GetVar( VAR_MAX_HP ) );
    glutWarpPointer( SCREENX / 2, SCREENY / 2 );
    mainCamHandle -> RespawnAngles( MainMap );
    ResetMainColor();
    for ( int i = 0; i < MAX_OBJECT; i++ ) {
        monsterSpawnPending[ i ] = 0;
    }
    for ( int i = 0; i < MAX_MONSTERS; i++ ) {
        if ( monsterStorage[ i ] ) {
            delete monsterStorage[ i ];
            monsterStorage[ i ] = NULL;
        }
    }
}

Timer* timerShoot;
Timer* timerMonsterSpawn;

void InitModels() { // MEMORY CORRUPTION OCCURED
    // UPDATE: no more leaks - mainModel is up to 256 elements now
    //image[ 1 ] -> Debug_Info();
    StringLinker* mslinker = new StringLinker( "data/models/models.slf" );
    //image[ 1 ] -> Debug_Info();
    for ( int i = 0; i < mslinker -> GetSize(); i++ ) {
        int index = mslinker -> GetIdOf( i );
        //printf( "i = %d, index = %d\n", i, index );
        //image[ 1 ] -> Debug_Info();
        mainModel[ index ] = new Model( 0 );
        if ( !( mainModel[ index ] -> FromFile( mslinker -> GetStringOf( i ).c_str() ) ) ) {
            delete mainModel[ index ];
            mainModel[ index ] = NULL;
        }
    }
    delete mslinker;
    //image[ 1 ] -> Debug_Info();
}

Point3D editMark = { 0.0, 0.0, 0.0 };
int editMode = 0;

void UpdateChunks() {
    MainMap -> RemoveOptimized();
    MainMap -> CreateOptimized();
}

int std_Texture_Width = 16;
int std_Texture_Height = 16;

#define GAME_SAVE_PATH "data/saved/test.sav"

bool level_Initialized = false;

void Level_CleanUp() {
    if ( MainMap ) {
        MainMap -> SaveMap();
        delete MainMap;
        MainMap = NULL;
    }
}

void Level_Init() {
    // cleanup old level
    Level_CleanUp();
    ResetStacks();
    MainBillBoardStorage -> Clear();
    // prefetch level
    StringLinker* levelsSL = new StringLinker( "data/level/levels.slf" );
    MainMap = new Map( levelsSL -> GetString( currentLevel ) );
    ReloadScheme( currentLevel );
    MainMap -> StdTextureParam( std_Texture_Width, std_Texture_Height, MAX_TEXTURES );
    for ( int i = 0; i < MAX_TEXTURES; i++ ) {
        if ( image[ i ] ) {
            MainMap -> AssignTexture( i, image[ i ] );
        }
    }
    MainMap -> JoinTextures();
    RespawnPlayer();
    editMark = player -> FixedPlayerSpot();
    if ( MainMap ) {
        //MainMap -> TryDrawVisibleFields();
        UpdateChunks();
    }
    // fog update
    glFogi( GL_FOG_MODE, GL_LINEAR );
    glHint( GL_FOG_HINT, GL_FASTEST );
    // pse2
    ReloadPSEVM( levelsSL -> GetString( currentLevel ) );
    // bg music
    StringLinker* musicBGlinker = new StringLinker( "data/audio/levelMusic.slf" );
    LoopSound( CHANNEL_BACKGROUND, musicBGlinker -> GetString( currentLevel ) );
    delete musicBGlinker;
    // postfetch level
    ammoSpawnStack -> Load( MainMap -> GetMapPath() );
    if ( weaponSpawnStack ) {
        delete weaponSpawnStack;
    }
    weaponSpawnStack = new WeaponSpawnStack();
    weaponSpawnStack -> Load( MainMap -> GetMapPath() );
    // get loaded game
    //LoadGameState( GAME_SAVE_PATH );
    // out init
    weapons_enabled = true;
    delete levelsSL;
    MainMapFiller -> Reset();
    level_Initialized = true;
}

void Level_Init_MP() {
    // basic level init
    Level_CleanUp();
    ResetStacks();
    MainBillBoardStorage -> Clear();
    editMark = player -> FixedPlayerSpot();
    glFogi( GL_FOG_MODE, GL_LINEAR );
    glHint( GL_FOG_HINT, GL_FASTEST );
    // get map data
    Message* request = new Message( 0, MESSAGE_CODE_MAP_REQUEST, NULL );
    connector -> directSend( request );
    delete request;
    bool loaded_by_network = false;
    while ( !loaded_by_network ) {
        connector -> lockQueue();
        Message* answer = connector -> nextMessage();
        connector -> unlockQueue();
        if ( answer ) {
            switch ( answer -> getCode() ) {
                case MESSAGE_CODE_MAP_FINISHED: {
                    MyInfo* info = ( MyInfo* )( answer -> getData() );
                    playersOnServer -> add( info -> id, player );
                    printf( "My ID = %d\n", info -> id );
                    loaded_by_network = true;
                    printf( "Finished.\n" );
                    break;
                }
                case MESSAGE_CODE_MAP_PROPERTIES: {
                    MapProperties* prop = ( MapProperties* )( answer -> getData() );
                    MainMap = new Map( prop -> sizeX, prop -> sizeY, prop -> sizeZ, prop -> blockSize );
                    break;
                }
                case MESSAGE_CODE_MAP_PART: {
                    if ( MainMap ) {
                        MapPart* part = ( MapPart* )( answer -> getData() );
                        int start = part -> begin;
                        int end = start + part -> length;
                        int* data = part -> data;
                        int* mapData = MainMap -> GetRawData();
                        for ( int i = start; i < end; i++ ) {
                            mapData[ i ] = data[ i - start ];
                        }
                    }
                    break;
                }
            }
            //printf( "Got message code: %d, bytes: %d\n", answer -> getCode(), answer -> getSize() );
            delete answer;
        } else {
            Sleep( 100 );
        }
    }
    MainMap -> StdTextureParam( std_Texture_Width, std_Texture_Height, MAX_TEXTURES );
    for ( int i = 0; i < MAX_TEXTURES; i++ ) {
        if ( image[ i ] ) {
            MainMap -> AssignTexture( i, image[ i ] );
        }
    }
    MainMap -> JoinTextures();
    RespawnPlayer();
    editMark = player -> FixedPlayerSpot();
    if ( MainMap ) {
        //MainMap -> TryDrawVisibleFields();
        //MainMap -> RebuildChunks();
        /*for ( int i = 0; i < MainMap -> GetX(); i++ ) {
            for ( int j = 0; j < MainMap -> GetY(); j++ ) {
                for ( int k = 0; k < MainMap -> GetZ(); k++ ) {
                    int id = MainMap -> GetID( i, j, k );
                    if ( ( id < 0 ) || ( id >= 256 ) ) {
                        printf( "ERROR: id = %d at ( %d %d %d )\n", id, i, j, k );
                    }
                    if ( id == 1 ) {
                        printf( "id = %d at ( %d %d %d )\n", id, i, j, k );
                    }
                }
            }
        }*/
        //UpdateChunks();
        MainMap -> RepairMap();
    }
    // out_init
    if ( weaponSpawnStack ) {
        delete weaponSpawnStack;
    }
    weaponSpawnStack = new WeaponSpawnStack();
    weapons_enabled = true;
    MainMapFiller -> Reset();
    level_Initialized = true;
}

void Read_Init( string path ) {
    FILE* handle = fopen( path.c_str(), "r" );
    if ( handle ) {
        fscanf( handle, "%d\n", &level_presented );
        fscanf( handle, "%d %d\n", &std_Texture_Width, &std_Texture_Height );
        currentLevel = level_presented;
        fclose( handle );
    }
}

#define PLAYER_STD_LOOK_OFFSET 0.25
#define PLAYER_STD_HEIGHT_OFFSET 0.1
#define PLAYER_COLLISION_POINT_COUNT 12
Point3D PLAYER_COLLISION_ARRAY[ PLAYER_COLLISION_POINT_COUNT ] = {
    Point( -PLAYER_STD_LOOK_OFFSET, 0.0, -PLAYER_STD_LOOK_OFFSET ),
    Point( -PLAYER_STD_LOOK_OFFSET, 0.0, PLAYER_STD_LOOK_OFFSET ),
    Point( PLAYER_STD_LOOK_OFFSET, 0.0, PLAYER_STD_LOOK_OFFSET ),
    Point( PLAYER_STD_LOOK_OFFSET, 0.0, -PLAYER_STD_LOOK_OFFSET ),
    Point( -PLAYER_STD_LOOK_OFFSET, pHeight + PLAYER_STD_HEIGHT_OFFSET, -PLAYER_STD_LOOK_OFFSET ),
    Point( -PLAYER_STD_LOOK_OFFSET, pHeight + PLAYER_STD_HEIGHT_OFFSET, PLAYER_STD_LOOK_OFFSET ),
    Point( PLAYER_STD_LOOK_OFFSET, pHeight + PLAYER_STD_HEIGHT_OFFSET, PLAYER_STD_LOOK_OFFSET ),
    Point( PLAYER_STD_LOOK_OFFSET, pHeight + PLAYER_STD_HEIGHT_OFFSET, -PLAYER_STD_LOOK_OFFSET ),
    Point( -PLAYER_STD_LOOK_OFFSET, pHeight / 2.0, -PLAYER_STD_LOOK_OFFSET ),
    Point( -PLAYER_STD_LOOK_OFFSET, pHeight / 2.0, PLAYER_STD_LOOK_OFFSET ),
    Point( PLAYER_STD_LOOK_OFFSET, pHeight / 2.0, PLAYER_STD_LOOK_OFFSET ),
    Point( PLAYER_STD_LOOK_OFFSET, pHeight / 2.0, -PLAYER_STD_LOOK_OFFSET )
};

VorbisRadio* v_radio;

void Core_Init() {
    //glutSetKeyRepeat( GLUT_KEY_REPEAT_OFF ); // well :)
    //printf( "    > Basic loading...\n" );
    glutIgnoreKeyRepeat( 1 );
    //printf( "Initialization...\n" );
    CheatInit();
    Effect_Init();
    Read_Init( "data/option.dat" );
    timerShoot = new Timer( SHOOT_INTERVAL );
    InitModels(); // /// WATCH OUT - MEMORY CORRUPTION HERE
    LoadObjects( "data/level/object.slf" );
    player = new Entity( mapX / 2, 12.0, mapZ / 2, PLAYER_COLLISION_POINT_COUNT, object[ PLAYER_OBJECT_ID ] );
    /*player -> RegisterOffset( 0, 0.0, 0.0, 0.0 ); // feet
    player -> RegisterOffset( 1, 0.0, pHeight + PLAYER_STD_LOOK_OFFSET, 0.0 ); // head*/
    for ( int i = 0; i < PLAYER_COLLISION_POINT_COUNT; i++ ) {
        player -> RegisterOffset( i, PLAYER_COLLISION_ARRAY[ i ] );
    }
    //printf( "    > Camera setup...\n" );
    mainCamHandle = new Camera( player, 0.0, 0.0 );
    mainCamHandle -> LinkAnxietyPointer( &___AnxietyExternLevel );
    mainCamQuaker = new CamQuaker( &mainCamHandle );
    glutSetCursor( GLUT_CURSOR_NONE );
    glEnable( GL_DEPTH_TEST );
    //printf( "    > Textures loading...\n" );
    StringLinker* textureSL = new StringLinker( "data/textures/textures.slf" );
    for ( int i = 0; i < MAX_TEXTURES; i++ ) {
        image[ i ] = NULL;
    }
    for ( int i = 0; i < textureSL -> GetSize(); i++ ) {
        //printf( "TX %d: %s\n", textureSL -> GetIdOf( i ), textureSL -> GetStringOf( i ).c_str() );
        image[ textureSL -> GetIdOf( i ) ] = new Bitmap( textureSL -> GetStringOf( i ).c_str() );
    }
    delete textureSL;
    for ( int i = 0; i < MAX_TEXTURES; i++ ) {
        if ( image[ i ] ) {
            //printf( "   i = %d, ", i );
            image[ i ] -> GL_Bitmap();
        }
    }
    //image[ 1 ] -> Debug_Info();
    // dark fog
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glFogfv( GL_FOG_COLOR, FogColor );
    glFogf( GL_FOG_DENSITY, 1.0 );
    glEnable( GL_FOG );
    //image[ 1 ] -> Debug_Info();
    // GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    /*GLfloat mat_specular[] = { 0.3, 0.3, 0.3, 1.0 };
    GLfloat mat_emission[] = { 0.3, 0.3, 0.3, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular );
    glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, mat_emission );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess );*/
    glEnable( GL_LIGHTING );
    //image[ 1 ] -> Debug_Info();
    playerLighter = new Lighter( 0 );
    // audio
    InitAudio();
    InitAudio_Player();
    //LoopSound( CHANNEL_BACKGROUND, "Village1.mp3" );
    // tmp test block
    block = new Block( cubeBlockSize, cubeBlockSize, cubeBlockSize );
    // models
    //image[ 1 ] -> Debug_Info();
    // blending // useful?
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    LoadForbidden( "data/textures/forbidden.dat" );
    //printf( "    > Weapons loading...\n" );
    InitWeapons( "data/stashes/Weapon.slf", "data/stashes/Ammo.slf" );
    // visiblityRange = ( int )( FOG_END_DIST + 2.0 ); // unused
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
    // glShadeModel( GL_SMOOTH );
    // face culling
    glFrontFace( GL_CW );
    glCullFace( GL_FRONT );
    //glEnable( GL_NORMALIZE );
    //image[ 1 ] -> Debug_Info();
    //printf( "Game initialized.\n" );
    //Level_Init(); /// auto state changing!
    //image[ 1 ] -> Debug_Info();
    //printf( "    > Radio initialization...\n" );
    v_radio = new VorbisRadio();
    //printf( "    > Sound init...\n" );
    LoopSound( CHANNEL_PAIN, "player/ConstantPain.ogg" );//"player/ConstantPain.mp3" );
    SetVolume( CHANNEL_PAIN, 0.0 );
    SetVolume( CHANNEL_HIT, 1.0 );
    InitBlockProperties();
    // materials
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_COLOR_MATERIAL );
}

void CleanSpawnStacks() {
    if ( !MainMap ) {
        return;
    }
    if ( ammoSpawnStack ) {
        ammoSpawnStack -> Save( MainMap -> GetMapPath() );
        ammoSpawnStack -> Flush();
    }
    if ( weaponSpawnStack ) {
        weaponSpawnStack -> Save( MainMap -> GetMapPath() );
        weaponSpawnStack -> Flush();
    }
}

void Core_Quit() {
    // test save
    //SaveGameState( GAME_SAVE_PATH );
    CleanSpawnStacks();
    Level_CleanUp();
    RemoveWeapons();
    /*for ( int i = 0; i < MAX_TEXTURES; i++ ) {
        if ( image[ i ] ) {
            delete image[ i ];
        }
    }*/
    delete v_radio;
    DeleteAudio_Player();
    RemoveForbidden();
    DeleteAudio();
    RemoveMainInterpreter();
    DestroyBlockProperties();
}

void Core_ForceLevel( int level ) {
    // cleanup old level
    // delete MainMap; // deleted during level init
    CleanSpawnStacks();
    CleanAudio();
    for ( int i = 0; i < MAX_MONSTERS; i++ ) {
        if ( monsterStorage[ i ] ) {
            delete monsterStorage[ i ];
            monsterStorage[ i ] = NULL;
        }
    }
    // load new level
    if ( level >= 0 ) {
        currentLevel = level;
    }
    Level_Init();
    editMode = 0;
}

double invertMouse = 1.0;

int lastMouseX = SCREENX / 2;
int lastMouseY = SCREENY / 2;
int wX = SCREENX;
int wY = SCREENY;

void SetMouse( int newX, int newY ) {
    lastMouseX = newX;
    lastMouseY = newY;
}

bool initDone = false;

#define S_TABLE_MOUSE_NUM 24
double mSensTable[ S_TABLE_MOUSE_NUM ][ 2 ] = {
    { 0.04, 0.04 }, { 0.07, 0.07 }, { 0.09, 0.09 }, { 0.13, 0.13 }, // ultra slow - dead zombie ranges
    { 0.22, 0.22 }, { 0.33, 0.33 }, { 0.4, 0.4 }, { 0.525, 0.525 }, // below normal - *yawn* ranges
    { 0.75, 0.75 }, { 1.0, 1.0 }, { 1.25, 1.25 }, { 1.5, 1.5 }, // normal - usual ranges
    { 1.75, 1.75 }, { 2.0, 2.0 }, { 2.25, 2.25 }, { 2.6, 2.6 }, // above normal - fast-and-precise-gamers ranges
    { 3.25, 3.25 }, { 4.0, 4.0 }, { 4.85, 4.85 }, { 6.0, 6.0 }, // very fast - "DAFUQ IS WITH MOUSE" ranges
    { 7.0, 7.0 }, { 8.0, 8.0 }, { 9.1, 9.1 }, { 10.5, 10.5 } // you really don't wanna aim with these.
};

bool MousePressed[ 2 ];

int mSensitivityI = 9;
int mSensitivitySTDI = 9;
double mSensScopeMul = 0.75;

void Uni_Motion( int mouseX, int mouseY ) {
    lastMouseX = mouseX;
    lastMouseY = mouseY;
	int ofsRectX = wX / 4;
	int ofsRectY = wY / 4;
    int screenLeft = ofsRectX;
	int screenTop = ofsRectY;
	int screenRight = wX - ofsRectX;
	int screenBottom = wY - ofsRectY;
	if ( mouseX <= screenLeft || mouseY <= screenTop || mouseX >= screenRight || mouseY >= screenBottom ) {
		lastMouseX = wX / 2;
        lastMouseY = wY / 2;
		glutWarpPointer( lastMouseX, lastMouseY );
	}
}

void Core_Motion( int mouseX, int mouseY ) {
    double difX = double( mouseX - lastMouseX ) * 0.0035 * mSensTable[ mSensitivityI ][ 0 ];//mSensitivityX;
    double difY = double( mouseY - lastMouseY ) * 0.0035 * mSensTable[ mSensitivityI ][ 1 ] * invertMouse;//mSensitivityY * invertMouse;
    if ( MousePressed[ 1 ] ) {
        difX *= mSensScopeMul;
        difY *= mSensScopeMul;
    }
    Uni_Motion( mouseX, mouseY );
    if ( initDone && mainCamHandle ) {
        mainCamHandle -> pitchAngle += difX;
        double aDGR = degr( mainCamHandle -> pitchAngle );
        if ( aDGR > 360.0 ) {
            mainCamHandle -> pitchAngle -= 2 * PI;
        } else if ( aDGR < 0.0 ) {
            mainCamHandle -> pitchAngle += 2 * PI;
        }
        mainCamHandle -> yawAngle += difY;
    } else {
        initDone = true;
    }
    if ( mainCamHandle ) {
        mainCamHandle -> CalcLookAngle();
    }
}

int blockID = 1;

void SwitchBlock( int direction ) {
    do {
        blockID = ( blockID + direction ) % MAX_TEXTURES;
        if ( blockID < 0 ) {
            blockID = MAX_TEXTURES - 1;
        }
    } while ( ( !image[ blockID ] ) || ( ForbiddenID( blockID ) ) );
}

#define CURSOR_EDIT_RANGE 7.0

#define MAX_STRICT_MODES 5
#define STRICT_EDIT 0
#define POLYGON_MODE 1
#define PLANTER_MODE 2
#define AMMO_SPAWN_MODE 3
#define WEAPON_SPAWN_MODE 4

double _CURSOR_COLOR_MODE[ MAX_STRICT_MODES ][ 3 ] = { { 1.0, 1.0, 1.0 }, { 0.0, 1.0, 1.0 }, { 0.0, 1.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 1.0, 0.75, 0.0 } };
string _STRICT_MODE_NAME[ MAX_STRICT_MODES ] = { "Edit mode", "Polygon mode", "Planter mode", "Ammo spawn", "Weapon spawn" };

Point3D pointedBlock = Point( -1.0, -1.0, -1.0 ); // initializer
Point3D pointedSide = pointedBlock;
int editStrictMode = STRICT_EDIT;
double FOV_Timer = 0.0;
double FOV_JOIN_TIME = 0.0;

#define TREE_MIN_SIZE 5
#define TREE_MAX_SIZE 8

#define WOOD_BLOCK 6
#define LEAF_BLOCK 7

class PlantStructure {
    public:
    PlantStructure() {
        dim = Point( 0.0, 0.0, 0.0 );
        data = NULL;
    }
    ~PlantStructure() {
        if ( data ) {
            free( data );
        }
    }
    void Deploy( Point3D root, bool aggressive ) {
        int midx = dim.x / 2.0;
        int midz = dim.z / 2.0;
        for ( int iy = 0; iy < dim.y; iy++ ) {
            for ( int iz = 0; iz < dim.z; iz++ ) {
                for ( int ix = 0; ix < dim.x; ix++ ) {
                    if ( ( MainMap -> GetID( root.x + ix - midx, root.y - iy, root.z + iz - midz ) == 0 ) || ( aggressive ) ) {
                        MainMap -> SetID( root.x + ix - midx, root.y - iy, root.z + iz - midz, data[ Calc_Index( ix, iy, iz ) ] * LEAF_BLOCK );
                        MainMap -> UpdateSides( root.x + ix - midx, root.y - iy, root.z + iz - midz );
                    }
                }
            }
        }
    }
    int Calc_Index( int x, int y, int z ) {
        return z * dim.y * dim.x + y * dim.x + x;
    }
    static PlantStructure* Load( string path ) {
        FILE* handle = fopen( path.c_str(), "rb" );
        if ( handle ) {
            PlantStructure* ret = new PlantStructure();
            int rx, ry, rz = 0;
            fscanf( handle, "%d %d %d\n\n", &rx, &ry, &rz );
            ret -> dim = Point( rx, ry, rz );
            ret -> data = ( char* )( malloc( rx * ry * rz ) );
            for ( int iy = 0; iy < ry; iy++ ) {
                for ( int iz = 0; iz < rz; iz++ ) {
                    char buffer[ 256 ];
                    fgets( buffer, 256, handle );
                    for ( int ix = 0; ix < rx; ix++ ) {
                        //printf( "%c", buffer[ ix ] );
                        ret -> data[ ret -> Calc_Index( ix, iy, iz ) ] = buffer[ ix ] - '0';
                    }
                    //printf( "\n" );
                }
                //printf( "\n" );
                fscanf( handle, "\n" );
            }
            fclose( handle );
            return ret;
        }
        return NULL;
    }
    private:
    Point3D dim;
    char* data;
};

PlantStructure* basicPlant = PlantStructure::Load( "planter.txt" );

void Plant( Point3D root, PlantStructure* structure = basicPlant, bool aggresivePlanting = false ) {
    if ( structure ) {
        int h = rand() % ( TREE_MAX_SIZE - TREE_MIN_SIZE + 1 ) + TREE_MIN_SIZE;
        structure -> Deploy( Point( root.x, root.y + h + 1, root.z ), aggresivePlanting );
        for ( int i = 0; i < h; i++ ) {
            Point3D p = AddPoint( root, Point( 0.0, i, 0.0 ) );
            if ( ( MainMap -> GetID( p.x, p.y, p.z ) == 0 ) || ( aggresivePlanting ) ) {
                MainMap -> SetID( p.x, p.y, p.z, WOOD_BLOCK );
                MainMap -> UpdateSides( p.x, p.y, p.z );
            }
        }
        MainMap -> RebuildChunks();
    }
}

int fogEnabled = 1;
int oldVRange = 0;
int spawnAmmoID = 0;
int spawnWeaponID = 0;
bool showBillBoards = true;

#define STD_SPAWNAMMO_REMOVE_RADIUS 1.0
#define STD_SPAWNWEAPON_REMOVE_RADIUS 1.0
#define STD_SPAWN_INTERVAL 30000.0

void Core_MousePressed( int button, int state, int x, int y ) {
    if ( editMode ) { // when editing
        if ( state == GLUT_DOWN ) {
            if ( button == GLUT_LEFT_BUTTON ) {
                /*if ( polygonMode ) {
                    MainMapFiller -> Finalize_Wall_Rect( blockID );
                } else {
                    MainMap -> SetID( pointedBlock.x, pointedBlock.y, pointedBlock.z, 0 );
                    RecursiveSideUpdate( pointedBlock.x, pointedBlock.y, pointedBlock.z );
                }*/
                switch ( editStrictMode ) {
                    case STRICT_EDIT: {
                        MainMap -> SetID( pointedBlock.x, pointedBlock.y, pointedBlock.z, 0 );
                        RecursiveSideUpdate( pointedBlock.x, pointedBlock.y, pointedBlock.z );
                        break;
                    }
                    case POLYGON_MODE: {
                        MainMapFiller -> Finalize_Wall_Rect( blockID );
                        break;
                    }
                    case PLANTER_MODE: {
                        MainMap -> RepairMap();
                        break;
                    }
                    case AMMO_SPAWN_MODE: {
                        Point3D placement = AddPoint( Floor( pointedSide ), Point( 0.5, 0.5, 0.5 ) );
                        ammoSpawnStack -> TryRemove( placement, STD_SPAWNAMMO_REMOVE_RADIUS );
                        break;
                    }
                    case WEAPON_SPAWN_MODE: {
                        Point3D placement = AddPoint( Floor( pointedSide ), Point( 0.5, 0.5, 0.5 ) );
                        weaponSpawnStack -> TryRemove( placement, STD_SPAWNWEAPON_REMOVE_RADIUS );
                        break;
                    }
                }
            } else if ( button == GLUT_RIGHT_BUTTON ) {
                switch ( editStrictMode ) {
                    case STRICT_EDIT: {
                        MainMap -> SetID( pointedSide.x, pointedSide.y, pointedSide.z, blockID );
                        RecursiveSideUpdate( pointedSide.x, pointedSide.y, pointedSide.z );
                        break;
                    }
                    case POLYGON_MODE: {
                        MainMapFiller -> Select( pointedSide );
                        break;
                    }
                    case PLANTER_MODE: {
                        Plant( pointedSide );
                        break;
                    }
                    case AMMO_SPAWN_MODE: {
                        Point3D placement = AddPoint( Floor( pointedSide ), Point( 0.5, 0.5, 0.5 ) );
                        if ( ammoSpawnStack -> NearestDist( placement ) > 0.1 ) {
                            ammoSpawnStack -> Add( new AmmoSpawn( ammoStash -> GetAmmoByIndex( spawnAmmoID ), placement.x, placement.y, placement.z, STD_SPAWN_INTERVAL ) );
                        }
                        break;
                    }
                    case WEAPON_SPAWN_MODE: {
                        Point3D placement = AddPoint( Floor( pointedSide ), Point( 0.5, 0.5, 0.5 ) );
                        weaponSpawnStack -> Add( new WeaponSpawn( weaponStash -> GetWeapon( spawnWeaponID ), placement ) );
                        break;
                    }
                }
            } else if ( button == GLUT_MIDDLE_BUTTON ) {
                switch ( editStrictMode ) {
                    case STRICT_EDIT: {
                        int pointedID = MainMap -> GetID( pointedBlock.x, pointedBlock.y, pointedBlock.z );
                        if ( pointedID > 0 ) {
                            blockID = pointedID;
                        }
                        break;
                    }
                    case POLYGON_MODE: {
                        MainMapFiller -> Undo();
                        break;
                    }
                    case PLANTER_MODE: {
                        break;
                    }
                    case AMMO_SPAWN_MODE: {
                        break;
                    }
                    case WEAPON_SPAWN_MODE: {
                        break;
                    }
                }
            }
        }
    } else { // when ingame
        if ( state == GLUT_DOWN ) {
            if ( button == GLUT_LEFT_BUTTON ) {
                MousePressed[ 0 ] = true;
            }
            if ( button == GLUT_RIGHT_BUTTON ) {
                MousePressed[ 1 ] = true;
                FOV_Timer = Timer::Current();
            }
        } else {
            if ( button == GLUT_LEFT_BUTTON ) {
                MousePressed[ 0 ] = false;
            }
            if ( button == GLUT_RIGHT_BUTTON ) {
                MousePressed[ 1 ] = false;
                FOV_Timer = 2.0 * Timer::Current() - FOV_Timer - FOV_JOIN_TIME;
            }
        }
    }
}

/*
void Core_MouseMotion( int mouseX, int mouseY ) {
    Core_Motion( mouseX, mouseY );
}
*/

#define KEY_NUMBER 256
bool PrevKeyPressed[ KEY_NUMBER ];
bool KeyPressed[ KEY_NUMBER ];
bool ShiftKey = false;

bool KeyStruck( unsigned char key ) {
    if ( KeyPressed[ key ] && !PrevKeyPressed[ key ] ) {
        return true;
    }
    return false;
}

bool KeyOut( unsigned char key ) {
    if ( !KeyPressed[ key ] && PrevKeyPressed[ key ] ) {
        return true;
    }
    return false;
}

void Core_InputProcessOnce() {
    if ( KeyStruck( 27 ) ) {
        switch ( MIND_STATE ) {
            case STATE_GAME: {
                MIND_STATE = STATE_ON_PAUSE_GAME;
                break;
            }
            case STATE_MENU: {
                if ( level_Initialized ) {
                    MIND_STATE = STATE_ON_RESUME_GAME;
                }
                break;
            }
        }
    } else if ( KeyStruck( 13 ) ) {
        MIND_STATE = STATE_QUIT;
    }
}

void Core_SpecialInput( int key, int x, int y ) {
    switch ( key ) {
        case GLUT_KEY_F1: {
            if ( editMode ) {
                ammoSpawnStack -> Save( MainMap -> GetMapPath() );
                MainMap -> SaveMap();
            }
            break;
        }
        case GLUT_KEY_F2: {
            if ( editMode ) {
                //Point3D ppos = player -> HotSpot();
                //ammoSpawnStack -> Add( new AmmoSpawn( ammoStash -> GetAmmoByIndex( spawnAmmoID ), ppos.x, ppos.y + 0.5, ppos.z, STD_SPAWN_INTERVAL ) );
            }
            break;
        }
        case GLUT_KEY_F3: {
            //MIND_STATE = ( MIND_STATE + 1 ) % 3;
            //if ( editMode ) {
                /*Point3D ppos = player -> HotSpot();
                ppos.y += 0.5;
                ammoSpawnStack -> TryRemove( ppos, STD_SPAWNAMMO_REMOVE_RADIUS );*/
            //}
            break;
        }
        case GLUT_KEY_F4: {
            int BPP = 3;
            GLbyte* bytes = ( GLbyte* )( calloc( 1, wX * wY * BPP ) );
            if ( bytes ) {
                glReadPixels( 0, 0, wX, wY, GL_BGR, GL_UNSIGNED_BYTE, bytes );
                Bitmap* bitmap = new Bitmap();
                bitmap -> SetRaw( bytes );
                bitmap -> SetDim_Const( wX, wY, BPP );
                bitmap -> SaveBitmap( "screenshot/" );
                //free( bytes );
                delete bitmap;
            }
            break;
        }
        case GLUT_KEY_F5: {
            if ( editMode ) {
                editStrictMode--;
                if ( editStrictMode < 0 ) {
                    editStrictMode += MAX_STRICT_MODES;
                }
            }
            break;
        }
        case GLUT_KEY_F6: {
            if ( editMode ) {
                editStrictMode++;
                if ( editStrictMode >= MAX_STRICT_MODES ) {
                    editStrictMode -= MAX_STRICT_MODES;
                }
            }
            break;
        }
        case GLUT_KEY_F7: {
            if ( editMode ) {
                showBillBoards = !showBillBoards;
            }
            break;
        }
        case GLUT_KEY_F8: {
            if ( editMode ) {
                playerSpeed_EditMode_ArrayID = 1 - playerSpeed_EditMode_ArrayID;
            }
            break;
        }
        case GLUT_KEY_F9: {
            mainCamQuaker -> SetQuake( 2.0, 4.0 );
            /*if ( foundRadio > 0 ) {
                if ( radio -> Paused() ) {
                    PauseSound( CHANNEL_BACKGROUND );
                } else {
                    ResumeSound( CHANNEL_BACKGROUND );
                }
                pauseRadio = true;
            }*/
            break;
        }
        case GLUT_KEY_F10: {
            /*if ( foundRadio > 0 ) {
                changeRadio = ( rand() % foundRadio );
            }*/
            break;
        }
        case GLUT_KEY_F11: {
            //changeRadio = -1;
            break;
        }
        case GLUT_KEY_F12: {
            //changeRadio = 1;
            break;
        }
    }
}

//double ax = 0.0;
//double ay = 0.0;

Weapon* cWeapon = NULL;

void ScopeFOVRecalc() {
    Weapon* cw = NULL;
    if ( weaponStash ) {
        cw = weaponStash -> GetWeapon( currentWeaponKey );
    }
    if ( ( cw ) && ( cWeapon ) ) {
        double R1 = Timer::Current() - FOV_Timer;
        double T1 = cWeapon -> GetVar( WEAPON_VAR_SCOPINGTIME );
        double T2 = cw -> GetVar( WEAPON_VAR_SCOPINGTIME );
        double R2 = R1 * T2 / T1;
        FOV_Timer -= ( R2 - R1 );
    }
}

bool playerLighterEnabled = true;
bool edit_Light = false;

void Core_Keystroke( unsigned char key, int x, int y ) {
    KeyPressed[ key ] = true;
    // instant reaction
    //Core_InputProcessOnce();
    switch ( key ) {
        case 27: { // ESCAPE
            if ( MainMap ) {
                if ( ( MIND_STATE == STATE_GAME ) || ( MIND_STATE == STATE_GAME_MP ) ) {
                    MIND_STATE = STATE_ON_PAUSE_GAME;
                } else if ( MIND_STATE == STATE_MENU ) {
                    MIND_STATE = STATE_ON_RESUME_GAME;
                }
            }
            break;
        }
        case 13: { // RETURN
            //MIND_STATE = STATE_QUIT;
            break;
        }
        case 'q': {
            if ( editMode ) {
                if ( editStrictMode == WEAPON_SPAWN_MODE ) {
                    spawnWeaponID--;
                    if ( spawnWeaponID < 0 ) {
                        spawnWeaponID = weaponStash -> Quantity() - 1;
                    }
                } else if ( editStrictMode == AMMO_SPAWN_MODE ) {
                    spawnAmmoID--;
                    if ( spawnAmmoID < 0 ) {
                        spawnAmmoID = ammoStash -> Quantity() - 1;
                    }
                } else if ( editStrictMode == STRICT_EDIT ) {
                    SwitchBlock( -1 );
                }
            } else {
                PrevWeapon();
                ScopeFOVRecalc();
            }
            break;
        }
        case 'e': {
            if ( editMode ) {
                if ( editStrictMode == WEAPON_SPAWN_MODE ) {
                    spawnWeaponID++;
                    if ( spawnWeaponID >= weaponStash -> Quantity() ) {
                        spawnWeaponID = 0;
                    }
                } else if ( editStrictMode == AMMO_SPAWN_MODE ) {
                    spawnAmmoID++;
                    if ( spawnAmmoID >= ammoStash -> Quantity() ) {
                        spawnAmmoID = 0;
                    }
                } else if ( editStrictMode == STRICT_EDIT ) {
                    SwitchBlock( 1 );
                }
            } else {
                NextWeapon();
                ScopeFOVRecalc();
            }
            break;
        }
        case 'f': {
            playerLighterEnabled = !playerLighterEnabled;
            if ( playerLighterEnabled ) {
                glEnable( GL_LIGHT0 );
            } else {
                glDisable( GL_LIGHT0 );
            }
            break;
        }
        case 'F': {
            if ( editMode ) {
                fogEnabled = ( fogEnabled + 1 ) & 0x01;
                if ( !fogEnabled ) {
                    oldVRange = visiblityRange;
                    visiblityRange = EDITMODE_VISIBLITY_RANGE;
                } else {
                    visiblityRange = oldVRange;
                }
            }
            break;
        }
        case 'g': {
            if ( editMode ) {
                edit_Light = !edit_Light;
            }
            break;
        }
        case '<': {
            chunk_Range--;
            if ( chunk_Range < CHUNK_RANGE_MIN ) {
                chunk_Range = CHUNK_RANGE_MAX;
            }
            break;
        }
        case '>': {
            chunk_Range++;
            if ( chunk_Range > CHUNK_RANGE_MAX ) {
                chunk_Range = CHUNK_RANGE_MIN;
            }
            break;
        }
        case 9: { // TAB
            invertMouse = -invertMouse;
            break;
        }
        case 127: { // DELETE
            if ( editMode ) {
                Point3D pos = player -> HotSpot();
                MainMap -> UpdateSpawn( pos.x, pos.y, pos.z, mainCamHandle -> pitchAngle, mainCamHandle -> yawAngle );
            }
            break;
        }
        case 8: { // BACKSPACE
            if ( editMode ) {
                editMark = player -> FixedPlayerSpot();
            }
            break;
        }
        case '/': {
            editMode = !editMode;
            break;
        }
        case '+': {
            mSensitivityI = abs( ( mSensitivityI + 1 ) % S_TABLE_MOUSE_NUM );
            break;
        }
        case '-': {
            mSensitivityI--;
            if ( mSensitivityI < 0 ) {
                mSensitivityI = S_TABLE_MOUSE_NUM - 1;
            }
            break;
        }
        case '*': {
            mSensitivityI = mSensitivitySTDI;
            break;
        }
        case '1': { // debug
            //glEnable( GL_LIGHTING );
            break;
        }
        case '2': { // debug
            //glDisable( GL_LIGHTING );
            break;
        }
        // test
        case '3': {
            // DRIVETO is correct!
            //mainCamHandle -> DriveTo( Point( PI, PI / 4.0, 0.0 ), 0.18 );
            static int cnum = 0;
            printf( "ATT %d\n", cnum );
            for ( int i = 0; i < MAX_MONSTERS; i++ ) {
                if ( monsterStorage[ i ] ) {
                    double d, ei, ea;
                    Point3D p = player -> FixedPlayerSpot();
                    Point3D e = monsterStorage[ i ] -> HotSpot();
                    cts( e.x - p.x, p.y - e.y, p.z - e.z, &d, &ea, &ei );
                    ei += DEGR90;
                    ea -= DEGR90;
                    ei = ei - mainCamHandle -> pitchAngle;
                    ea = ea - mainCamHandle -> yawAngle;
                    printf( "   > monster[ %d ]:\n", i );
                    printf( "   > angledif = %g, %g\n", degr( ei ), degr( ea ) );
                }
            }
            break;
        }
    }
    ShiftKey = ( glutGetModifiers() == GLUT_ACTIVE_SHIFT );
    cheatSystem -> AppendChar( key );
}

void Core_KeyUp( unsigned char key, int x, int y ) {
    KeyPressed[ key ] = false;
    ShiftKey = ( glutGetModifiers() == GLUT_ACTIVE_SHIFT );
}

void Move( Entity* e, Camera* cam, float front, float side ) {
    if ( e && cam ) {
        double xSpeed = sin( cam -> pitchAngle );
        double zSpeed = - cos( cam -> pitchAngle );
        Point3D epos = e -> HotSpot();
        epos.x += ( xSpeed * front - zSpeed * side );
        epos.z += ( zSpeed * front + xSpeed * side );
        e -> UpdateHotSpot( epos );
    }
}

void Move3D( Point3D* point, Camera* cam, double front, double side ) {
    double xSpeed = sin( cam -> pitchAngle );
    double zSpeed = - cos( cam -> pitchAngle );
    if ( ( level_Gravity <= 0.2 ) && ( !editMode ) ) {
        double ySpeed = sin( cam -> yawAngle );
        point -> y += ( ySpeed * front );
        double scaleXZSpeed = cos( cam -> yawAngle );
        front *= scaleXZSpeed;
        //zSpeed *= scaleXZSpeed;
    }
    point -> x += ( xSpeed * front - zSpeed * side );
    point -> z += ( zSpeed * front + xSpeed * side );
}

void Move3D( Point3D* point, Point3D dir, double scale ) {
    ScalePoint3D( &dir, scale );
    ( *point ) = AddPoint( ( *point ), dir );
}

bool HasMovedOnGround( Point3D pos, Point3D vecMove ) {
    Point3D nextPos = AddPoint( pos, vecMove );
    if ( MainMap ) {
        if ( MainMap -> GetID( nextPos.x, nextPos.y, nextPos.z ) > 0 ) {
            return true;
        }
    }
    return false;
}

void SpawnAuto( double spawnDist ) {
    for ( int sI = 0; sI < MAX_OBJECT; sI++ ) {
        if ( monsterSpawnPending[ sI ] <= 0 ) {
            continue;
        }
        int indexSel = -1;
        for ( int i = 0; i < MAX_MONSTERS; i++ ) {
            if ( !monsterStorage[ i ] ) {
                indexSel = i;
                break;
            }
        }
        if ( indexSel >= 0 ) { // still free place
            double baseAngle = Random( 2.0 * PI ); // all around
            double seekDirMultiplier = ( double )( ( ( rand() & 0x01 ) << 1 ) - 1 ); // { -1, 1 }
            double currentStep = seekDirMultiplier * Random( SpawnAutoAngleStepMin, SpawnAutoAngleStepMax );
            for ( int j = 0; j < trySpawnTimes; j++ ) {
                Point3D spawnPlace = { 0.0, 0.0, 0.0 };
                if ( object[ sI ] ) {
                    Model* model = object[ sI ] -> GetModel();
                    if ( model ) {
                        spawnPlace.y = - model -> GetLowestY() + 0.05;
                    }
                }
                ptc( spawnDist, baseAngle + double( j ) * currentStep, &spawnPlace.x, &spawnPlace.z );
                spawnPlace = AddPoint( player -> HotSpot(), spawnPlace );
                if ( NothingBetween( object[ sI ] -> GetModel(), spawnPlace, player -> HotSpot() ) ) {
                    // spawn new entity
                    monsterStorage[ indexSel ] = new Entity( spawnPlace.x, spawnPlace.y, spawnPlace.z, 1, object[ sI ] );
                    // lower pending entities count
                    monsterSpawnPending[ sI ]--;
                    // finish it!
                    break;
                }
            }
        }
    }
}

/*void MapDisplay( Map* m, double baseX, double baseY, double baseZ ) { // NO MORE LAGS, ALMOST FULLY OPTIMIZED!
    if ( m ) {
        Point3D cam = player -> HotSpot();
        glEnable( GL_TEXTURE_2D );
        glColor3f( MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
        glPushMatrix();
        for ( int iz = cam.z - visiblityRange; iz < cam.z + visiblityRange; iz++ ) {
            for ( int iy = cam.y - visiblityRange; iy < cam.y + visiblityRange; iy++ ) {
                for ( int ix = cam.x - visiblityRange; ix < cam.x + visiblityRange; ix++ ) {
                    m -> Display( ix, iy, iz );
                }
            }
        }
        glPopMatrix();
    }
}*/

void SkyDisplay( double SkyHeight, double Horizon ) {
    if ( image[ skyTex ] ) {
        if ( image[ skyTex ] -> UseImage() ) {
            Point3D playerPos = player -> HotSpot();
            Sky( playerPos.x, playerPos.y + pHeight, playerPos.z, SkyHeight, Horizon, 1 );
        }
    }
}

double FOV = 70.0;
double FOV_NEAR_DEATH = 107.5;
double ASPECT_NEAR_DEATH_AMP = 0.32;
double ASPECT_PULSE_SPEED = 0.375;
double FOV_PULSE_AMPLITUDE = 0.0;
double FOV_PULSE_SPEED = 0.1667;
double COLOR_DRIVE_INERTIA = 0.992;

double FOV_LIFE_LEVEL_THRESHOLD = 0.5;
double ASPECT_LIFE_LEVEL_THRESHOLD = 0.42;
double COLOR_LIFE_LEVEL_THRESHOLD_MIN = 0.2;
double COLOR_LIFE_LEVEL_THRESHOLD_MAX = 0.6;

double PointPlacementMultiplier( double x, double a, double b ) {
    if ( x <= a ) {
        return 0.0;
    }
    if ( x >= b ) {
        return 1.0;
    }
    return ( x - a ) / ( b - a );
}

void DefaultViewCam() {
    double playerPercentageHP = player -> GetVar( VAR_HP ) / player -> GetVar( VAR_MAX_HP );
    if ( wY > 0 ) {
        double T = Timer::Current();
        double fCalc = FOV_PULSE_AMPLITUDE * sin( T * FOV_PULSE_SPEED );
        double fovDeath = 0.0;
        if ( playerPercentageHP < FOV_LIFE_LEVEL_THRESHOLD ) {
            fovDeath = ( FOV_NEAR_DEATH - FOV ) * ( 1.0 - playerPercentageHP / FOV_LIFE_LEVEL_THRESHOLD );
        }
        double aspectChange = 1.0;
        if ( playerPercentageHP < ASPECT_LIFE_LEVEL_THRESHOLD ) {
            aspectChange += sin( T / ASPECT_PULSE_SPEED ) * ASPECT_NEAR_DEATH_AMP * ( 1.0 - playerPercentageHP / ASPECT_LIFE_LEVEL_THRESHOLD );
        }
        gluPerspective( FOV + fCalc + fovDeath - FOV_SCOPE + FOV_ANXIETY_LEVEL, double( wX ) / double ( wY ) * aspectChange, 0.1, MAX_DRAW_DIST );
    }
    SetDeathColorLevel( PointPlacementMultiplier( playerPercentageHP, COLOR_LIFE_LEVEL_THRESHOLD_MIN, COLOR_LIFE_LEVEL_THRESHOLD_MAX ) );
    DriveMainColor( COLOR_DRIVE_INERTIA );
}

Point3D drunkPoint = { 0.0, 0.0, 0.0 };
Point3D stdUp = { 0.0, 1.0, 0.0 };

void Drunk( double power ) {
    drunkPoint.x = DEGR90 * power * sin( Timer::Current() * 0.65 );
    drunkPoint.z = DEGR90 * power * cos( Timer::Current() * 0.65 );
}

double Drunk_Roll( double power ) {
    return power * sin( Timer::Current() * 0.65 );
}

const double PAIN_LIFE_LEVEL_THRESHOLD = 0.75;
const double HEADACHE_LIFE_LEVEL_THRESHOLD = 0.4;
double ___apl_mul = 1.0;

void AutoKillCam() {
    double playerPercentageHP = player -> GetVar( VAR_HP ) / player -> GetVar( VAR_MAX_HP );
    if ( playerPercentageHP < PAIN_LIFE_LEVEL_THRESHOLD ) {
        SetVolume( CHANNEL_PAIN, 1.0 - playerPercentageHP / PAIN_LIFE_LEVEL_THRESHOLD );
    } else {
        SetVolume( CHANNEL_PAIN, 0.0 );
    }
    if ( playerPercentageHP < HEADACHE_LIFE_LEVEL_THRESHOLD ) {
        ROLLAMP = ( 1.0 - playerPercentageHP / HEADACHE_LIFE_LEVEL_THRESHOLD ) * sin( Timer::Current() * 0.65 ) * 0.125;
    }
}

Lighter* overallLighter = NULL;
Lighter* cloudLighter = NULL;
#define LIGHT_OVERALL_ASSIGNED 1
#define LIGHT_CLOUD_ASSIGNED 3
GLfloat MOONLIGHT_POWER = 0.285;
GLfloat EMISSION_POWER = 0.145;
GLfloat SPECULAR_POWER = 0.18;
GLfloat FULL_POWER = 1.0; // almost like white light... power. Racist light.
GLfloat SPECULAR[ 3 ] = { 1.0, 1.0, 1.0 };
GLfloat SHININESS_POWER = 110.0;
GLfloat PLAYER_LIGHTER_POWER = 5.5;

void SceneLight() {
    if ( !overallLighter ) {
        overallLighter = new Lighter( LIGHT_OVERALL_ASSIGNED );
    }
    GLfloat moonlight, emission, specular;
    if ( edit_Light ) {
        moonlight = FULL_POWER;
        specular = FULL_POWER;
        emission = FULL_POWER;
    } else {
        moonlight = MOONLIGHT_POWER;
        specular = SPECULAR_POWER;
        emission = EMISSION_POWER;
    }
    GLfloat shininess = SHININESS_POWER;
    GLfloat mat_specular[] = { SPECULAR[ 0 ] * specular, SPECULAR[ 1 ] * specular, SPECULAR[ 2 ] * specular, 1.0 };
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular );
    GLfloat mat_emission[] = { MainColor[ 0 ] * emission, MainColor[ 1 ] * emission, MainColor[ 2 ] * emission, 1.0 };
    glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, mat_emission );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, &shininess );
    Point3D sceneLight = Point( MainColor[ 0 ] * moonlight, MainColor[ 1 ] * moonlight, MainColor[ 2 ] * moonlight );
    overallLighter -> FlashPrepare( Point( 0.0, 100000.0, 0.0 ), Point( 0.0, -1.0, 0.0 ), sceneLight, 180.0 );
    glLightf( GL_LIGHT0 + LIGHT_OVERALL_ASSIGNED, GL_LINEAR_ATTENUATION, 0.0 );
}

GLfloat MenuColor[ 3 ] = { 0.3, 0.3, 0.3 };

void MenuLight() {
    GLfloat moonlight, emission, specular;
    moonlight = FULL_POWER;
    specular = FULL_POWER;
    emission = FULL_POWER;
    GLfloat shininess = SHININESS_POWER;
    GLfloat mat_specular[] = { SPECULAR[ 0 ] * specular, SPECULAR[ 1 ] * specular, SPECULAR[ 2 ] * specular, 1.0 };
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular );
    GLfloat mat_emission[] = { MenuColor[ 0 ] * emission, MenuColor[ 1 ] * emission, MenuColor[ 2 ] * emission, 1.0 };
    glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION, mat_emission );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SHININESS, &shininess );
    //Point3D sceneLight = Point( MenuColor[ 0 ] * moonlight, MenuColor[ 1 ] * moonlight, MenuColor[ 2 ] * moonlight );
    //overallLighter -> FlashPrepare( Point( 0.0, 100000.0, 0.0 ), Point( 0.0, -1.0, 0.0 ), sceneLight, 180.0 );
    //glLightf( GL_LIGHT0 + LIGHT_OVERALL_ASSIGNED, GL_LINEAR_ATTENUATION, 0.0 );
}

double cloudLightStrenghener = 1.5;

void CloudLight_Begin() {
    if ( !cloudLighter ) {
        cloudLighter = new Lighter( LIGHT_CLOUD_ASSIGNED );
    }
    glEnable( GL_LIGHT0 + LIGHT_CLOUD_ASSIGNED );
    Point3D cloudLight = Point( MainColor[ 0 ] * cloudLightStrenghener, MainColor[ 1 ] * cloudLightStrenghener, MainColor[ 2 ] * cloudLightStrenghener );//Point( MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
    cloudLighter -> FlashPrepare( Point( 0.0, -100000.0, 0.0 ), Point( 0.0, 1.0, 0.0 ), cloudLight, 180.0 );
    glLightf( GL_LIGHT0 + LIGHT_CLOUD_ASSIGNED, GL_LINEAR_ATTENUATION, 0.0 );
}

void CloudLight_End() {
    glDisable( GL_LIGHT0 + LIGHT_CLOUD_ASSIGNED );
}

void ApplyLight( Point3D mainPos, Point3D dirRel ) {
    Point3D lightColor = Point( MainColor[ 0 ] * 0.96 * PLAYER_LIGHTER_POWER, MainColor[ 1 ] * 0.92 * PLAYER_LIGHTER_POWER, MainColor[ 2 ] * 0.65 * PLAYER_LIGHTER_POWER );
    playerLighter -> FlashPrepare( mainPos, dirRel, lightColor, 50.0 );
}

bool light = true;
bool lightOverall = false;

void SwitchLight( bool forceOn = false ) {
    light = !light;
    if ( lightOverall ) {
        if ( ( light ) || ( forceOn ) ) {
            glEnable( GL_LIGHTING );
        } else {
            glDisable( GL_LIGHTING );
        }
    }
}

/*
    FPS buffered counter - test only, doesn't work though
*/

#define FPS_BUFFER 1
int fpsCount[ FPS_BUFFER ];
int lastMeasuredFPS = -1;

double RealFPS() {
    double realCount = 0;
    double count = 0.0;
    for ( int i = 0; i < FPS_BUFFER; i++ ) {
        if ( fpsCount[ i ] >= 0 ) {
            count += fpsCount[ i ];
            realCount++;
        }
    }
    return count / realCount;
}

void FPSCounter() {
    if ( lastMeasuredFPS < 0 ) {
        for ( int i = 0; i < FPS_BUFFER; i++ ) {
            fpsCount[ i ] = -1;
        }
        lastMeasuredFPS = 0;
    }
    static int cfps = 0;
    static double lastFPStimer = 0;
    if ( lastFPStimer < Timer::Current() + 1.0 ) {
        lastFPStimer += 1.0;
        fpsCount[ lastMeasuredFPS ] = cfps;
        cfps = 0;
        lastMeasuredFPS = ( lastMeasuredFPS + 1 ) % FPS_BUFFER;
    }
    cfps++;
}

double playerSpeed_InertialValue = 0.0;
#define WALK_BOUNCE_SPEED 5.2
#define RUN_BOUNCE_SPEED WALK_BOUNCE_SPEED * 1.8
#define WALK_BOUNCE_AMP 0.007
#define RUN_BOUNCE_AMP 0.011
double bounce_Speed = WALK_BOUNCE_SPEED;

double ___AnxietyPowerLight = 3.0;

bool forceFPS = false;

void Core_Display() {
    ___apl_mul = 1.0 + ___AnxietyExternLevel * ___AnxietyPowerLight / STD_ANXIETY_MAX_LEVEL;
    //printf( "___apl_mul = %g\n", ___apl_mul );
    glColor3f( MainColor[ 0 ] * ___apl_mul, MainColor[ 1 ] * ___apl_mul, MainColor[ 2 ] * ___apl_mul );
    glFogf(	GL_FOG_START, FOG_START_DIST );
    glFogf(	GL_FOG_END, FOG_END_DIST );
    float FogCColor[ 3 ];
    for ( int i = 0; i < 3; i++ ) {
        FogCColor[ i ] = FogColor[ i ] * MainColor[ i ];
    }
    glClearColor( FogCColor[ 0 ], FogCColor[ 1 ], FogCColor[ 2 ], 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // TEMP
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    DefaultViewCam();
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glFogfv( GL_FOG_COLOR, FogCColor );
    // Drunk cam
    AutoKillCam();
    // /TEMP
    Point3D playerPos = player -> FixedPlayerSpot();
    Point3D playerLook = playerPos;
    playerLook.x += mainCamHandle -> lx + drunkPoint.x * 0.45;
    playerLook.y += mainCamHandle -> ly;
    playerLook.z += mainCamHandle -> lz + drunkPoint.z * 0.45;
    mainCamHandle -> SetRollAngle( ROLLAMP );
    double c_run_amp = sin( Timer::Current() * bounce_Speed ) * playerSpeed_InertialValue * ( 1 - ( int )( editMode ) );
    mainCamHandle -> AddRollAngle( c_run_amp );
    mainCamHandle -> LookAtAngles();
    SceneLight();
    ApplyLight( player -> FixedPlayerSpot(), mainCamHandle -> GetView() );
    ShootLight( player -> FixedPlayerSpot(), mainCamHandle -> GetView() );
    if ( ( !fogOverSky ) || ( !fogEnabled ) ) {
        glDisable( GL_FOG );
    }
    glDisable( GL_DEPTH_TEST );
    CloudLight_Begin();
    SkyDisplay( skyHeight, skyHorizon );
    CloudLight_End();
    glEnable( GL_DEPTH_TEST );
    if ( fogEnabled ) {
        glEnable( GL_FOG );
    }
    glDisable( GL_TEXTURE_2D );
    bloodStorage -> RegisterCam( player -> FixedPlayerSpot() );
    bloodStorage -> Process( grndVector, level_Gravity * 2.5, 0.05 );
    if ( editMode ) {
        Point3D pointedBlock = mainCamHandle -> NearestBlockLookingAt( MainMap, CURSOR_EDIT_RANGE, NULL );
        if ( pointedBlock.x >= 0 ) {
            DrawWCube( pointedBlock.x, pointedBlock.y, pointedBlock.z, _CURSOR_COLOR_MODE[ editStrictMode ][ 0 ], _CURSOR_COLOR_MODE[ editStrictMode ][ 1 ], _CURSOR_COLOR_MODE[ editStrictMode ][ 2 ] );//1.0, 1.0, 1.0 );
        }
        if ( editStrictMode == POLYGON_MODE ) {
            MainMapFiller -> Display();
        }
    }
    glEnable( GL_TEXTURE_2D );
    if ( showBillBoards ) {
        MainBillBoardStorage -> Display();
    }
    if ( !editMode ) {
        chunk_Range = min( ( int )( FOG_END_DIST / CHUNK_SIZE + 2 ), chunk_Range );
    }
    //omTest -> display( Point( 5.0, 7.0, 15.0 ), player -> FixedPlayerSpot() );
    //omTest -> display( Point( 10.0, 7.0, 10.0 ), player -> FixedPlayerSpot() );
    //omTest -> display( Point( 15.0, 7.0, 5.0 ), player -> FixedPlayerSpot() );
    MainMap -> DisplayOptimized( playerPos, chunk_Range, mainCamHandle -> pitchAngle );
    glPushMatrix();
    ammoBoxStack -> DisplayAll( player -> HotSpot() );
    fleshStorage -> DisplayAll();
    if ( editMode ) {
        ammoSpawnStack -> DisplayAll();
    }
    weaponSpawnStack -> DisplayAll( editMode );
    glPopMatrix();
    DisplayBullets();
    for ( int i = 0; i < MAX_MONSTERS; i++ ) {
        if ( monsterStorage[ i ] ) {
            monsterStorage[ i ] -> Display( monsterStorage[ i ] -> HotSpot() );
        }
    }
    if ( connector ) {
        map < int, Entity* >::iterator it = playersOnServer -> getIterator();
        while ( !playersOnServer -> allIterated( it ) ) {
            Entity* e = it -> second;
            e -> Display( e -> HotSpot() );
            it++;
        }
    }
    ProcessBloodVestiges();
    glDisable( GL_FOG );
    if ( editMode ) {
        if ( ( Dist3D( playerPos, editMark ) > 1.5 ) && ( image[ TEXTURE_MARK ] -> UseImage() ) ) {
            Cube( editMark.x, editMark.y, editMark.z, 1.0, TEXTURE_MARK );
        }
    }
    glPushMatrix();
    glLoadIdentity();
    if ( ( cWeapon ) && ( !editMode ) ) {
        if ( weapons_enabled ) {
            double fovTime = max( 0.0, min( ( Timer::Current() - FOV_Timer ) / FOV_JOIN_TIME, 1.0 ) );
            if ( MousePressed[ 1 ] ) {
                StdShootPoint = Balance( BasicShootPoint, StdScopePoint, fovTime );
            } else {
                StdShootPoint = Balance( StdScopePoint, BasicShootPoint, fovTime );
            }
            StdShootPoint = Balance( StdShootPoint, StdRunAmplitudePoint, c_run_amp * 10.0 );
            Point3D target = Balance( Point( StdShootPoint.x, StdShootPoint.y, StdShootPoint.z - 1.0 ), StdRunAmplitudeTargetPoint, c_run_amp * 10.0 );
            cWeapon -> Display( StdShootPoint.x, StdShootPoint.y, StdShootPoint.z, target.x, target.y, target.z );//StdShootPoint.x, StdShootPoint.y, StdShootPoint.z - 1.0 );
        }
    } else {
        int tID = blockID;
        switch ( editStrictMode ) {
            case AMMO_SPAWN_MODE:
                tID = ammoStash -> GetAmmo( spawnAmmoID ) -> GetBox();
            case PLANTER_MODE:
            case POLYGON_MODE:
            case STRICT_EDIT: {
                if ( image[ tID ] -> UseImage() ) {
                    block -> DrawRotated( NULL, Point( 0.35, -0.45, -0.7 ), 0, 0, 0 );
                }
                break;
            }
            case WEAPON_SPAWN_MODE: {
                Weapon* w = weaponStash -> GetWeapon( spawnWeaponID );
                if ( w ) {
                    w -> Display( 0.5, -0.55, -1.0, -1.0, -0.45, -1.2 );//p1.x, p1.y, p1.z, p2.x, p2.y, p2.z );
                }
                break;
            }
        }
    }
    glPopMatrix();
    glDisable( GL_DEPTH_TEST );
    if ( image[ TEXTURE_CURSOR ] -> UseImage() ) {
        glPushMatrix();
        glLoadIdentity();
        glTranslatef( 0.0, 0.0, -1.0 );
        glRasterPos2f( 0.0, 0.0 );
        glDrawPixels( 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, image[ TEXTURE_CURSOR ] -> RawData() );
        // mark
        glPopMatrix();
    }
    glDisable( GL_LIGHTING ); // text display
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluPerspective( 70, double( wX ) / double ( wY ), 0.1, MAX_DRAW_DIST );
    glMatrixMode( GL_MODELVIEW );
    if ( editMode ) {
        stdCon -> stdPrint( 0, 0, "Player X = " + str( playerPos.x ) );
        stdCon -> stdPrint( 0, 1, "Player Y = " + str( playerPos.y ) );
        stdCon -> stdPrint( 0, 2, "Player Z = " + str( playerPos.z ) );
        stdCon -> stdPrint( 0, 3, "Player AXZ = " + str( degr( mainCamHandle -> pitchAngle ) ) );
        stdCon -> stdPrint( 0, 4, "Player AY = " + str( degr( mainCamHandle -> yawAngle ) ) );
        stdCon -> stdPrint( 0, 6, "Mark X = " + str( editMark.x ) );
        stdCon -> stdPrint( 0, 7, "Mark Y = " + str( editMark.y ) );
        stdCon -> stdPrint( 0, 8, "Mark Z = " + str( editMark.z ) );
        stdCon -> stdPrint( 0, 9, "Distance( Player, Mark ) = " + str( Dist3D( playerPos, editMark ) ) );
        stdCon -> stdPrint( 0, 10, "Player HP = " + str( player -> GetVar( VAR_HP ) ) + " / " + str( player -> GetVar( VAR_MAX_HP ) ) );
        stdCon -> stdPrint( 0, 11, "Strict mode: " + _STRICT_MODE_NAME[ editStrictMode ] );
        switch ( editStrictMode ) {
            case STRICT_EDIT: {
                break;
            }
            case POLYGON_MODE: {
                break;
            }
            case PLANTER_MODE: {
                break;
            }
            case AMMO_SPAWN_MODE: {
                Ammo* a = ammoStash -> GetAmmo( spawnAmmoID );
                if ( a ) {
                    stdCon -> stdPrint( 0, 12, "Ammo spawning = " + a -> GetName() );//str( spawnAmmoID ) );
                }
                break;
            }
            case WEAPON_SPAWN_MODE: {
                Weapon* w = weaponStash -> GetWeapon( spawnWeaponID );
                if ( w ) {
                    stdCon -> stdPrint( 0, 12, "Weapon spawning = " + w -> GetName() );//str( spawnAmmoID ) );
                }
                break;
            }
        }
        if ( pointedBlock.x >= 0.0 ) {
            stdCon -> stdPrint( 0, 13, "Pointed block = " + str( pointedBlock.x ) + " / " + str( pointedBlock.y ) + " / " + str( pointedBlock.z ) );
        }
        /*if ( ( radioSoundState ) && ( foundRadio > 0 ) ) {
            stdCon -> stdPrint( 0, 79, "Current playing: " + string( radio -> GetCurrentPath() ) );
        }*/
    } else {
        if ( ( cWeapon ) && ( weapons_enabled ) ) {
            string strAmmo = "(INF)";
            Ammo* ammo;
            if ( ( ammo = ammoStash -> GetAmmo( cWeapon -> GetVar( WEAPON_VAR_AMMOUSEDKEY ) ) ) != NULL ) {
                if ( !( ammo -> IsInfinity() ) ) {
                    strAmmo = str( ammo -> GetBulletCount() );
                }
            }
            stdCon -> stdPrint( 0, 79, "Ammo: " + strAmmo );
        }
    }
    if ( ( editMode ) || ( forceFPS ) ) {
        stdCon -> stdPrint( 0, 77, "FPS = " + str( RealFPS() ) );
    }
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glEnable( GL_FOG );
    // 2d test
    /* //glDisable( GL_TEXTURE_2D );
    glColor3f( 0.0, 0.5, 0.0 );
    glBegin( GL_QUADS );
        glVertex2f( -0.5, -0.5 );
        glVertex2f( 0.5, -0.5 );
        glVertex2f( 0.5, 0.5 );
        glVertex2f( -0.5, 0.5 );
    glEnd();
    glEnable( GL_TEXTURE_2D );*/
    glEnable( GL_TEXTURE_2D );
    Effect_Roar( ___AnxietyExternLevel / STD_ANXIETY_MAX_LEVEL );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_LIGHTING );
    glFlush();
    glutSwapBuffers();
}

void Core_Reshape( int winX, int winY ) {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    DefaultViewCam();
    glMatrixMode( GL_MODELVIEW );
    Core_Display();
}

double playerWalk = 0.03;
double playerSprint = playerWalk * 3.5;

double playerSpeed = playerWalk;

#define PLAYER_INERTIA 0.9

// double SHOOT_LIGHT_RADIUS_POINT = 2.0;

void AlertMobs() {
    for ( int i = 0; i < MAX_MONSTERS; i++ ) {
        if ( monsterStorage[ i ] ) {
            int seen = monsterStorage[ i ] -> GetVar( VAR_ENTITY_SEEN );
            double alert_range = ENTITY_ALERT_RANGE_DEFAULT + monsterStorage[ i ] -> GetVar( VAR_ENTITY_ALERT_RANGE );
            if ( !seen ) {
                double d, ei, ea;
                Point3D p = player -> FixedPlayerSpot();
                Point3D e = monsterStorage[ i ] -> HotSpot();
                cts( e.x - p.x, p.y - e.y, e.z - p.z, &d, &ea, &ei );
                ei += DEGR90;
                ea -= DEGR90;
                //if ( e
                if ( d < FOG_END_DIST ) {
                    double dif_ei = ei - mainCamHandle -> pitchAngle;
                    double dif_ea = ea - mainCamHandle -> yawAngle;
                    if ( ( absf( dif_ei ) < ( PI / 4.0 ) ) && ( absf( dif_ea ) < ( PI / 2.0 ) ) ) {
                        //printf( "Seen enemy.\n Pos = ( %g, %g, %g ),\n Player = ( %g, %g, %g ),\n Dist = %g,\n Angles = ( %g, %g / dif %g, %g )\n",
                               //e.x, e.y, e.z, p.x, p.y, p.z, d, ei, ea, ei - mainCamHandle -> pitchAngle, ea - mainCamHandle -> yawAngle );
                        seen = 1;
                    }
                    if ( !seen ) {
                        if ( d < alert_range ) {
                            double angleAbstractDist = sqrt( dif_ei * dif_ei + dif_ea * dif_ea );
                            mainCamHandle -> DriveTo( Point( ei, ea, 0.0 ), PLAYER_REACTION_TIME * angleAbstractDist );
                        }
                    }
                }
                monsterStorage[ i ] -> SetVar( VAR_ENTITY_SEEN, seen ); // only if not seen before, because it cannot be unseen
            }
        }
    }
}

void Player_Move() {
    // key detect
    if ( KeyPressed[ 'x' ] ) {
        playerSpeed = playerSprint;
    } else {
        if ( editMode ) {
            playerSpeed = playerSpeed_EditMode_Init[ playerSpeed_EditMode_ArrayID ];
        } else {
            playerSpeed = playerWalk;
        }
    }
    Point3D Offset = { 0.0, 0.0, 0.0 };
    player_Running = false;
    if ( KeyPressed[ ' ' ] ) {
        if ( player -> TryJump( MainMap, 0.18 ) ) {
            StopSound( CHANNEL_STEPS );
            player_Running = true;
        }
    }
    if ( KeyPressed[ 'w' ] ) {
        Move3D( &Offset, mainCamHandle, playerSpeed, 0.0 );
        player_Running = KeyPressed[ 'x' ];
        if ( player_Running ) {
            SetVolume( CHANNEL_STEPS, 0.9 );
            SetPitch( CHANNEL_STEPS, 1.8 );
            playerSpeed_InertialValue = playerSpeed_InertialValue * 0.9 + RUN_BOUNCE_AMP * 0.1;
            bounce_Speed = RUN_BOUNCE_SPEED;
        } else {
            SetVolume( CHANNEL_STEPS, 0.62 );
            SetPitch( CHANNEL_STEPS, 1.0 );
            playerSpeed_InertialValue = playerSpeed_InertialValue * 0.9 + WALK_BOUNCE_AMP * 0.1;
            bounce_Speed = WALK_BOUNCE_SPEED;
        }
    } else {
        playerSpeed_InertialValue *= 0.9;
    }
    if ( KeyPressed[ 's' ] ) {
        if ( !KeyPressed[ 'w' ] ) {// && ( !KeyPressed[ 'W' ] ) ) {
            if ( !editMode ) {
                playerSpeed = playerWalk;
            }
        }
        Move3D( &Offset, mainCamHandle, - playerSpeed, 0.0 );
    }
    if ( !editMode ) {
        playerSpeed = playerWalk;
    }
    if ( KeyPressed[ 'a' ] ) {
        Move3D( &Offset, mainCamHandle, 0.0, - playerSpeed );
    }
    if ( KeyPressed[ 'd' ] ) {
        Move3D( &Offset, mainCamHandle, 0.0, playerSpeed );
    }
    // TEMP
    if ( KeyPressed[ '=' ] ) { // DAMAGE ME!
        player -> SetVar( VAR_HP, player -> GetVar( VAR_HP ) - 1.0 );
    }
    // /TEMP
    if ( editMode ) {
        Point3D verticalMoveVector = Point( 0.0, playerSpeed, 0.0 );
        if ( KeyPressed[ ' ' ] ) {
            Move3D( &Offset, verticalMoveVector, 1.0 );
        }
        if ( KeyPressed[ 'z' ] ) {
            Move3D( &Offset, verticalMoveVector, -1.0 );
        }
    } else {
        if ( level_Gravity > 0.0 ) {
            player -> GravityAffect( MainMap, STD_GRAVITY_POWER, STD_GRAVITY_INERTIA, &Offset );
        }
    }
    if ( connector ) {
        //Point3D normOffset = Offset;
        //double offset_length = Dist3D( Offset, Point( 0.0, 0.0, 0.0 ) );
        //if ( offset_length >= 1.0E-12 ) {
            //ScalePoint3D( &normOffset, 1.0 / offset_length );
            /// but will we trust the player?
            Offset = player -> CollidedAtMap( MainMap, Offset );
            player -> FaceTo( AddPoint( player -> FixedPlayerSpot(), mainCamHandle -> GetView() ) );
            Point3D look = player -> Facing();
            MoveRequest request;
            request.x = Offset.x;
            request.y = Offset.y;
            request.z = Offset.z;
            request.lx = look.x;
            request.ly = look.y;
            request.lz = look.z;
            //request.spd = offset_length;
            Message* msg = new Message( sizeof( request ), MESSAGE_CODE_MOVE_REQUEST, &request );
            /*while ( !connector -> directSend( msg ) ) {
                Sleep( 1 );
            }*/
            connector -> directSend( msg );
            delete msg;
            printf( "Sent request.\n" );
        //}
    } else {
        player -> MoveAtMap( MainMap, Offset );
    }
    int ksPressed = KeyPressed[ 'w' ] || KeyPressed[ 's' ] || KeyPressed[ 'a' ] || KeyPressed[ 'd' ];
    bool movedOnGround = HasMovedOnGround( player -> HotSpot(), Offset );
    if ( movedOnGround ) {
        if ( !IsPlaying( CHANNEL_STEPS ) ) {
            if ( ksPressed > 0 ) {
                LoopSound( CHANNEL_STEPS, "footsteps.ogg" );
            }
        }
    }
    if ( ( ( ksPressed == 0 ) && IsPlaying( CHANNEL_STEPS ) ) || ( !movedOnGround ) ) {
        StopSound( CHANNEL_STEPS );
    }
}

void Core_Main() {
    //printf( "Color: %g %g %g\n", MainColor[ 0 ], MainColor[ 1 ], MainColor[ 2 ] );
    FPSCounter();
    cheatSystem -> Tick();
    if ( level_presented != currentLevel ) {
        Core_ForceLevel( level_presented );
    }
    if ( !editMode ) {
        PSEVM_Inject();
    }
    // player is alive?
    if ( player -> GetVar( VAR_HP ) <= 0.0 ) {
        //RespawnPlayer(); // Welcome to hell
        MIND_STATE = STATE_ON_KILLED;
    }
    Player_Move();
    // pointed block
    pointedBlock = mainCamHandle -> NearestBlockLookingAt( MainMap, CURSOR_EDIT_RANGE, &pointedSide );
    // quake
    mainCamQuaker -> QuakeTick();
    // shooting
    mainCamHandle -> RecoilAdaption();
    Weapon* picked = NULL;
    if ( !editMode ) {
        picked = weaponSpawnStack -> TryPickUp( player -> HotSpot(), STD_PICKUP_RANGE );
    }
    if ( weaponStash ) {
        if ( picked ) {
            cWeapon = picked;
            currentWeaponKey = weaponStash -> GetWeaponKey( picked );
            PlaySound( CHANNEL_PICKUP, "pickUp.ogg" );
        } else {
            cWeapon = weaponStash -> GetWeapon( currentWeaponKey );
        }
    } else {
        cWeapon = NULL;
    }
    if ( ( MousePressed[ 0 ] ) && ( !editMode ) && ( weapons_enabled ) ) {
        int j = 0;
        if ( cWeapon ) {
            if ( cWeapon -> TryShoot() == SHOOT_CORRECT ) {
                Point3D playerSpot = player -> HotSpot();
                playerSpot.y += pHeight;
                Point3D StdShootPointRotated = StdShootPoint;
                double r, iP, aP;
                Point3D weaponOffsetPoint = Point(
                                    cWeapon -> GetVar( WEAPON_VAR_SHOOTOFFSET_X ),
                                    cWeapon -> GetVar( WEAPON_VAR_SHOOTOFFSET_Y ),
                                    cWeapon -> GetVar( WEAPON_VAR_SHOOTOFFSET_Z ) );
                cts( StdShootPointRotated.x + weaponOffsetPoint.x,
                     StdShootPointRotated.y + weaponOffsetPoint.y,
                     StdShootPointRotated.z + weaponOffsetPoint.z, &r, &iP, &aP );
                iP -= mainCamHandle -> yawAngle;
                aP += mainCamHandle -> pitchAngle;
                stc( r, iP, aP, &StdShootPointRotated.x, &StdShootPointRotated.y, &StdShootPointRotated.z );
                playerSpot = AddPoint( playerSpot, StdShootPointRotated );
                double recoil = cWeapon -> GetVar( WEAPON_VAR_RECOIL );
                double bulletDisp = cWeapon -> GetVar( WEAPON_VAR_BULLETDISPERSION );
                for ( int i = 0; i < MAX_BULLETS; i++ ) {
                    if ( !bulletStorage[ i ] ) { // free slot
                        Point3D scaledLook = mainCamHandle -> GetDispersedView( bulletDisp );
                        ScalePoint3D( &scaledLook, double( FOG_END_DIST ) );
                        Ammo* used_ammo = cWeapon -> UsedAmmo();
                        bulletStorage[ i ] = new Bullet( player, playerSpot, AddPoint( scaledLook, player -> FixedPlayerSpot() ), used_ammo -> BulletColor(), cWeapon -> GetRandomDamage(), cWeapon -> GetVar( WEAPON_VAR_BULLETSPEED ), STD_BULLET_MAX_DISTANCE, cWeapon -> GetVar( WEAPON_VAR_BULLETVEL ) );
                        lastShootOccuredPos = playerSpot;
                        j++;
                        if ( j >= ( int )( cWeapon -> GetVar( WEAPON_VAR_BULLETSHOOTCOUNT ) ) ) {
                            break;
                        }
                    }
                }
                if ( j > 0 ) {
                    FullShootLight( mainCamHandle -> GetView() );
                    mainCamHandle -> Recoil( recoil );
                }
            }
        }
    }
    if ( ( !editMode ) && ( cWeapon ) && ( weapons_enabled ) ) {
        FOV_JOIN_TIME = cWeapon -> GetVar( WEAPON_VAR_SCOPINGTIME );
        double fovTime = max( 0.0, min( ( Timer::Current() - FOV_Timer ) / FOV_JOIN_TIME, 1.0 ) );
        if ( fovTime >= 1.0 ) {
            FOV_Timer = Timer::Current() - FOV_JOIN_TIME;
        }
        if ( MousePressed[ 1 ] ) {
            FOV_SCOPE = fovTime * cWeapon -> GetVar( WEAPON_VAR_FOVSCOPE );
        } else {
            FOV_SCOPE = ( 1.0 - fovTime ) * cWeapon -> GetVar( WEAPON_VAR_FOVSCOPE );//0.0;
        }
    }
    fleshStorage -> ProcessAll();
    // monster spawning & ammo pickups
    if ( !editMode ) {
        //if ( ( timerMonsterSpawn -> Tick() ) && ( !editMode ) ) { // unused, now it can spawn different monsters! WOW!
        // #wow
        // #verycomplication
        // #dogelikeit
        // #dogewow
        // #sohorrible
        // #wow
            MonsterSpawnCycle();
        //} // Headless dogs won't like it though.
        RecalcSpawnDist();
        SpawnAuto( spawnDist );
        // pick up the stuff
        ammoBoxStack -> TryPickUp( player -> HotSpot() );
        ammoSpawnStack -> SpawnAmmoCycle();
    } else {
        for ( int i = 0; i < MAX_MONSTERS; i++ ) {
            if ( monsterStorage[ i ] ) {
                delete monsterStorage[ i ];
                monsterStorage[ i ] = NULL;
            }
        }
    }
    MoveBullets();
    MonsterControl();
    AlertMobs();
    // display
    Core_Display();
}

void Multi_Main() {
    FPSCounter();
    // grab info
    bool received = true;
    while ( received ) {
        connector -> lockQueue();
        Message* info = connector -> nextMessage();
        connector -> unlockQueue();
        if ( info ) {
            switch ( info -> getCode() ) {
                case MESSAGE_CODE_POSITION: {
                    PositionInfo* pos = ( PositionInfo* )( info -> getData() );
                    Entity* e = playersOnServer -> get( pos -> id );
                    if ( e == NULL ) {
                        e = new Entity( pos -> x, pos -> y, pos -> z, 8, object[ 33 ] );
                        playersOnServer -> add( pos -> id, e );
                        //printf( "Added entity: id = %d\n", pos -> id );
                    } else {
                        Point3D newPos = Point( pos -> x, pos -> y, pos -> z );
                        Point3D oldPos = e -> HotSpot();
                        e -> SetVar( VAR_ENTITY_MOVE_SPEED_XZ, Dist3D( newPos, oldPos ) / 2.0 );
                        e -> UpdateHotSpot( newPos );
                        e -> FaceTo( Point( pos -> lx, pos -> ly, pos -> lz ) );
                        //printf( "Modified entity: id = %d\n", pos -> id );
                    }
                    break;
                }
            }
            delete info;
        } else {
            received = false;
        }
    }
    // send sth
    Player_Move();
    // move to display actual data
    Core_Display();
}

#include "Menu.h"

void Mind_MousePressed( int button, int state, int x, int y ) {
    switch ( MIND_STATE ) {
        case STATE_GAME: {
            Core_MousePressed( button, state, x, y );
            break;
        }
        case STATE_GAME_MP: {
            Core_MousePressed( button, state, x, y );
            break;
        }
        case STATE_MENU: {
            Menu_MousePressed( button, state, x, y );
            break;
        }
    }
}

void Mind_MouseMotion( int x, int y ) {
    //Core_Motion( x, y );
    // /// Do not remove - TO-DO section
    switch ( MIND_STATE ) {
        case STATE_GAME: {
            Core_Motion( x, y );
            break;
        }
        case STATE_GAME_MP: {
            Core_Motion( x, y );
            break;
        }
        case STATE_MENU: {
            Menu_Motion( x, y );
            break;
        }
    }
}

void Mind_Display() {
    switch ( MIND_STATE ) {
        case STATE_GAME: {
            Core_Display();
            break;
        }
        case STATE_GAME_MP: {
            Core_Display();
            break;
        }
        case STATE_MENU: {
            Menu_Display();
            break;
        }
        case STATE_LOADING: {
            int state = MIND_STATE;
            MIND_STATE = MIND_SAVED_STATE;
            Mind_Display();
            MIND_STATE = state;
            break;
        }
    }
}

void Mind_Reshape( int x, int y ) {
    glViewport( 0, 0, x, y );
    wX = x;
    wY = y;
    switch ( MIND_STATE ) {
        case STATE_GAME: {
            Core_Reshape( x, y );
            break;
        }
        case STATE_GAME_MP: {
            Core_Reshape( x, y );
            break;
        }
        case STATE_MENU: {
            Menu_Reshape();
            break;
        }
        case STATE_LOADING: {
            int state = MIND_STATE;
            MIND_STATE = MIND_SAVED_STATE;
            Mind_Reshape( x, y );
            MIND_STATE = state;
            break;
        }
    }
}

#include "DelayedLoading.h"

void Mind_Main() {
    //printf( " > Mind_Main()\n" );
    switch ( MIND_STATE ) {
        case STATE_GAME: {
            //printf( " > Core_Main()\n" );
            Core_Main();
            break;
        }
        case STATE_GAME_MP: {
            Multi_Main();
            break;
        }
        case STATE_MENU: {
            //printf( " > Menu_Main()\n" );
            Menu_Main();
            break;
        }
        case STATE_INIT: {
            //printf( " > Core_Init()\n" );
            Core_Init();
            //printf( " > Menu_Init()\n" );
            Menu_Init();
            //printf( " > STATE_ON_MENU_INIT_GAME()\n" );
            MIND_STATE = STATE_ON_MENU_INIT_GAME;
            break;
        }
        case STATE_LOADING: {
            if ( !delayedLoader ) {
                if ( connector ) {
                    //delayedLoader = new DelayedThread( LOADER_STATE_INITIALIZED, Delayed_Level_MP );
                    Level_Init_MP();
                } else {
                    //delayedLoader = new DelayedThread( LOADER_STATE_INITIALIZED, Delayed_Level );
                    Level_Init();
                }
            } else if ( delayedLoader -> stateEquals( LOADER_STATE_FINISHED ) ) {
                delete delayedLoader;
                delayedLoader = NULL;
                MIND_STATE = STATE_ON_RESUME_GAME;
            } else {
                // ???!??!?!???!?!??!??!?!?
                int state = MIND_STATE;
                MIND_STATE = MIND_SAVED_STATE;
                Mind_Main();
                MIND_STATE = state;
            }
            MIND_STATE = STATE_ON_RESUME_GAME;
            break;
        }
        case STATE_QUIT: {
            Core_Quit();
            Menu_Destroy();
            exit( 0 );
            break;
        }
        case STATE_ON_MENU_INIT_GAME: {
            //printf( " > Pause sounds...\n" );
            for ( int i = 0; i < MAX_DEF_CHANNELS; i++ ) {
                PauseSound( i );
            }
            //printf( " > Loop sound BG...\n" );
            LoopSound( CHANNEL_MENUMUSIC, "levelMusic/main.ogg" );
            //printf( " > Reset menu camera...()\n" );
            MenuResetCam();
            //printf( " > STATE_MENU()\n" );
            MIND_STATE = STATE_MENU;
            break;
        }
        case STATE_ON_ABORT_GAME: {
            Level_CleanUp();
            MIND_STATE = STATE_ON_MENU_INIT_GAME;
            break;
        }
        case STATE_ON_KILLED: {
            MIND_STATE = STATE_ON_ABORT_GAME;
            break;
        }
        case STATE_ON_PAUSE_GAME: {
            menuButtonGroup = MENU_GROUP_PAUSE;
            MIND_STATE = STATE_ON_MENU_INIT_GAME;
            break;
        }
        case STATE_ON_RESUME_GAME: {
            for ( int i = 0; i < MAX_DEF_CHANNELS; i++ ) {
                ResumeSound( i );
            }
            StopSound( CHANNEL_MENUMUSIC );
            if ( connector ) {
                MIND_STATE = STATE_GAME_MP;
            } else {
                MIND_STATE = STATE_GAME;
            }
            break;
        }
    }
}


