#pragma once

#include "Map.h"

class BinaryMap {
    private:
    struct _Header {
        public:
        int x, y, z;
    };
    protected:
    BinaryMap() {
        memset( &_h, 0, sizeof( _h ) );
        _d = NULL;
    }
    public:
    ~BinaryMap() {
        if ( _d ) {
            free( _d );
        }
    }
    static BinaryMap* load( string path ) {
        return NULL; // not yet;
    }
    Map* convertToMap() {
        Map* m = new Map();
        return m;
    }
    private:
    _Header _h;
    Uint8* _d;
};
