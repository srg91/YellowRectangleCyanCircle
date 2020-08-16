#pragma once

#include "controller.hpp"
#include "exception.hpp"
#include "resource.hpp"
#include "types.hpp"

#include <memory>
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

        std::shared_ptr<Controller> controller;

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
