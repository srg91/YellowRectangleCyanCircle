#include "controller.hpp"

namespace YellowRectangleCyanCircle {
    Controller::Controller() :
        timerInterval(1000)
    {
        this->EnableDetector(DetectorType::Area, true);
        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            this->EnableDetector(dt, false);
        }
    }

    Controller::~Controller() {
        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            this->EnableDetector(dt, false);
        }
    }

    bool Controller::IsDetectorEnabled(DetectorType dt) {
        return this->m[dt];
    }

    void Controller::EnableDetector(DetectorType dt, bool value) {
        this->m[dt] = value;
        this->updateTimer();
    }

    bool Controller::isAnyDetectorEnabled() {
        // Area is not real detector
        return this->IsDetectorEnabled(DetectorType::Fingerprint) || this->IsDetectorEnabled(DetectorType::Keypad);
    }

    void Controller::onTimer() {
        while (this->isAnyDetectorEnabled()) {
            auto start = std::chrono::high_resolution_clock::now();
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(900ms);
            OutputDebugString(L"OnTimer tick\n");
            auto end = std::chrono::high_resolution_clock::now();

            if (end - start > this->timerInterval) continue;
            auto d = this->timerInterval - (end - start);
            std::wstringstream s;
            s << "Waiting for " << std::chrono::duration_cast<std::chrono::milliseconds>(d).count() << " ms" << std::endl;
            OutputDebugString(std::data(s.str()));
            std::this_thread::sleep_for(this->timerInterval - (end - start));
        }
    }

    void Controller::updateTimer() {
        bool shouldEnable = this->isAnyDetectorEnabled();
        bool timerEnabled = this->timer.joinable();

        if (!shouldEnable && timerEnabled) {
            this->timer.join();
            // TODO: Clear
            // TODO: invalidate rect in application
        }
        else if (shouldEnable && !timerEnabled) {
            this->timer = std::thread([this]() { this->onTimer(); });
        }
    };
}
