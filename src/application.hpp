#pragma once

#include "exception.hpp"
#include "resource.hpp"

#include <Windows.h>

#include <string>

namespace YellowRectangleCyanCircle {
    class Application {
    public:
        Application(HINSTANCE hInstance);
        ~Application();

        void OnCommand(int commandID);
        void OnDestroy();
        void OnNotifyIconRightClick();

        int RunMessageLoop();

    private:
        HINSTANCE hInstance;
        HWND hWnd;
        NOTIFYICONDATA notifyIcon;
        HMENU notifyIconMenu;

        void createNotifyIcon();
        void createNotifyIconMenu();
        void createWindow(std::wstring_view className);
        void destroyNotifyIcon();
        void registerWindowClass(std::wstring_view className) const;
        void showNotifyIconMenu();
        static LRESULT CALLBACK wndProc(
            HWND hWnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        );
    };
}
