#pragma once

#include "interface.hpp"
#include "types.hpp"
#include "winapi.hpp"

#include <shared_mutex>
#include <string>

namespace YellowRectangleCyanCircle {
    class Game : public IAction, public IWindowCallbackReceiver {
    public:
        bool IsFound() const noexcept;
        Rect::Rect GetRect() const noexcept;

        Game(const std::wstring_view windowTitle, HWND hWnd = 0);

        void OnWindowCreated(HWND hWnd) override final;
        void OnWindowDestroyed(HWND hWnd) override final;
        void OnWindowMoved(HWND hWnd) override final;

        void Perform(std::shared_ptr<IContext> context) override final;
    private:
        HWND hWnd;
        Rect::Rect rect;
        mutable std::shared_mutex mutex;

        std::wstring windowTitle;

        void clear();
    };
}
