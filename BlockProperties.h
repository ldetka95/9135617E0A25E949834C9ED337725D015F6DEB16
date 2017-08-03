#pragma once

#include "Textures.h"
#include <string>
#include <string.h>
using namespace std;
#include <map>
#include <math.h>

#define BLOCKPROP_PARSE_ERROR_UNDEFINED 1
#define BLOCKPROP_PARSE_ERROR_LACKARG 2

#define BLOCKPROP_LINE_BUFFER_LENGTH 2048

class BlockProperties {
    protected:
    BlockProperties( FILE* handle ) {
        ___parse( handle );
    }
    BlockProperties() {
    }
    public:
    static BlockProperties* create( string path ) {
        FILE* handle = fopen( path.c_str(), "r" );
        if ( handle ) {
            BlockProperties* prop = new BlockProperties( handle );
            fclose( handle );
            return prop;
        }
        return new BlockProperties();
    }
    ~BlockProperties() {
        map < int, ___SingleBlockProperties* >::iterator it = ___prop.begin();
        while ( it != ___prop.end() ) {
            delete it -> second;
            it++;
        }
    }
    double getWalkSpeedFactor( int index ) {
        ___SingleBlockProperties* p = ___get( index );
        if ( p == NULL ) {
            return NAN;
        }
        return p -> ___entity_speed_factor;
    }
    double getSlipFactor( int index ) {
        ___SingleBlockProperties* p = ___get( index );
        if ( p == NULL ) {
            return NAN;
        }
        return p -> ___entity_slip_factor;
    }
    double getTransparency( int index ) {
        ___SingleBlockProperties* p = ___get( index );
        if ( p == NULL ) {
            return NAN;
        }
        return p -> ___transparency_factor;
    }
    double isSolid( int index ) {
        ___SingleBlockProperties* p = ___get( index );
        if ( p == NULL ) {
            return NAN;
        }
        return ( double )( p -> ___solid ); // god dammit
    }
    double getLiquidSlowDown( int index ) {
        ___SingleBlockProperties* p = ___get( index );
        if ( p == NULL ) {
            return NAN;
        }
        return p -> ___liquid_slow_down;
    }
    private:
    class ___SingleBlockProperties {
        public:
        ___SingleBlockProperties( int id ) {
            ___entity_speed_factor = 1.0;
            ___entity_slip_factor = 0.0;
            ___solid = true;
            ___transparency_factor = 0.0;
            ___liquid_slow_down = 0.0;
            ___id = id;
        }
        int ___id;
        double ___entity_speed_factor;
        double ___entity_slip_factor;
        bool ___solid;
        double ___transparency_factor;
        double ___liquid_slow_down;
    };
    ___SingleBlockProperties* ___get( int index ) {
        if ( ___prop.find( index ) == ___prop.end() ) {
            return NULL;
        }
        return ___prop[ index ];
    }
    void ___set( ___SingleBlockProperties* prop ) {
        if ( prop ) {
            ___prop[ prop -> ___id ] = prop;
        }
    }
    map < int, ___SingleBlockProperties* > ___prop;
    int ___parse( FILE* handle ) {
        ___SingleBlockProperties* currentBlock = NULL;
        while ( !feof( handle ) ) {
            char line[ BLOCKPROP_LINE_BUFFER_LENGTH ];
            fgets( line, BLOCKPROP_LINE_BUFFER_LENGTH, handle );
            if ( !feof( handle ) ) {
                // debug
                //printf( "%s", line );
                // read
                char bufferedPart[ BLOCKPROP_LINE_BUFFER_LENGTH ];
                if ( sscanf( line, "%s=", bufferedPart ) > 0 ) {
                    const char* args = strstr( line, "=" ) + 1;
                    //printf( "> [%s]\n", args );
                    if ( strcmp( bufferedPart, "ID" ) == 0 ) {
                        int id = -1;
                        sscanf( args, "%d", &id );
                        if ( id >= 0 ) {
                            currentBlock = new ___SingleBlockProperties( id );
                            ___set( currentBlock );
                        } else {
                            return BLOCKPROP_PARSE_ERROR_LACKARG;
                        }
                    } else if ( strcmp( bufferedPart, "BLOCK_SPEED" ) == 0 ) {
                        sscanf( args, "%lf", &( currentBlock -> ___entity_speed_factor ) );
                    } else if ( strcmp( bufferedPart, "BLOCK_SLIP" ) == 0 ) {
                        sscanf( args, "%lf", &( currentBlock -> ___entity_slip_factor ) );
                    } else if ( strcmp( bufferedPart, "BLOCK_TRANSPARENCY" ) == 0 ) {
                        sscanf( args, "%lf", &( currentBlock -> ___transparency_factor ) );
                    } else if ( strcmp( bufferedPart, "BLOCK_IS_SOLID" ) == 0 ) {
                        currentBlock -> ___solid = ( bool )( sscanf( args, "TRUE" ) );
                    } else if ( strcmp( bufferedPart, "BLOCK_LIQUID_SLOWDOWN" ) == 0 ) {
                        sscanf( args, "%lf", &( currentBlock -> ___liquid_slow_down ) );
                    } else {
                        return BLOCKPROP_PARSE_ERROR_UNDEFINED;
                    }
                }
            }
        }
        return 0;
    }
};

BlockProperties* blockProperties = NULL;

void PassTextures() {
    for ( int i = 0; i < MAX_TEXTURES; i++ ) {
        if ( image[ i ] ) {
            double transparency = blockProperties -> getTransparency( i );
            if ( ( isnan( transparency ) ) || ( transparency <= 1E-11 ) ) {
                continue;
            }
            image[ i ] -> makeTransparent32( transparency );
        }
    }
}

void InitBlockProperties() {
    blockProperties = BlockProperties::create( "data/globalconfig/blockProperties.dat" );
    PassTextures();
}

void DestroyBlockProperties() {
    if ( blockProperties ) {
        delete blockProperties;
        blockProperties = NULL;
    }
}

