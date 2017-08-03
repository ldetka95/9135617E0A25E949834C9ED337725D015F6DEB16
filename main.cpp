#include <gl/gl.h>
#include <glut/glut.h>

#include "Core.h"
#include <time.h>

#define IDR_ICO_MAIN 1234

int main( int argc, char** argv ) {
    srand( time( NULL ) );
    InitMainInterpreter();
    // init radio
    /*PDWORD thrID = 0;
    HANDLE radioThread = CreateThread( NULL, 0, Radio, NULL, 0, thrID );
    while ( !radioEstablished ) {
        // wait for radio thread
    }*/
    // GL/GLUT
    glutInit( &argc, argv );
    glutInitWindowSize( SCREENX, SCREENY );
    InitSTDConsole( SCREENX, SCREENY );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA );
    glutCreateWindow( "@NIGHT CORE 0.2.4.A" );
    //Core_Init();
    glutIdleFunc( Mind_Main );
    glutDisplayFunc( Mind_Display );
    glutReshapeFunc( Mind_Reshape );
    glutFullScreen();
    // keyboard
    glutKeyboardFunc( Core_Keystroke );
    glutKeyboardUpFunc( Core_KeyUp );
    glutSpecialFunc( Core_SpecialInput );
    // mouse
    glutMouseFunc( Mind_MousePressed );
    glutMotionFunc( Mind_MouseMotion );
    glutPassiveMotionFunc( Mind_MouseMotion );
    // main
    //printf( "Entering main loop...\n" );
    glutMainLoop();
    // out
    //RadioDone = true;
    //CloseHandle( radioThread );
    //RemoveMainInterpreter();
    //delete v_radio;
    return 0;
}

