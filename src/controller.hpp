#pragma once

#include "types.hpp"

#include <chrono>
#include <thread>
#include <unordered_map>
#include <vector>

namespace YellowRectangleCyanCircle {
    class Controller {
    public:
        Controller();
        ~Controller();

        bool IsDetectorEnabled(DetectorType dt);
        void EnableDetector(DetectorType dt, bool value);
    private:
        // TODO: rework
        std::unordered_map<DetectorType, bool> m;

        std::chrono::milliseconds timerInterval;
        std::thread timer;

        bool isAnyDetectorEnabled();
        void onTimer();
        void updateTimer();
    };
}
