#include "controller.hpp"

namespace YellowRectangleCyanCircle {
    Controller::Controller() :
        context(std::make_shared<Context>()),
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
        if (this->context) this->context = nullptr;
    }

    bool Controller::IsDetectorEnabled(DetectorType dt) {
        if (!this->context) return false;
        auto lock = this->context->LockOnRead();
        return this->context->IsDetectorEnabled(dt);
    }

    void Controller::EnableDetector(DetectorType dt, bool value) {
        if (!this->context) return;

        {
            auto lock = this->context->LockOnWrite();
            this->context->SetDetectorEnabled(dt, value);
        }
        this->updateTimer();
    }

    bool Controller::isAnyDetectorEnabled() {
        // Area is not real detector
        return this->IsDetectorEnabled(DetectorType::Fingerprint) || this->IsDetectorEnabled(DetectorType::Keypad);
    }

    void Controller::onTimer() {
        while (this->isAnyDetectorEnabled()) {
            auto timerStart = std::chrono::high_resolution_clock::now();
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(900ms);
            OutputDebugString(L"OnTimer tick\n");
            auto timerEnd = std::chrono::high_resolution_clock::now();

            auto duration = timerEnd - timerStart;
            if (duration > this->timerInterval) continue;
            std::this_thread::sleep_for(this->timerInterval - duration);
        }
    }

    void Controller::updateTimer() {
        bool shouldEnable = this->isAnyDetectorEnabled();
        bool timerEnabled = this->timer.joinable();

        if (!shouldEnable && timerEnabled) {
            this->timer.join();
            // Clear context (thread-safe, cause we already stopped thread)
            this->context = std::make_shared<Context>();
        }
        else if (shouldEnable && !timerEnabled) {
            this->timer = std::thread([this]() { this->onTimer(); });
        }
    };
}
