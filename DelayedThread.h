#pragma once
#include <windows.h>

class DelayedThread {
    public:
    DelayedThread( int initialState, void ( *func )( DelayedThread* ) ) {
        ___func = func;
        ___thread = CreateThread( NULL, 0, ___do_Threading, this, 0, NULL );
    }
    ~DelayedThread() {
        CloseHandle( ___thread );
    }
    bool stateEquals( int state ) {
        return ( ___state == state );
    }
    void setState( int state ) {
        ___state = state;
    }
    private:
    static DWORD WINAPI ___do_Threading( void* vcaller ) {
        DelayedThread* caller = ( ( DelayedThread* )( vcaller ) );
        caller -> ___func( caller );
        return 0;
    }
    int ___state;
    HANDLE ___thread;
    void ( *___func )( DelayedThread* );
};
