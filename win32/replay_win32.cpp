#include <SFML/Graphics.hpp>
#include <windows.h>
#include <process.h> // _beginthreadex
#include <atomic>
#include <stdint.h>
#include <queue>
#include <mutex>

#include "replay.h"

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

static sf::RenderWindow* s_renderWindow;
static LARGE_INTEGER s_lastTime;
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
    void* permanentStorage; // This gets casted to GameMemory

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

unsigned int GameMain(void*) {
    sf::Texture texture1;
    if (!texture1.loadFromFile("../assets/test.png")) {
        return EXIT_FAILURE;
    }
    sf::Sprite sprite1(texture1);
    sprite1.setOrigin(sf::Vector2f(texture1.getSize()) / 2.f);
    sprite1.setPosition(sprite1.getOrigin());
    sprite1.setScale(5.0f, 5.0f);

    while (s_running.load()) {
        ParseMessages();
        //g_renderApi->ImGuiNewFrame();
        //s_gameFuncs.UpdateGame(&gameInfo);

        s_renderWindow->clear();
        s_renderWindow->draw(sprite1);
        s_renderWindow->display();
    }


    return EXIT_SUCCESS;
}

int main() {
    QueryPerformanceFrequency(&s_perfFreq);

    HINSTANCE instance = GetModuleHandle(NULL);

    auto className = L"StarlightClassName";

    WNDCLASSEXW wndClass = { 0 };
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
    RECT windowRect = { 0, 0, 800, 600 }; // TODO: Config file?
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

    HWND hwnd = CreateWindowExW(
        0L,
        className,
        L"Starlight",
        WS_OVERLAPPEDWINDOW,
        x,
        y,
        windowWidth,
        windowHeight,
        NULL,
        NULL,
        GetModuleHandleW(NULL),
        NULL
    );

    s_renderWindow = new sf::RenderWindow(hwnd);

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
    HANDLE thread = (HANDLE) _beginthreadex(NULL, 0, GameMain, NULL, 0, NULL);
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

/*
    MSG message;
    message.message = static_cast<UINT>(~WM_QUIT);
    while (message.message != WM_QUIT) {
        if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        } else {
            
        }
    }
*/

    // Close thread
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);

    // Close window
    DestroyWindow(hwnd);
    UnregisterClass(className, instance);
    delete s_renderWindow;

    return EXIT_SUCCESS;
}
