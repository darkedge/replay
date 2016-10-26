#pragma once
#include "replay.h"
#include "mj_controls.h"
#include <stdint.h>

// Specify tick rate in ms
// 15 ms = 66.66
// 30 ms = 33.33
// 60 ms = 16.66
// 120 ms = 8.33
#define TICK_TIME 0.120f

// Specify tick rate in ticks
//#define TICK_RATE 20
//#define TICK_TIME (1.0f / TICK_RATE)

namespace sf {
	class RenderWindow;
}

struct PlatformAPI {
    // Platform functions go here
};

struct ImGuiContext;

struct Memory {
    uint64_t permanentStorageSize;
    void* permanentStorage; // This gets casted to GameState

    ImGuiContext* imguiState;
    PlatformAPI platformAPI;

    MJControls controls;
};

#define GAME_UPDATE(name) void name(float, Memory*, sf::RenderWindow*)
typedef GAME_UPDATE(UpdateGameFunc);

#define GAME_DEBUG(name) void name(Memory*)
typedef GAME_DEBUG(DebugGameFunc);
