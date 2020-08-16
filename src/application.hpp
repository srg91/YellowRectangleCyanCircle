#pragma once

#include "controller.hpp"
#include "exception.hpp"
#include "resource.hpp"
#include "types.hpp"
#include "winapi.hpp"

#include <memory>
#include <string>

namespace YellowRectangleCyanCircle {
    class Application {
    public:
        Application(HINSTANCE hInstance);
        ~Application();

        void OnCommand(int commandID);
        void OnDestroy();
        void OnDisplayChange();
        void OnNotifyIconRightClick();
        void OnResize(UINT width, UINT height);
        void OnPaint();

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
        void notifyIconMenuCheck(int itemID, bool value);
        void notifyIconMenuShow();
        void registerWindowClass(std::wstring_view className) const;
        static LRESULT CALLBACK wndProc(
            HWND hWnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
        );

        CComPtr<ID2D1Factory> factory;
        CComPtr<ID2D1HwndRenderTarget> renderTarget;
        CComPtr<ID2D1SolidColorBrush> brushCyan;
        CComPtr<ID2D1SolidColorBrush> brushYellow;

        void clearResources();
        void createFactory();
        HRESULT initializeResources();

        std::shared_ptr<Controller> controller;
    };
}
