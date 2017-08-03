#pragma once

#include "pse2_main.h"
//#include "pse2_alloctypes.h"





#include <stdio.h>

//#define MAX_HDD_STRUCT_ARGS 64

struct ___PSE2_HDDStruct {
    public:
    int Buffer_Code;
    int Buffer_ArgNum;
    //double Buffer_Arg[ MAX_HDD_STRUCT_ARGS ];
};

typedef struct ___PSE2_HDDStruct PSE2_HDDStruct;

PSE2_Line* PSE2_LoadFrom( const char* path, PSE2_Interpreter* interpreter = NULL ) {
    FILE* handle = fopen( path, "rb" );
    PSE2_Line* line = new PSE2_Line( interpreter );
    if ( handle ) {
        PSE2_HDDStruct hstruct;
        while ( fread( &hstruct, sizeof( hstruct ), 1, handle ) > 0 ) {
            PSE2_Argument* args = new PSE2_Argument();
            for ( int i = 0; i < hstruct.Buffer_ArgNum; i++ ) {
                double i_argument;
                fread( &i_argument, sizeof( i_argument ), 1, handle );
                args -> AppendArgument( i_argument );
            }
            line -> Append( hstruct.Buffer_Code, args );
            delete args;
        }
        fclose( handle );
    }
    return line;
}



