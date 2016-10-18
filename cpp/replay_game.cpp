#include "replay_game.h"
#include "replay.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <assert.h>
#include <imgui.h>

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
    GameState lerpState; // TODO: This could be stored in previousState?

    // Render data
    sf::Texture textures[256];
    unsigned int numTextures;
};

static sf::Sprite* CreateSprite(GameData* gameState, const char* filename) {
    assert(gameState);
    assert(filename);

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

static void Destroy(GameData* gameData, sf::RenderWindow* window) {
    assert(gameData);
    assert(window);

    // TODO
}

static void Init(GameData* gameData, MJControls* controls, sf::RenderWindow* window) {
    assert(gameData);
    assert(controls);
    assert(window);

    GameState* gameState = &gameData->currentState;
    gameState->player = CreateSprite(gameData, "../assets/PNG/ufoBlue.png");
    gameState->player->setPosition(gameState->player->getOrigin());

    controls->AssociateKey(MJControls::Left, "Left");
    controls->AssociateKey(MJControls::Right, "Right");
    controls->AssociateKey(MJControls::Up, "Up");
    controls->AssociateKey(MJControls::Down, "Down");
}

// This runs at a fixed FPS
static void Simulate(GameState* gameState, MJControls* controls) {
    assert(gameState);
    assert(controls);

    gameState->time += TICK_TIME;
    gameState->player->setRotation(gameState->time * 50.0f);

    sf::Sprite* player = gameState->player;
    sf::Vector2f position = player->getPosition();
    // This misses inputs!!
    if (controls->GetButton("Left")) {
        position.x -= TICK_TIME * 300.0f;
    }
    if (controls->GetButton("Right")) {
        position.x += TICK_TIME * 300.0f;
    }
    if (controls->GetButton("Up")) {
        position.y -= TICK_TIME * 300.0f;
    }
    if (controls->GetButton("Down")) {
        position.y += TICK_TIME * 300.0f;
    }
    player->setPosition(position);

    static bool show_test_window = true;
    ImGui::ShowTestWindow(&show_test_window);
}

// Does simple rendering
static void Render(GameState* gameState, sf::RenderWindow* window) {
    assert(gameState);
    assert(window);

    // TODO: Sprite sorting, double pointers
    for (unsigned int i = 0; i < gameState->numSprites; i++) {
        window->draw(gameState->sprites[i]);
    }
}

template <typename T>
static T Lerp(T v0, T v1, float t) {
    return v0 + t * (v1 - v0);
}

// Runs once per frame, creates an interpolated state
static void Lerp(GameState* dst, GameState* src0, GameState* src1, float t) {
    assert(dst);
    assert(src0);
    assert(src1);
    assert(t >= 0.0f);
    assert(t <= 1.0f);

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
    MJControls* controls = &memory->controls;
    ImGui::SetCurrentContext(memory->imguiState);

    if (!gameData->initialized) {
        Init(gameData, controls, window);
        gameData->initialized = true;
    }

    static float accumulator;
    accumulator += dt;
    while (accumulator >= TICK_TIME) {
        memcpy(&gameData->previousState, &gameData->currentState, sizeof(GameState));
        ImGui::NewFrame();
        controls->BeginFrame();
        Simulate(&gameData->currentState, controls);
        controls->EndFrame();
        accumulator -= TICK_TIME;
    }

    float t = accumulator / TICK_TIME;
    Lerp(&gameData->lerpState, &gameData->previousState, &gameData->currentState, t);

    window->clear();
    window->pushGLStates();
    Render(&gameData->lerpState, window);
    window->popGLStates();

    ImGui::Render();
    window->display();
}

