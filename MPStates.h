#pragma once

#define MAP_PACKAGE_SIZE 256
#define MAX_PACKET_SIZE 1280

class Message {
    public:
    Message( int size, int code, void* data ) {
        //printf( " > message %dB, code %d\n", size, code );
        ___size = size;
        ___code = code;
        if ( ___size > 0 ) {
            memcpy( ___data, data, ___size );
        }
    }
    ~Message() {
    }
    void* getData() {
        return ___data;
    }
    int getSize() {
        return ___size;
    }
    int getCode() {
        return ___code;
    }
    void* getCodePointer() {
        return &___code;
    }
    private:
    int ___size;
    int ___code;
    unsigned char ___data[ MAX_PACKET_SIZE ];
};

#define MESSAGE_CODE_TEST 1

#define MESSAGE_CODE_MAP_REQUEST 1001
#define MESSAGE_CODE_MAP_FINISHED 1002
#define MESSAGE_CODE_MAP_PROPERTIES 1003
#define MESSAGE_CODE_MAP_PART 1004

#define MESSAGE_CODE_POSITION 2001
#define MESSAGE_CODE_CAMERA 2002

#define MESSAGE_CODE_INPUT_KEY 2003
#define MESSAGE_CODE_INPUT_MOUSE 2004

#define MESSAGE_CODE_MOVE_REQUEST 2010

class TestState {
};

class MapProperties {
    public:
    int sizeX;
    int sizeY;
    int sizeZ;
    double blockSize;
};

class MapPart {
    public:
    int begin;
    int length;
    int data[ MAP_PACKAGE_SIZE ];
};

class PositionInfo {
    public:
    float x, y, z;
    float lx, ly, lz;
    int id;
};

class CameraInfo {
    public:
    double a, i;
};

class InputKey {
    public:
    char key;
    char pressed;
};

class InputMouse {
    public:
    short moveX, moveY;
    char button;
    char pressed;
};

class MoveRequest {
    public:
    float x, y, z;
    float lx, ly, lz;
};

class MyInfo {
    public:
    int id;
};
