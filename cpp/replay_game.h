#pragma once
#include "replay.h"
#include <stdint.h>

namespace sf {
	class RenderWindow;
}

struct PlatformAPI {
    // Platform functions go here
};

struct Memory {
    uint64_t permanentStorageSize;
    void* permanentStorage; // This gets casted to GameState

    PlatformAPI platformAPI;
};

#define GAME_UPDATE(name) void name(float, Memory*, sf::RenderWindow*)
typedef GAME_UPDATE(UpdateGameFunc);
