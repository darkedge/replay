#pragma once
#include "replay.h"

namespace sf {
	class RenderWindow;
}

#define GAME_UPDATE(name) void name(void*, sf::RenderWindow*)
typedef GAME_UPDATE(UpdateGameFunc);

MJ_EXPORT(void) UpdateGame(void*, sf::RenderWindow*);
