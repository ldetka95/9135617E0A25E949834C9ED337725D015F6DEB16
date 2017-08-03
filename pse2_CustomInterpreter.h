#pragma once

#include "Core.h"
#include "pse2.h"

/// CUSTOM FUNCTIONS

#define STD_REGISTER_NUMBER 64
double REGISTER[ STD_REGISTER_NUMBER ];

#define STD_CALLSTACK_SIZE 64
int CALLSTACK[ STD_CALLSTACK_SIZE ];
int CALLSTACK_index = -1;

bool parsingCritical = false;

#define STD_DATASTACK_SIZE 64
int DATASTACK[ STD_DATASTACK_SIZE ];
int DATASTACK_index = 0;

#define CFUNC_NOOP 0
void* CFUNC_Noop( void* vcaller, PSE2_Argument* args ) {
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_REG_MOV 1
void* CFUNC_Reg_Mov( void* vcaller, PSE2_Argument* args ) {
    int arg[ 2 ];
    for ( int i = 0; i < 2; i++ ) {
        arg[ i ] = ( int )( *( double* )( args -> GetArgument( i ) ) );
    }
    REGISTER[ arg[ 0 ] ] = REGISTER[ arg[ 1 ] ];
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_REG_SET 2
void* CFUNC_Reg_Set( void* vcaller, PSE2_Argument* args ) {
    double arg[ 2 ];
    for ( int i = 0; i < 2; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    REGISTER[ int( arg[ 0 ] ) ] = arg[ 1 ];
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_REG_ADD_CONST 3
void* CFUNC_Reg_Add_Const( void* vcaller, PSE2_Argument* args ) {
    double arg[ 2 ];
    for ( int i = 0; i < 2; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    REGISTER[ int( arg[ 0 ] ) ] += arg[ 1 ];
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_REG_ADD 4
void* CFUNC_Reg_Add( void* vcaller, PSE2_Argument* args ) {
    double arg[ 2 ];
    for ( int i = 0; i < 2; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    REGISTER[ int( arg[ 0 ] ) ] += REGISTER[ int( arg[ 1 ] ) ];
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_REG_SUB 5
void* CFUNC_Reg_Sub( void* vcaller, PSE2_Argument* args ) {
    double arg[ 2 ];
    for ( int i = 0; i < 2; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    REGISTER[ int( arg[ 0 ] ) ] -= REGISTER[ int( arg[ 1 ] ) ];
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_REG_NEG 6
void* CFUNC_Reg_Neg( void* vcaller, PSE2_Argument* args ) {
    double arg = *( double* )( args -> GetArgument( 0 ) );
    REGISTER[ int( arg ) ] = -REGISTER[ int( arg ) ];
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_REG_MUL 7
void* CFUNC_Reg_Mul( void* vcaller, PSE2_Argument* args ) {
    double arg[ 2 ];
    for ( int i = 0; i < 2; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    REGISTER[ int( arg[ 0 ] ) ] *= REGISTER[ int( arg[ 1 ] ) ];
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_REG_DIV 8
void* CFUNC_Reg_Div( void* vcaller, PSE2_Argument* args ) {
    double arg[ 2 ];
    for ( int i = 0; i < 2; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    REGISTER[ int( arg[ 0 ] ) ] /= REGISTER[ int( arg[ 1 ] ) ];
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_LOOP_BY_REG 9
void* CFUNC_Loop_By_Reg( void* vcaller, PSE2_Argument* args ) {
    int arg[ 2 ];
    for ( int i = 0; i < 2; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    if ( REGISTER[ arg[ 0 ] ] > 0 ) {
        REGISTER[ arg[ 0 ] ]--;
        ( ( PSE2_Line* )( vcaller ) ) -> JumpInstruction( arg[ 1 ] );
    } else {
        ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    }
    return NULL;
}

#define CFUNC_CRITICALBEGIN 12
void* CFUNC_CriticalBegin( void* vcaller, PSE2_Argument* args ) {
    parsingCritical = true;
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_CRITICALEND 13
void* CFUNC_CriticalEnd( void* vcaller, PSE2_Argument* args ) {
    parsingCritical = false;
    ( ( PSE2_Line* )( vcaller ) ) -> NextInstruction();
    return NULL;
}

#define CFUNC_RET 14
void* CFUNC_Ret( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int addr = CALLSTACK[ CALLSTACK_index ];
    CALLSTACK_index--;
    caller -> JumpInstruction( addr );
    return NULL;
}

#define CFUNC_CALL 15
void* CFUNC_Call( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int addr = ( int )( *( double* )( args -> GetArgument( 0 ) ) );
    CALLSTACK_index++;
    CALLSTACK[ CALLSTACK_index ] = caller -> CurrentInstruction();
    caller -> JumpInstruction( addr );
    return NULL;
}

#define CFUNC_JUMP 16
void* CFUNC_Jump( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int addr = ( int )( *( double* )( args -> GetArgument( 0 ) ) );
    caller -> JumpInstruction( addr );
    return NULL;
}

#define CFUNC_JUMP_IF_LESS 17
void* CFUNC_Jump_If_Less( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int arg[ 3 ];
    for ( int i = 0; i < 3; i++ ) {
        arg[ i ] = ( int )( *( double* )( args -> GetArgument( i ) ) );
    }
    if ( REGISTER[ arg[ 0 ] ] < REGISTER[ arg[ 1 ] ] ) {
        caller -> JumpInstruction( arg[ 2 ] );
    } else {
        caller -> NextInstruction();
    }
    return NULL;
}

#define CFUNC_JUMP_IF_GREATER 18
void* CFUNC_Jump_If_Greater( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int arg[ 3 ];
    for ( int i = 0; i < 3; i++ ) {
        arg[ i ] = ( int )( *( double* )( args -> GetArgument( i ) ) );
    }
    if ( REGISTER[ arg[ 0 ] ] > REGISTER[ arg[ 1 ] ] ) {
        caller -> JumpInstruction( arg[ 2 ] );
    } else {
        caller -> NextInstruction();
    }
    return NULL;
}

#define CFUNC_JUMP_IF_EQUAL 19
void* CFUNC_Jump_If_Equal( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int arg[ 3 ];
    for ( int i = 0; i < 3; i++ ) {
        arg[ i ] = ( int )( *( double* )( args -> GetArgument( i ) ) );
    }
    if ( REGISTER[ arg[ 0 ] ] == REGISTER[ arg[ 1 ] ] ) {
        caller -> JumpInstruction( arg[ 2 ] );
    } else {
        caller -> NextInstruction();
    }
    return NULL;
}

#define CFUNC_WHILE_NOT_INDIST 20
void* CFUNC_While_Not_Indist( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    double arg[ 4 ];
    for ( int i = 0; i < 4; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    Point3D target = Point( arg[ 0 ], arg[ 1 ], arg[ 2 ] );
    double dist = Dist3D( player -> HotSpot(), target );
    //printf( "Distance = %lf [Expected %lf]\n", dist, arg[ 3 ] );
    if ( dist <= arg[ 3 ] ) {
        //printf( "   Processing next...\n" );
        caller -> NextInstruction();
    }
    return NULL;
}

#define CFUNC_TELEPORT 22
void* CFUNC_Teleport( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    double arg[ 3 ];
    for ( int i = 0; i < 3; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    player -> UpdateHotSpot( Point( arg[ 0 ], arg[ 1 ], arg[ 2 ] ) );
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_TELEPORTRELATIVE 23
void* CFUNC_TeleportRelative( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    double arg[ 3 ];
    for ( int i = 0; i < 3; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    Point3D hs = player -> HotSpot();
    player -> UpdateHotSpot( Point( hs.x + arg[ 0 ], hs.y + arg[ 1 ], hs.z + arg[ 2 ] ) );
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_JUMP_IF_NOT_INDIST 24
void* CFUNC_Jump_If_Not_Indist( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    double arg[ 5 ];
    for ( int i = 0; i < 5; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    Point3D target = Point( arg[ 0 ], arg[ 1 ], arg[ 2 ] );
    double dist = Dist3D( player -> HotSpot(), target );
    if ( dist > arg[ 3 ] ) {
        caller -> JumpInstruction( arg[ 4 ] );
    } else {
        caller -> NextInstruction();
    }
    return NULL;
}

#define CFUNC_SPAWN 30
void* CFUNC_Spawn( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    double arg[ 4 ];
    for ( int i = 0; i < 4; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    for ( int i = 0; i < MAX_MONSTERS; i++ ) {
        if ( !monsterStorage[ i ] ) {
            monsterStorage[ i ] = new Entity( arg[ 0 ], arg[ 1 ], arg[ 2 ], 1, object[ ( int )( REGISTER[ ( int )( arg[ 3 ] ) ] ) ] );
            break;
        }
    }
    caller -> NextInstruction();
    return NULL;
}

int level_presented = 0;

#define CFUNC_NEXTLEVEL 31
void* CFUNC_NextLevel( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    level_presented++;
    /*if ( level_presented > 8 ) {
        level_presented = 0;
    }*/
    printf( "Next level called: %d.\n", level_presented );
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_SETLEVEL 32
void* CFUNC_SetLevel( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    level_presented = *( double* )( args -> GetArgument( 0 ) );
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_BILLBOARD 40
void* CFUNC_BillBoard( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    double arg[ 8 ];
    for ( int i = 0; i < 8; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }
    MainBillBoardStorage -> Append( Point( arg[ 0 ], arg[ 1 ], arg[ 2 ] ), arg[ 3 ], arg[ 4 ], arg[ 5 ], arg[ 6 ], arg[ 7 ] );
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_STACKPUSH 41
void* CFUNC_StackPush( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    /*double arg[ 8 ];
    for ( int i = 0; i < 8; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }*/
    //int newIndex_hi = ( DATASTACK_index_hi + 1 ) % ;
    //DATASTACK[
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_STACKPOP 42
void* CFUNC_StackPop( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    /*double arg[ 8 ];
    for ( int i = 0; i < 8; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }*/
    //
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_STACKBOTTOM 43
void* CFUNC_StackBottom( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    /*double arg[ 8 ];
    for ( int i = 0; i < 8; i++ ) {
        arg[ i ] = *( double* )( args -> GetArgument( i ) );
    }*/
    //
    caller -> NextInstruction();
    return NULL;
}

#define ___PSE_MEM_SIZE_BYTES 4*1024 // 4kB
int ___PSE_TOT_MEM_SIZE = ___PSE_MEM_SIZE_BYTES / sizeof( double );
int ___PSE_MemAddress = 0;
double* ___PSE_MEMORY = ( double* )( calloc( ___PSE_TOT_MEM_SIZE, sizeof( double ) ) );

#define CFUNC_STO_DOUBLE 44
void* CFUNC_Sto_Double( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int arg = *( double* )( args -> GetArgument( 0 ) );
    if ( ( ___PSE_MemAddress >= 0 ) && ( ___PSE_MemAddress < ___PSE_TOT_MEM_SIZE ) ) {
        ___PSE_MEMORY[ ___PSE_MemAddress ] = REGISTER[ arg ];
        //printf( "STO: MEM[ %d ] = %lf\n", ___PSE_MemAddress, REGISTER[ arg ] );
    } else {
        //printf( "STO: Address %d is out of bounds\n", ___PSE_MemAddress );
    }
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_LOD_DOUBLE 45
void* CFUNC_Lod_Double( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int arg = *( double* )( args -> GetArgument( 0 ) );
    if ( ( ___PSE_MemAddress >= 0 ) && ( ___PSE_MemAddress < ___PSE_TOT_MEM_SIZE ) ) {
        REGISTER[ arg ] = ___PSE_MEMORY[ ___PSE_MemAddress ];
        //printf( "LOD: MEM[ %d ] = %lf\n", ___PSE_MemAddress, REGISTER[ arg ] );
    } else {
        //printf( "LOD: Address %d is out of bounds\n", ___PSE_MemAddress );
    }
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_MEMORY_ADDRESS_GET 46
void* CFUNC_Memory_Address_Get( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int arg = *( double* )( args -> GetArgument( 0 ) );
    REGISTER[ arg ] = ___PSE_MemAddress;
    //printf( "Current memory address: %d\n", ___PSE_MemAddress );
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_MEMORY_ADDRESS_SET 47
void* CFUNC_Memory_Address_Set( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int arg = *( double* )( args -> GetArgument( 0 ) );
    ___PSE_MemAddress = ( int )( REGISTER[ arg ] );
    //printf( "Memory address set to: %d\n", ___PSE_MemAddress );
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_ENABLE_WEAPONS 48
void* CFUNC_Enable_Weapons( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int arg = *( double* )( args -> GetArgument( 0 ) );
    if ( arg == 0 ) {
        weapons_enabled = false;
    } else if ( arg == 1 ) {
        weapons_enabled = true;
    }
    caller -> NextInstruction();
    return NULL;
}

#define CFUNC_PRINTREG 63
void* CFUNC_PrintReg( void* vcaller, PSE2_Argument* args ) {
    PSE2_Line* caller = ( PSE2_Line* )( vcaller );
    int arg = *( double* )( args -> GetArgument( 0 ) );
    printf( "%g\n", REGISTER[ arg ] );
    caller -> NextInstruction();
    return NULL;
}

/// FUNCTIONS CONTROL

#define STD_OPERATION_NUMBER 64
PSE2_Interpreter* defaultInterpreter;

#define MAX_PSEVM_LINES 128
PSE2_Line* psevm[ MAX_PSEVM_LINES ];

void ResetPSEVM() {
    for ( int i = 0; i < MAX_PSEVM_LINES; i++ ) {
        if ( psevm[ i ] ) {
            delete psevm[ i ];
            psevm[ i ] = NULL;
        }
    }
}

void ReloadPSEVM( string incomplete_path ) {
    ResetPSEVM();
    StringLinker* scriptPathes = new StringLinker( incomplete_path + "script.slf" );
    for ( int i = 0; i < scriptPathes -> GetSize(); i++ ) {
        psevm[ scriptPathes -> GetIdOf( i ) ] = PSE2_LoadFrom( ( incomplete_path + scriptPathes -> GetStringOf( i ) ).c_str(), defaultInterpreter );
        //printf( "Script: %s\n", scriptPathes -> GetStringOf( i ).c_str() );
    }
    delete scriptPathes;
}

void InitMainInterpreter() {
    defaultInterpreter = new PSE2_Interpreter( STD_OPERATION_NUMBER );
    defaultInterpreter -> AddEntry( CFUNC_NOOP, CFUNC_Noop );
    defaultInterpreter -> AddEntry( CFUNC_REG_MOV, CFUNC_Reg_Mov );
    defaultInterpreter -> AddEntry( CFUNC_REG_SET, CFUNC_Reg_Set );
    defaultInterpreter -> AddEntry( CFUNC_REG_ADD_CONST, CFUNC_Reg_Add_Const );
    defaultInterpreter -> AddEntry( CFUNC_REG_ADD, CFUNC_Reg_Add );
    defaultInterpreter -> AddEntry( CFUNC_REG_SUB, CFUNC_Reg_Sub );
    defaultInterpreter -> AddEntry( CFUNC_REG_NEG, CFUNC_Reg_Neg );
    defaultInterpreter -> AddEntry( CFUNC_REG_MUL, CFUNC_Reg_Mul );
    defaultInterpreter -> AddEntry( CFUNC_REG_DIV, CFUNC_Reg_Div );
    defaultInterpreter -> AddEntry( CFUNC_LOOP_BY_REG, CFUNC_Loop_By_Reg );
    defaultInterpreter -> AddEntry( CFUNC_CRITICALBEGIN, CFUNC_CriticalBegin );
    defaultInterpreter -> AddEntry( CFUNC_CRITICALEND, CFUNC_CriticalEnd );
    defaultInterpreter -> AddEntry( CFUNC_RET, CFUNC_Ret );
    defaultInterpreter -> AddEntry( CFUNC_CALL, CFUNC_Call );
    defaultInterpreter -> AddEntry( CFUNC_JUMP, CFUNC_Jump );
    defaultInterpreter -> AddEntry( CFUNC_JUMP_IF_LESS, CFUNC_Jump_If_Less );
    defaultInterpreter -> AddEntry( CFUNC_JUMP_IF_GREATER, CFUNC_Jump_If_Greater );
    defaultInterpreter -> AddEntry( CFUNC_JUMP_IF_EQUAL, CFUNC_Jump_If_Equal );
    defaultInterpreter -> AddEntry( CFUNC_WHILE_NOT_INDIST, CFUNC_While_Not_Indist );
    defaultInterpreter -> AddEntry( CFUNC_TELEPORT, CFUNC_Teleport );
    defaultInterpreter -> AddEntry( CFUNC_TELEPORTRELATIVE, CFUNC_TeleportRelative );
    defaultInterpreter -> AddEntry( CFUNC_JUMP_IF_NOT_INDIST, CFUNC_Jump_If_Not_Indist );
    defaultInterpreter -> AddEntry( CFUNC_SPAWN, CFUNC_Spawn );
    defaultInterpreter -> AddEntry( CFUNC_NEXTLEVEL, CFUNC_NextLevel );
    defaultInterpreter -> AddEntry( CFUNC_SETLEVEL, CFUNC_SetLevel );
    defaultInterpreter -> AddEntry( CFUNC_BILLBOARD, CFUNC_BillBoard );
    defaultInterpreter -> AddEntry( CFUNC_LOD_DOUBLE, CFUNC_Lod_Double );
    defaultInterpreter -> AddEntry( CFUNC_STO_DOUBLE, CFUNC_Sto_Double );
    defaultInterpreter -> AddEntry( CFUNC_MEMORY_ADDRESS_GET, CFUNC_Memory_Address_Get );
    defaultInterpreter -> AddEntry( CFUNC_MEMORY_ADDRESS_SET, CFUNC_Memory_Address_Set );
    defaultInterpreter -> AddEntry( CFUNC_ENABLE_WEAPONS, CFUNC_Enable_Weapons );
    defaultInterpreter -> AddEntry( CFUNC_PRINTREG, CFUNC_PrintReg );
}

void RemoveMainInterpreter() {
    delete defaultInterpreter;
}

/// --- MAIN INJECT ---

void PSEVM_Inject() {
    for ( int i = 0; i < MAX_PSEVM_LINES; i++ ) {
        if ( psevm[ i ] ) {
            do {
                psevm[ i ] -> Process();
            } while ( parsingCritical );
        }
    }
}



