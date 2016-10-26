#include "replay_game.h"
#include "replay.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <assert.h>
#ifdef MJ_DEBUG
#include <imgui.h>
#endif

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

struct Demo {
    std::string name;
    unsigned int startFrame;
    unsigned int numFrames;
};

struct Debug {
    std::vector<Demo> demos;
    int selectedDemo;
    int demoSeekBar;
    bool looping;
    enum State {
        Idle,
        Recording,
        Playing,
        Paused
    };
    State state;
};

// Zero-initialized
struct GameData {
    bool initialized;
    float accumulator; // Stores deltaTime
    unsigned int numTicks;

    GameState currentState; // Only init this one
    GameState previousState;
    GameState lerpState; // TODO: This could be stored in previousState?

    // Render data
    sf::Texture textures[256];
    unsigned int numTextures;

#ifdef MJ_DEBUG
    Debug debug;
#endif
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

#ifdef MJ_DEBUG
static void DrawDebugMenu(GameState* gameState, Debug* debug) {
    // Keep demo around for looking up stuff
    static bool show_test_window = true;
    ImGui::ShowTestWindow(&show_test_window);

    ImGui::Begin("DrawDebugMenu");
    ImGui::Text("Hello World!");
    sf::Vector2f pos = gameState->player->getPosition();
    float v[] = { pos.x, pos.y };
    if (ImGui::InputFloat2("Player position", v, 3, ImGuiInputTextFlags_EnterReturnsTrue)) {
        gameState->player->setPosition(sf::Vector2f(v[0], v[1]));
    }
    ImGui::End();


    {
        ImGui::Begin("Demo Menu");

        ImGui::Text((std::string("Status: ") + std::to_string(debug->state)).c_str());

        // Demo recording/playback
        if (debug->demos.size() == 0) {
            debug->selectedDemo = -1;
        }

        // Get demo info
        int numTicks = 0;
        if (debug->selectedDemo != -1) {
            numTicks = debug->demos[debug->selectedDemo].numFrames;
        }

        ImGui::SliderInt("", &debug->demoSeekBar, 0, numTicks);
        ImGui::SameLine();
        ImGui::Checkbox("Loop", &debug->looping);
        if (ImGui::Button("Save Snapshot")) {
            // TODO
        }
        ImGui::SameLine();
        if (ImGui::Button("Take Over")) {
            // TODO
        }
        
        ImGui::PushID(0);
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(2/7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(2/7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(2/7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Play")) {
            // TODO
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        
        ImGui::SameLine();
        ImGui::PushID(1);
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0/7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0/7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0/7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Record")) {
            // TODO
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        
        ImGui::SameLine();
        ImGui::PushID(2);
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(1/7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(1/7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(1/7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Pause")) {
            // TODO
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            // TODO
        }

        // List demo's
        ImGui::Text((std::string("Stored demo's: ") + std::to_string(debug->demos.size())).c_str());
        ImGui::Columns(3, "Demo's");
        ImGui::Separator();
        ImGui::Text("Name"); ImGui::NextColumn();
        ImGui::Text("Start"); ImGui::NextColumn();
        ImGui::Text("Length"); ImGui::NextColumn();
        ImGui::Separator();
        for (int i = 0; i < debug->demos.size(); i++) {
            Demo* demo = debug->demos.data() + i;
            if (ImGui::Selectable(demo->name.c_str(), debug->selectedDemo == i, ImGuiSelectableFlags_SpanAllColumns)) {
                debug->selectedDemo = i;
            }
            ImGui::NextColumn();
            ImGui::Text(std::to_string(demo->startFrame).c_str()); ImGui::NextColumn();
            ImGui::Text(std::to_string(demo->numFrames).c_str()); ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::End();
    }
}
#endif

MJ_EXPORT(void) UpdateGame(float deltaTime, Memory* memory, sf::RenderWindow* window) {
    assert(memory);
    assert(window);

    GameData* gameData = (GameData*) memory->permanentStorage;
    MJControls* controls = &memory->controls;
#ifdef MJ_DEBUG
    ImGui::SetCurrentContext(memory->imguiState);
#endif

    if (!gameData->initialized) {
        Init(gameData, controls, window);
        gameData->initialized = true;
    }

    gameData->accumulator += deltaTime;
    while (gameData->accumulator >= TICK_TIME) {
        memcpy(&gameData->previousState, &gameData->currentState, sizeof(GameState));
        controls->BeginFrame();
        Simulate(&gameData->currentState, controls);
        controls->EndFrame();
        gameData->accumulator -= TICK_TIME;
        gameData->numTicks++;
    }

    float t = gameData->accumulator / TICK_TIME;
    Lerp(&gameData->lerpState, &gameData->previousState, &gameData->currentState, t);

    window->clear();
#ifdef MJ_DEBUG
    window->pushGLStates();
#endif
    Render(&gameData->lerpState, window);
#ifdef MJ_DEBUG
    window->popGLStates();
#endif

#ifdef MJ_DEBUG
    ImGui::GetIO().DeltaTime = deltaTime;
    ImGui::NewFrame();
    DrawDebugMenu(&gameData->currentState, &gameData->debug);
    ImGui::Render();
#endif

    window->display();
}

