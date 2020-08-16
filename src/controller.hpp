#pragma once

#include "context.hpp"
#include "game.hpp"
#include "hook.hpp"
#include "screen.hpp"

#include <chrono>
#include <thread>
#include <vector>

namespace YellowRectangleCyanCircle {
    class Controller {
    public:
        Controller(HWND hWnd, std::wstring_view gameWindowName);
        ~Controller();

        bool IsDetectorEnabled(DetectorType dt);
        void EnableDetector(DetectorType dt, bool value);

        void DrawShapes(DetectorType dt, CComPtr<ID2D1HwndRenderTarget> target, CComPtr<ID2D1SolidColorBrush> brush);
    private:
        HWND hWnd;

        std::shared_ptr<Context> context;
        std::chrono::milliseconds timerInterval;
        std::thread timer;

        void initializeContext();
        bool isAnyDetectorEnabled();
        void onTimer();
        void updateTimer();

        std::shared_ptr<Game> game;
        std::shared_ptr<Screen> screen;
        std::vector<std::shared_ptr<IAction>> actions;
        void initializeActions(HWND gameHWnd);

        std::wstring gameWindowName;
        std::shared_ptr<Hook> hookCreateWindow;
        std::shared_ptr<Hook> hookDestroyWindow;
        std::shared_ptr<Hook> hookMoveWindow;

        void initializeHooks(HWND gameHWnd);
        void clearHooks();
        void onCreateWindow(HWND hWnd, LONG idObject);
        void onDestroyWindow(HWND hWnd, LONG idObject);
        void onMoveWindow(HWND hWnd, LONG idObject);
    };
}
