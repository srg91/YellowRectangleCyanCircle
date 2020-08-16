#pragma once

#include "desktop.hpp"
#include "interface.hpp"
#include "types.hpp"
#include "winapi.hpp"

#include <shared_mutex>

namespace YellowRectangleCyanCircle {
    class Screen : public IAction, public IWindowCallbackReceiver {
    public:
        Screen(
            std::shared_ptr<IDirect> direct,
            HWND hWnd = 0
        );

        void OnWindowMoved(HWND hWnd) override final;
        void Perform(std::shared_ptr<IContext> context);
    private:
        std::shared_ptr<IDirect> direct;

        std::unique_ptr<Desktop> desktop;
        mutable std::shared_mutex desktopMutex;

        WinAPI::DisplayInfo currentDisplay;
    };
}
