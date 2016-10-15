#include "replay_game.h"
#include "replay.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <assert.h>

struct GameState {
	// Used for update, not interpolated
	float time;
	sf::Sprite* player;

	// To be interpolated
	sf::Sprite sprites[256];
	unsigned int numSprites;
	//sf::Text texts[16];
	//unsigned int numTexts;
};

// Zero-initialized
struct GameData {
	bool initialized;

	GameState currentState; // Only init this one
	GameState previousState;
	GameState lerpState;

	// Render data
	sf::Texture textures[256];
	unsigned int numTextures;
};

static sf::Sprite* CreateSprite(GameData* gameState, const char* filename) {
	// Create texture
	sf::Texture* texture = gameState->textures + gameState->numTextures++;
	new (texture) sf::Texture();
	texture->loadFromFile(filename);
	texture->setSmooth(true);

	// Create sprite
	sf::Sprite* sprite = gameState->currentState.sprites + gameState->currentState.numSprites++;
	new (sprite) sf::Sprite(*texture);
	sprite->setOrigin(sf::Vector2f(texture->getSize()) / 2.0f);

	return sprite;
}

// No static variables here, everything is in GameData

static void Init(GameData* gameData, sf::RenderWindow* window) {
	GameState* gameState = &gameData->currentState;
	gameState->player = CreateSprite(gameData, "../assets/PNG/ufoBlue.png");
    gameState->player->setPosition(gameState->player->getOrigin());
    //gameState->player->setScale(1.0f, 1.0f);
}

// This runs at a fixed FPS
static void Simulate(GameState* gameState) {
	gameState->time += TICK_TIME;
	gameState->player->setRotation(gameState->time * 50.0f);
}

// Does simple rendering
static void Render(GameState* gameState, sf::RenderWindow* window) {
	// TODO: Sprite sorting, double pointers
	window->clear();
	for (unsigned int i = 0; i < gameState->numSprites; i++) {
		window->draw(gameState->sprites[i]);
	}
	window->display();
}

template <typename T>
static T Lerp(T v0, T v1, float t) {
	return v0 + t * (v1 - v0);
}

// Runs once per frame, creates an interpolated state
static void Lerp(GameState* dst, GameState* src0, GameState* src1, float t) {
	// First do a memcpy because of extra sfml data
	memcpy(dst, src0, sizeof(GameState));

	// Interpolate sprites
	// How to deal with removed entities?
	// Just use t0's sprite count for now
	for (unsigned int i = 0; i < src0->numSprites; i++) {
		dst->sprites[i].setPosition(Lerp(
			src0->sprites[i].getPosition(),
			src1->sprites[i].getPosition(),
			t));

		{
			// SFML wraps rotation between 0 and 360
			float f0 = src0->sprites[i].getRotation();
			float f1 = src1->sprites[i].getRotation();
			if (f1 < f0) {
				f1 += 360.0f;
			}
			dst->sprites[i].setRotation(Lerp(f0, f1, t));
		}
	}
}

MJ_EXPORT(void) UpdateGame(float dt, Memory* memory, sf::RenderWindow* window) {
	assert(memory);
	assert(window);
	GameData* gameData = (GameData*) memory->permanentStorage;

	sf::Event event;
	while (window->pollEvent(event)) {
		switch (event.type) {
			case sf::Event::KeyPressed: {
				gameData->initialized = false;
				break;
			}
		}
	}

	if (!gameData->initialized) {
		Init(gameData, window);
		gameData->initialized = true;
	}

	static float accumulator;
	accumulator += dt;
	while (accumulator >= TICK_TIME) {
		memcpy(&gameData->previousState, &gameData->currentState, sizeof(GameState));
		Simulate(&gameData->currentState);
		accumulator -= TICK_TIME;
	}

	float t = accumulator / TICK_TIME;
	Lerp(&gameData->lerpState, &gameData->previousState, &gameData->currentState, t);

	Render(&gameData->lerpState, window);
}
