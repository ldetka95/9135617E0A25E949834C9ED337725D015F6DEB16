#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <string>

using namespace std;

class ___StringLinkerField {
    public:
    ___StringLinkerField() {
        id = -1;
        s = "";
    }
    ~___StringLinkerField() {
    }
    void Read( FILE* handle ) {
        /* Expects:
                4 bytes : id
                2 bytes : n ( string length )
                n bytes : s
        */
        fread( &id, sizeof( id ), 1, handle );
        unsigned short int stringSize;
        fread( &stringSize, sizeof( stringSize ), 1, handle );
        char* buffer = ( char* )( calloc( ( stringSize ), sizeof( char ) ) );
        fread( buffer, stringSize * sizeof( char ), 1, handle );
        s.append( buffer, stringSize );
        free( buffer );
    }
    int id;
    string s;
};


class StringLinker {
    public:
    StringLinker( string path ) {
        array = NULL;
        size = 0;
        FILE* handle = fopen( path.c_str(), "rb" );
        if ( handle ) {
            fread( &size, sizeof( size ), 1, handle ); // And get the size of size, genius! :D
            array = ( ___StringLinkerField* (*)[] )( calloc( size, sizeof( ___StringLinkerField* ) ) );
            for ( int i = 0; i < size; i++ ) {
                ( *array )[ i ] = new ___StringLinkerField();
                ( *array )[ i ] -> Read( handle );
                // printf( "   %d : %s\n", ( *array )[ i ] -> id, ( ( *array )[ i ] -> s ).c_str() );
            }
            fclose( handle );
        } else {
            // printf( "Stringlinker cannot open: %s\n", path.c_str() );
        }
    }
    ~StringLinker() {
        if ( array ) {
            for ( int i = 0; i < size; i++ ) {
                delete ( *array )[ i ];
            }
            free( array );
        }
    }
    int GetIdOf( int index ) {
        if ( array ) {
            return ( *array )[ index ] -> id;
        }
        return -1;
    }
    string GetStringOf( int index ) {
        if ( array ) {
            return ( *array )[ index ] -> s;
        }
        return "";
    }
    string GetString( int requiredID ) {
        if ( array ) {
            for ( int i = 0; i < size; i++ ) {
                if ( ( *array )[ i ] -> id == requiredID ) {
                    return ( *array )[ i ] -> s;
                }
            }
        }
        return "";
    }
    int GetId( string searchedString ) {
        if ( array ) {
            for ( int i = 0; i < size; i++ ) {
                if ( ( *array )[ i ] -> s == searchedString ) {
                    return ( *array )[ i ] -> id;
                }
            }
        }
        return -1;
    }
    int GetSize() {
        return size;
    }
    private:
    ___StringLinkerField* ( *array )[] = NULL;
    int size;
};


