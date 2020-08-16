#pragma once

#include "context.hpp"

#include <chrono>
#include <thread>

namespace YellowRectangleCyanCircle {
    class Controller {
    public:
        Controller(HWND hWnd);
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
    };
}
