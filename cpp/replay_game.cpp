#include "replay.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <assert.h>

// Zero-initialized
struct GameState {
	bool initialized;
	sf::Texture texture1;
	sf::Sprite sprite1;
	float time;
};

// No static variables here, everything is in GameState

static void Init(GameState* gameState, sf::RenderWindow* window) {
	new (&gameState->texture1) sf::Texture();
    if (!gameState->texture1.loadFromFile("../assets/test.png")) {
        return;
    }
	new (&gameState->sprite1) sf::Sprite(gameState->texture1);
    gameState->sprite1.setOrigin(sf::Vector2f(gameState->texture1.getSize()) / 2.f);
    gameState->sprite1.setPosition(gameState->sprite1.getOrigin());
    gameState->sprite1.setScale(5.0f, 5.0f);
}

MJ_EXPORT(void) UpdateGame(void* memory, sf::RenderWindow* window) {
	assert(memory);
	assert(window);
	GameState* gameState = (GameState*) memory;
	if (!gameState->initialized) {
		Init(gameState, window);
		gameState->initialized = true;
	}
	//gameState->time += TICK_TIME;
	//gameState->sprite1.setRotation(gameState->time * 0.1f);
	window->draw(gameState->sprite1);
}
