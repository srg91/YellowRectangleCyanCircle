#include "controller.hpp"

namespace YellowRectangleCyanCircle {
    Controller::Controller(HWND hWnd, std::wstring_view gameWindowName) :
        hWnd(hWnd),
        timerInterval(333),
        gameWindowName(gameWindowName)
    {
        L(trace, "[Controller::Controller] called");
        this->initializeContext();

        auto gameHWnd = this->prefindGame();
        this->initializeActions(gameHWnd);
        this->initializeHooks(gameHWnd);
    }

    Controller::~Controller() {
        L(trace, "[Controller::~Controller] called");

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
        L(trace, "[Controller::DrawShapes] called with detector: {}", dt);

        if (!this->IsDetectorEnabled(dt)) {
            L(trace, "[Controller::DrawShapes] detector disabled, do nothing");
            return;
        }

        if (!target) {
            L(debug, "[Controller::DrawShapes] failed, no target");
            return;
        }

        if (!brush) {
            L(debug, "[Controller::DrawShapes] failed, no brush");
            return;
        }

        auto context = this->context;
        if (!context) {
            L(debug, "[Controller::DrawShapes] failed, no context");
            return;
        }

        auto shapes = context->GetShapes(dt);
        L(trace, "[Controller::DrawShapes] shapes count: {}", std::size(shapes));

        for (const auto& shape : shapes) {
            shape->OnDraw(target, brush);
        }
    }

    void Controller::initializeContext() {
        L(trace, "[Controller::initializeContext] called");

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

    void Controller::onException() {
        // TODO: bad way, rework
        this->clearActions();

        auto gameHWnd = this->prefindGame();
        this->initializeActions(gameHWnd);
    }

    void Controller::onTimer() {
        L(trace, "[Controller::onTimer] called");

        while (this->isAnyDetectorEnabled()) {
            auto timerStart = std::chrono::high_resolution_clock::now();

            try {
                context = this->context;
                if (context) {
                    context->ClearOnTick();

                    for (auto& action : this->actions) {
                        if (context) action->Perform(context);
                    }
                }
                else {
                    L(debug, "[Controller::onTimer] failed, no context");
                }
            }
            catch (const std::exception& exc) {
                L(error, "[Controller::onTimer] caught exception: {}", exc.what());
                this->onException();
            }
            catch (...) {
                L(error, "[Controller::onTimer] caught unexpected exception");
                this->onException();
            }

            auto timerEnd = std::chrono::high_resolution_clock::now();

            auto duration = timerEnd - timerStart;
            if (duration > this->timerInterval) continue;

            L(trace, "[Controller::onTimer] sleep for {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(this->timerInterval - duration).count());
            std::this_thread::sleep_for(this->timerInterval - duration);
        }
    }

    void Controller::updateTimer() {
        L(trace, "[Controller::updateTimer] called");

        bool shouldEnable = this->isAnyDetectorEnabled();
        bool timerEnabled = this->timer.joinable();

        if (!shouldEnable && timerEnabled) {
            L(debug, "[Controller::updateTimer] no one detector enabled, but timer is, stopping");

            this->timer.join();
            // Clear context (thread-safe, cause we already stopped thread)
            this->context = nullptr;
            this->initializeContext();
        }
        else if (shouldEnable && !timerEnabled) {
            L(debug, "[Controller::updateTimer] start timer");
            this->timer = std::thread([this]() { this->onTimer(); });
        }
    };

    void Controller::initializeActions(HWND gameHWnd) {
        L(trace, "[Controller::initializeActions] called with game handle: {}", reinterpret_cast<std::size_t>(gameHWnd));

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
        L(debug, "[Controller::initializeActions] success");
    }

    void Controller::clearActions() {
        L(trace, "[Controller::clearActions] called");
        this->game = nullptr;
        this->screen = nullptr;
        this->actions.clear();
        L(debug, "[Controller::clearActions] success");
    }

    void Controller::initializeHooks(HWND gameHWnd) {
        L(trace, "[Controller::initializeHooks] called with game handle: {}", reinterpret_cast<std::size_t>(gameHWnd));

        this->hookCreateWindow = std::make_shared<Hook>(EVENT_OBJECT_CREATE);
        this->hookCreateWindow->SetCallback([this](HWND hWnd, LONG idObject) { this->onCreateWindow(hWnd, idObject); });

        this->hookDestroyWindow = std::make_shared<Hook>(EVENT_OBJECT_DESTROY, gameHWnd, true, false);
        this->hookDestroyWindow->SetCallback([this](HWND hWnd, LONG idObject) { this->onDestroyWindow(hWnd, idObject); });

        this->hookMoveWindow = std::make_shared<Hook>(EVENT_OBJECT_LOCATIONCHANGE, gameHWnd, true, true);
        this->hookMoveWindow->SetCallback([this](HWND hWnd, LONG idObject) { this->onMoveWindow(hWnd, idObject); });

        if (gameHWnd) {
            L(debug, "[Controller::initializeHooks] there is game window, enable destroy and move hooks");

            this->hookCreateWindow->Disable();
            this->hookDestroyWindow->Enable();
            this->hookMoveWindow->Enable();
        }
        else {
            L(debug, "[Controller::initializeHooks] there is no game window, enable create window hook");

            this->hookDestroyWindow->Disable();
            this->hookMoveWindow->Disable();
            this->hookCreateWindow->Enable();
        }
    }

    void Controller::clearHooks() {
        L(trace, "[Controller::clearHooks] called");

        this->hookCreateWindow = nullptr;
        this->hookDestroyWindow = nullptr;
        this->hookMoveWindow = nullptr;

        L(trace, "[Controller::clearHooks] success");
    }

    void Controller::onCreateWindow(HWND hWnd, LONG idObject) {
        if (!this->game) return;

        this->game->OnWindowCreated(hWnd);
        if (this->game->IsFound()) {
            L(trace, "[Controller::onCreateWindow] game found");
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
            L(trace, "[Controller::onDestroyWindow] game window destroyed");
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

            if (this->screen && prevRect != newRect) {
                L(trace, "[Controller::onMoveWindow] game window moved, game rect: {}", newRect);
                this->screen->OnWindowMoved(hWnd);
            }
        }
    }

    HWND Controller::prefindGame() {
        L(trace, L"[Controller::prefindGame] called with game window name: {}", this->gameWindowName);

        HWND gameHWnd = 0;
        if (!std::empty(this->gameWindowName)) {
            gameHWnd = ::FindWindow(nullptr, std::data(this->gameWindowName));
            if (gameHWnd)
                L(debug, "[Controller::prefindGame] game found: {}", reinterpret_cast<std::size_t>(gameHWnd));
            else
                L(debug, "[Controller::prefindGame] unable prefind game: game window not found");
        }
        else {
            L(debug, "[Controller::prefindGame] unable prefind game: empty game window name");
        }
        return gameHWnd;
    }
}
