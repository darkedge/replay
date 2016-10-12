#include <SFML/Graphics.hpp>
#include <windows.h>
#include <cmath>

static sf::RenderWindow* s_renderWindow;

LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_SIZE:
        {
            if (s_renderWindow) {
                s_renderWindow->setSize(sf::Vector2u(LOWORD(lParam), HIWORD(lParam)));
                s_renderWindow->setView(sf::View(sf::FloatRect(0.0f, 0.0f, (float)LOWORD(lParam), (float)HIWORD(lParam))));
            }
            break;
        }
    }

    return DefWindowProc(handle, message, wParam, lParam);
}

int main()
{
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
    if (wcscmp(computerName, L"JOTARO") == 0 ||
        wcscmp(computerName, L"JOSUKE") == 0) {
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

    sf::Texture texture1;
    if (!texture1.loadFromFile("../assets/test.png")) {
        return EXIT_FAILURE;
    }
    sf::Sprite sprite1(texture1);
    sprite1.setOrigin(sf::Vector2f(texture1.getSize()) / 2.f);
    sprite1.setPosition(sprite1.getOrigin());
    sprite1.setScale(5.0f, 5.0f);

    sf::Clock clock;

    MSG message;
    message.message = static_cast<UINT>(~WM_QUIT);
    while (message.message != WM_QUIT)
    {
        if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        else
        {
            float time = clock.getElapsedTime().asSeconds();

            s_renderWindow->clear();

            sprite1.setRotation(time * 100);
            s_renderWindow->draw(sprite1);

            s_renderWindow->display();
        }
    }

    DestroyWindow(hwnd);
    UnregisterClass(className, instance);

    delete s_renderWindow;

    return EXIT_SUCCESS;
}
