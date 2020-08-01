#include "game.hpp"
#include "hook.hpp"
#include "main.hpp"
#include "resource.hpp"

#include <chrono>
#include <sstream>
#include <thread>

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	CoInitialize(nullptr);

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"YellowRectangleCyanCircleApplication";
    wcex.hIconSm = nullptr;

    RegisterClassExW(&wcex);

    HWND hWnd = CreateWindow(L"YellowRectangleCyanCircleApplication", L"Hello, world1", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 200, 200, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return -1;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    // wchar_t gameWindowName[256];
    // LoadString(hInstance, IDS_GAME_WINDOW_NAME, gameWindowName, 256);
    std::wstring gameWindowName = L"WindowsProject1234567";
    
    using namespace YellowRectangleCyanCircle;

    auto winAPI = std::make_shared<WinAPI>();

    auto h = winAPI->FindWindowByName(gameWindowName);
    auto game = Game(winAPI, gameWindowName, h);

    auto hookCW = Hook(winAPI, EVENT_OBJECT_CREATE, 0, false, true);
    auto hookDW = Hook(winAPI, EVENT_OBJECT_DESTROY, h, true, false);
    auto hookMW = Hook(winAPI, EVENT_OBJECT_LOCATIONCHANGE, h, true, true);

    // auto game = Game(winAPI, 

    hookCW.SetCallback([&hookCW, &hookDW, &hookMW, &game](HWND hWnd, LONG idObject) {
        game.OnWindowCreated(hWnd);
        if (game.IsFound()) {
            OutputDebugString(L"Game found with handle: ");
            OutputDebugString(std::data(std::to_wstring(reinterpret_cast<std::uint32_t>(hWnd))));
            OutputDebugString(L"\n");

            hookCW.Disable();

            hookDW.SetHandle(hWnd);
            hookDW.Enable();

            hookMW.SetHandle(hWnd);
            hookMW.Enable();
        }
    });

    hookDW.SetCallback([&hookCW, &hookDW, &hookMW, &game](HWND hWnd, LONG idObject) {
        game.OnWindowDestroyed(hWnd);
        if (!game.IsFound()) {
            OutputDebugString(L"Game destroyed\n");
            hookDW.Disable();
            hookMW.Disable();
            hookCW.Enable();
        }
    });

    hookMW.SetCallback([&hookMW, &game](HWND hWnd, LONG idObject) {
        if (idObject == OBJID_CURSOR) return;

        game.OnWindowMoved(hWnd);
        if (game.IsFound()) {
            auto rect = game.GetRect();

            std::wstringstream buf;
            buf << "Game moved: Rect(" << rect.x << ", " << rect.y << ", " << rect.width << ", " << rect.height << ")" << std::endl;
            OutputDebugString(std::data(buf.str()));
        }
    });

    if (game.IsFound()) {
        hookDW.SetHandle(h);
        hookMW.SetHandle(h);

        hookDW.Enable();
        hookMW.Enable();
    } else {
        hookCW.Enable();
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	CoUninitialize();
	return 0;
}
