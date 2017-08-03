#pragma once

#include "Core.h"

DelayedThread* delayedLoader = NULL;
#define LOADER_STATE_INITIALIZED 0
#define LOADER_STATE_PROCESSING 1
#define LOADER_STATE_FINISHED 2

void Delayed_Level( DelayedThread* caller ) {
    caller -> setState( LOADER_STATE_PROCESSING );
    MENU_loading_start_time = Timer::Current();
    Level_Init();
    MENU_loading_start_time = -1.0;
    caller -> setState( LOADER_STATE_FINISHED );
}

void Delayed_Level_MP( DelayedThread* caller ) {
    caller -> setState( LOADER_STATE_PROCESSING );
    MENU_loading_start_time = Timer::Current();
    Level_Init_MP();
    MENU_loading_start_time = -1.0;
    caller -> setState( LOADER_STATE_FINISHED );
}
