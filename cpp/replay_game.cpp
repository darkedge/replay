#include "replay.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <assert.h>

// Zero-initialized
struct GameState {
	bool initialized;
	sf::Sprite* sprite1;
	float time;

	// Render data
	sf::Texture textures[256];
	unsigned int numTextures;
	sf::Sprite sprites[256];
	unsigned int numSprites;
	sf::Text texts[16];
	unsigned int numTexts;
};

static sf::Sprite* CreateSprite(GameState* gameState, const char* filename) {
	// Create texture
	sf::Texture* texture = gameState->textures + gameState->numTextures++;
	new (texture) sf::Texture();
	texture->loadFromFile(filename);
	texture->setSmooth(true);

	// Create sprite
	sf::Sprite* sprite = gameState->sprites + gameState->numSprites++;
	new (sprite) sf::Sprite(*texture);
	sprite->setOrigin(sf::Vector2f(texture->getSize()) / 2.0f);

	return sprite;
}

// No static variables here, everything is in GameState

static void Init(GameState* gameState, sf::RenderWindow* window) {
	gameState->sprite1 = CreateSprite(gameState, "../assets/PNG/ufoBlue.png");
    gameState->sprite1->setPosition(gameState->sprite1->getOrigin());
    gameState->sprite1->setScale(1.0f, 1.0f);
}

MJ_EXPORT(void) UpdateGame(void* memory, sf::RenderWindow* window) {
	assert(memory);
	assert(window);
	GameState* gameState = (GameState*) memory;

	sf::Event event;
	while (window->pollEvent(event)) {
		switch (event.type) {
			case sf::Event::KeyPressed: {
				gameState->initialized = false;
				break;
			}
		}
	}

	if (!gameState->initialized) {
		Init(gameState, window);
		gameState->initialized = true;
	}

	gameState->time += TICK_TIME;
	gameState->sprite1->setRotation(gameState->time * 0.5f);

	// Rendering
	// TODO: Sprite sorting, double pointers
	for (unsigned int i = 0; i < gameState->numSprites; i++) {
		window->draw(gameState->sprites[i]);
	}
}
