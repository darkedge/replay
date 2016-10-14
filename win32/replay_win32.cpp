#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <windows.h>
#include <process.h> // _beginthreadex
#include <atomic>
#include <stdint.h>
#include <queue>
#include <mutex>

#include "replay.h"
#include "replay_game.h"

class MSGQueue {
public:
    void Enqueue(MSG msg) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(msg);
    }
    bool Dequeue(MSG* msg) {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty()) {
            return false;
        }
        *msg = queue.front();
        queue.pop();
        return true;
    }
private:
    std::queue<MSG> queue;
    std::mutex mutex;
};

static const char* s_dllName = "replay.dll";

static HWND s_hwnd;
static LARGE_INTEGER s_lastDLLLoadTime;
static sf::RenderWindow* s_renderWindow;
static LARGE_INTEGER s_perfFreq;
static std::atomic_bool s_running;
static MSGQueue s_queue;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

struct PlatformAPI {
    // Platform functions go here
};

struct Memory {
    uint64_t permanentStorageSize;
    void* permanentStorage; // This gets casted to GameState

    PlatformAPI platformAPI;
};

void ParseMessages() {
    MSG msg;
    while (s_queue.Dequeue(&msg)) {
/*
        if (g_renderApi) {
            g_renderApi->ImGuiHandleEvent(&msg);
        }
        switch (msg.message)
        {
        case WM_KEYDOWN:
            if (msg.wParam < NUM_KEYBOARD_KEYS) {
                s_controls.ChangeKeyState((int)msg.wParam, true);
            }
            break;
        case WM_KEYUP:
            if (msg.wParam < NUM_KEYBOARD_KEYS) {
                s_controls.ChangeKeyState((int)msg.wParam, false);
            }
            break;
        }
*/
    }
}

LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
    s_queue.Enqueue({ handle, message, wParam, lParam });
    switch (message) {
        case WM_CLOSE: {
            s_running.store(false);
            PostQuitMessage(0);
            return 0;
        }
        case WM_SIZE: {
            if (s_renderWindow) {
                s_renderWindow->setSize(sf::Vector2u(LOWORD(lParam), HIWORD(lParam)));
                s_renderWindow->setView(sf::View(sf::FloatRect(0.0f, 0.0f, (float)LOWORD(lParam), (float)HIWORD(lParam))));
            }
            break;
        }
    }

    return DefWindowProc(handle, message, wParam, lParam);
}

inline FILETIME Win32GetLastWriteTime(char *filename) {
    FILETIME lastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExA(filename, GetFileExInfoStandard, &data)) {
        lastWriteTime = data.ftLastWriteTime;
    }

    return lastWriteTime;
}

struct GameFuncs {
    UpdateGameFunc* UpdateGame;
    FILETIME DLLLastWriteTime;
    bool valid;
    HMODULE dll;
};

static void UnloadGameFuncs(GameFuncs* gameFuncs) {
    if (gameFuncs->dll) {
        FreeLibrary(gameFuncs->dll);
    }

    memset(gameFuncs, 0, sizeof(GameFuncs));
}

static GameFuncs LoadGameFuncs() {
    GameFuncs gameFuncs = {};

    WIN32_FILE_ATTRIBUTE_DATA foo;
    if (!GetFileAttributesExA("lock.tmp", GetFileExInfoStandard, &foo))
    {
        gameFuncs.DLLLastWriteTime = Win32GetLastWriteTime((char*)s_dllName);
        QueryPerformanceCounter(&s_lastDLLLoadTime);
        CopyFileA(s_dllName, "replay_temp.dll", FALSE);

        gameFuncs.dll = LoadLibraryA("replay_temp.dll");
        if (gameFuncs.dll) {
            gameFuncs.UpdateGame = (UpdateGameFunc*)
                GetProcAddress(gameFuncs.dll, "UpdateGame");
            
            gameFuncs.valid = !!gameFuncs.UpdateGame;
        }
    }

    // If any functions fail to load, set all to 0
    if (!gameFuncs.valid) {
        gameFuncs.UpdateGame = 0;
    }

    return gameFuncs;
}

struct GameMainParams {
    Memory* memory;
};

unsigned int GameMain(void* gameParams) {
    GameMainParams* params = (GameMainParams*) gameParams;

    {
        sf::ContextSettings settings;
        s_renderWindow = new sf::RenderWindow(s_hwnd, settings);
    }

    GameFuncs gameFuncs = LoadGameFuncs();

    LARGE_INTEGER lastTime;
    QueryPerformanceCounter(&lastTime);

    float accumulator = 0.0f;

    while (s_running.load()) {
        ParseMessages();
        //g_renderApi->ImGuiNewFrame();
        //gameFuncs.UpdateGame(&gameInfo);

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float deltaTime = float(now.QuadPart - lastTime.QuadPart) / float(s_perfFreq.QuadPart);

        {
            // Reload DLL
            FILETIME NewDLLWriteTime = Win32GetLastWriteTime((char*)s_dllName);

            if (CompareFileTime(&NewDLLWriteTime, &gameFuncs.DLLLastWriteTime) != 0)
            {
                UnloadGameFuncs(&gameFuncs);
                for(int i = 0; !gameFuncs.valid && (i < 100); i++)
                {
                    gameFuncs = LoadGameFuncs();
                    Sleep(100);
                }
            }
        }

        s_renderWindow->clear();

        
        // Breakpoint guard
        if (deltaTime > 1.0f) {
            deltaTime = 1.0f / 60.0f;
        }
        lastTime = now;

        accumulator += deltaTime;
        //if (accumulator > TICK_TIME) {
        //    accumulator -= TICK_TIME;
            gameFuncs.UpdateGame(params->memory->permanentStorage, s_renderWindow);
        //}

        s_renderWindow->display();
    }

    return EXIT_SUCCESS;
}

// Win32 GUI application signature
// This makes the command line return immediately after starting the application
// One disadvantage is that printf() logging is now disabled
int CALLBACK WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow) {
    QueryPerformanceFrequency(&s_perfFreq);

    HINSTANCE instance = GetModuleHandle(NULL);

    auto className = L"ReplayClassName";

    WNDCLASSEXW wndClass = {};
    wndClass.cbSize = sizeof(WNDCLASSEXW);
    wndClass.style = CS_CLASSDC;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = GetModuleHandleW(NULL);
    wndClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wndClass.lpszClassName = className;

    RegisterClassExW(&wndClass);

    // Get desktop rectangle
    RECT desktopRect;
    GetClientRect(GetDesktopWindow(), &desktopRect);

    // Get window rectangle
    RECT windowRect = { 0, 0, 1280, 720 }; // TODO: Config file?
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Calculate window dimensions
    LONG windowWidth = windowRect.right - windowRect.left;
    LONG windowHeight = windowRect.bottom - windowRect.top;
    LONG x = desktopRect.right / 2 - windowWidth / 2;
    LONG y = desktopRect.bottom / 2 - windowHeight / 2;

#ifdef MJ_DEBUG
    // Move the screen to the right monitor on JOTARO
    wchar_t computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize = sizeof(computerName);
    GetComputerNameW(computerName, &dwSize);
    if (wcscmp(computerName, L"JOTARO") == 0) {
        x += 1920;
    }
#endif

    s_hwnd = CreateWindowExW(
        0L,
        className,
        L"Replay",
        WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
        x,
        y,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        GetModuleHandleW(NULL),
        NULL
    );

#ifdef MJ_DEBUG
    LPVOID baseAddress = (LPVOID)Terabytes(2);
#else
    LPVOID baseAddress = 0;
#endif

    Memory memory = {};
    memory.permanentStorageSize = Megabytes(256);
    memory.permanentStorage = VirtualAlloc(baseAddress, (size_t) memory.permanentStorageSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    // Spawn game thread
    s_running.store(true);
	GameMainParams params = { &memory };
    HANDLE thread = (HANDLE) _beginthreadex(NULL, 0, GameMain, &params, 0, NULL);
    if (!thread) {
        return EXIT_FAILURE;
    }

    // Message loop
    MSG msg;
    while (s_running.load() && GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Close thread
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);

    // Close window
    DestroyWindow(s_hwnd);
    UnregisterClass(className, instance);
    delete s_renderWindow;

    return EXIT_SUCCESS;
}
