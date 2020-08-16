#include "controller.hpp"

namespace YellowRectangleCyanCircle {
    Controller::Controller(HWND hWnd, std::wstring_view gameWindowName) :
        hWnd(hWnd),
        timerInterval(333),
        gameWindowName(gameWindowName)
    {
        this->initializeContext();

        auto gameHWnd = this->prefindGame();
        this->initializeActions(gameHWnd);
        this->initializeHooks(gameHWnd);
    }

    Controller::~Controller() {
        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            this->EnableDetector(dt, false);
        }
        this->context = nullptr;
        this->clearHooks();
        this->clearActions();
    }

    bool Controller::IsDetectorEnabled(DetectorType dt) {
        if (!this->context) return false;
        return this->context->IsDetectorEnabled(dt);
    }

    void Controller::EnableDetector(DetectorType dt, bool value) {
        if (!this->context) return;

        this->context->SetDetectorEnabled(dt, value);
        this->updateTimer();
    }

    void Controller::DrawShapes(DetectorType dt, CComPtr<ID2D1HwndRenderTarget> target, CComPtr<ID2D1SolidColorBrush> brush) {
        if (!this->IsDetectorEnabled(dt)) return;
        if (!this->context) return;

        auto shapes = context->GetShapes(dt);
        for (const auto& shape : shapes) {
            shape->OnDraw(target, brush);
        }
    }

    void Controller::initializeContext() {
        this->context = std::make_shared<Context>();
        this->context->SetWindowHandle(this->hWnd);
        this->EnableDetector(DetectorType::Area, true);
        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            this->EnableDetector(dt, false);
        }
    }

    bool Controller::isAnyDetectorEnabled() {
        // Area is not real detector
        return this->IsDetectorEnabled(DetectorType::Fingerprint) || this->IsDetectorEnabled(DetectorType::Keypad);
    }

    void Controller::onTimer() {
        while (this->isAnyDetectorEnabled()) {
            auto timerStart = std::chrono::high_resolution_clock::now();

            try {
                this->context->ClearOnTick();

                for (auto& action : this->actions) {
                    action->Perform(this->context);
                }
            }
            catch (...) {
                // TODO: bad way, rework
                this->clearActions();

                auto gameHWnd = this->prefindGame();
                this->initializeActions(gameHWnd);
            }

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

    void Controller::initializeActions(HWND gameHWnd) {
        this->game = std::make_shared<Game>(this->gameWindowName, gameHWnd);
        this->screen = std::make_shared<Screen>(std::make_shared<Direct>(), gameHWnd);

        this->actions = {
            this->game,
            this->screen,
            std::make_shared<AreaDetector>(),
            std::make_shared<FingerprintDetector>(),
            std::make_shared<KeypadDetector>(),
            std::make_shared<WindowUpdater>(),
        };
    }

    void Controller::clearActions() {
        this->actions.clear();
        this->game = nullptr;
        this->screen = nullptr;
    }

    void Controller::initializeHooks(HWND gameHWnd) {
        this->hookCreateWindow = std::make_shared<Hook>(EVENT_OBJECT_CREATE);
        this->hookCreateWindow->SetCallback([this](HWND hWnd, LONG idObject) { this->onCreateWindow(hWnd, idObject); });

        this->hookDestroyWindow = std::make_shared<Hook>(EVENT_OBJECT_DESTROY, gameHWnd, true, false);
        this->hookDestroyWindow->SetCallback([this](HWND hWnd, LONG idObject) { this->onDestroyWindow(hWnd, idObject); });

        this->hookMoveWindow = std::make_shared<Hook>(EVENT_OBJECT_LOCATIONCHANGE, gameHWnd, true, true);
        this->hookMoveWindow->SetCallback([this](HWND hWnd, LONG idObject) { this->onMoveWindow(hWnd, idObject); });

        if (gameHWnd) {
            this->hookCreateWindow->Disable();
            this->hookDestroyWindow->Enable();
            this->hookMoveWindow->Enable();
        }
        else {
            this->hookDestroyWindow->Disable();
            this->hookMoveWindow->Disable();
            this->hookCreateWindow->Enable();
        }
    }

    void Controller::clearHooks() {
        this->hookCreateWindow = nullptr;
        this->hookDestroyWindow = nullptr;
        this->hookMoveWindow = nullptr;
    }

    void Controller::onCreateWindow(HWND hWnd, LONG idObject) {
        if (!this->game) return;

        this->game->OnWindowCreated(hWnd);
        if (this->game->IsFound()) {
            this->game->OnWindowMoved(hWnd);

            this->hookDestroyWindow->SetHandle(hWnd);
            this->hookMoveWindow->SetHandle(hWnd);

            this->hookCreateWindow->Disable();
            this->hookDestroyWindow->Enable();
            this->hookMoveWindow->Enable();
        }
    }

    void Controller::onDestroyWindow(HWND hWnd, LONG idObject) {
        if (this->game && this->game->IsFound()) {
            this->game->OnWindowDestroyed(hWnd);
        }

        this->hookDestroyWindow->Disable();
        this->hookMoveWindow->Disable();
        this->hookCreateWindow->Enable();
    }

    void Controller::onMoveWindow(HWND hWnd, LONG idObject) {
        // Skip unnecessary events
        if (idObject == OBJID_CURSOR) return;

        if (this->game && this->game->IsFound()) {
            auto prevRect = this->game->GetRect();
            this->game->OnWindowMoved(hWnd);
            auto newRect = this->game->GetRect();

            if (this->screen && prevRect != newRect) this->screen->OnWindowMoved(hWnd);
        }
    }

    HWND Controller::prefindGame() {
        HWND gameHWnd = 0;
        if (std::empty(this->gameWindowName)) return gameHWnd;
        return ::FindWindow(nullptr, std::data(this->gameWindowName));
    }
}
