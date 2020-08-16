#include "controller.hpp"

namespace YellowRectangleCyanCircle {
    Controller::Controller(HWND hWnd) :
        hWnd(hWnd),
        timerInterval(1000)
    {
        this->initializeContext();

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

    void Controller::DrawShapes(DetectorType dt, CComPtr<ID2D1HwndRenderTarget> target, CComPtr<ID2D1SolidColorBrush> brush) {
        if (!this->IsDetectorEnabled(dt)) return;
        if (!this->context) return;

        auto lock = this->context->LockOnRead();
        for (const auto& shape : context->GetShapes(dt)) {
            shape->OnDraw(target, brush);
        }
    }

    void Controller::initializeContext() {
        this->context = std::make_shared<Context>();
        this->context->SetWindowHandle(this->hWnd);
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
            this->context = nullptr;
            this->initializeContext();
        }
        else if (shouldEnable && !timerEnabled) {
            this->timer = std::thread([this]() { this->onTimer(); });
        }
    };
}
