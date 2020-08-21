#include "context.hpp"

namespace YellowRectangleCyanCircle {
    Context::Context() :
        currentDetector(DetectorType::None),
        windowHandle(0)
    {
        this->KeypadClearEmptyRunCounter();
        this->KeypadClearShapesCache();

        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            this->detectorStates[dt] = false;
            this->detectorShapes[dt] = {};
            this->shapesChanged[dt] = false;
        }
    }

    void Context::ClearOnTick() {
        L(trace, "[Controller::onTimer] called");

        auto lock = this->lockOnWrite();
        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            this->shapesChanged[dt] = false;
        }
        this->workingArea = Rect::Rect();
        this->currentDetector = DetectorType::None;
    };

    bool Context::IsGameFound() const {
        auto lock = this->lockOnRead();
        return this->isGameFound;
    }

    void Context::SetGameFound(bool value) {
        auto lock = this->lockOnWrite();
        this->isGameFound = value;
    }

    const Rect::Rect& Context::GetGameRect() const {
        auto lock = this->lockOnRead();
        return this->gameRect;
    }

    void Context::SetGameRect(const Rect::Rect& rect) {
        auto lock = this->lockOnWrite();
        this->prevGameRect = this->gameRect;
        this->gameRect = rect;
    }

    const Rect::Rect& Context::GetPreviousGameRect() const {
        auto lock = this->lockOnRead();
        return this->prevGameRect;
    }

    const Rect::Rect& Context::GetDisplayRect() const {
        auto lock = this->lockOnRead();
        return this->displayRect;
    };

    void Context::SetDisplayRect(const Rect::Rect& rect) {
        auto lock = this->lockOnWrite();
        this->prevDisplayRect = this->displayRect;
        this->displayRect = rect;
    };

    const Rect::Rect& Context::GetPreviousDisplayRect() const {
        auto lock = this->lockOnRead();
        return this->prevDisplayRect;
    };

    bool Context::IsDetectorEnabled(DetectorType dt) const {
        auto lock = this->lockOnRead();
        if (this->detectorStates.find(dt) != this->detectorStates.end())
            return this->detectorStates.at(dt);
        else
            return false;
    }

    void Context::SetDetectorEnabled(DetectorType dt, bool value) {
        auto lock = this->lockOnWrite();
        this->detectorStates[dt] = value;
    }

    DetectorType Context::GetCurrentDetector() const {
        auto lock = this->lockOnRead();
        return this->currentDetector;
    };

    void Context::SetCurrentDetector(DetectorType dt) {
        auto lock = this->lockOnWrite();
        this->currentDetector = dt;
    }

    unsigned int Context::KeypadGetEmptyRunCounter() const {
        auto lock = this->lockOnRead();
        return this->keypadEmptyRunCounter;
    }

    void Context::KeypadRegisterEmptyRun() {
        auto lock = this->lockOnWrite();
        this->keypadEmptyRunCounter++;
    }

    void Context::KeypadClearEmptyRunCounter() {
        auto lock = this->lockOnWrite();
        this->keypadEmptyRunCounter = 0;
    }

    const std::vector<std::shared_ptr<IShape>>& Context::KeypadGetShapesCache() const {
        auto lock = this->lockOnRead();
        return this->keypadShapesCache;
    }

    void Context::KeypadSetShapesCache(const std::vector<std::shared_ptr<IShape>>& value) {
        auto lock = this->lockOnWrite();
        this->keypadShapesCache = value;
    }

    void Context::KeypadClearShapesCache() {
        auto lock = this->lockOnWrite();
        this->keypadShapesCache.clear();
    }

    const Rect::Rect& Context::GetWorkingArea() const {
        auto lock = this->lockOnRead();
        return this->workingArea;
    }

    const Rect::Rect& Context::GetPreviousWorkingArea() const {
        auto lock = this->lockOnRead();
        return this->prevWorkingArea;
    }

    void Context::SetWorkingArea(const Rect::Rect& rect) {
        auto lock = this->lockOnWrite();
        this->prevWorkingArea = this->workingArea;
        this->workingArea = rect;
    }

    const Mat& Context::GetScreenImage() const {
        auto lock = this->lockOnRead();
        return this->screenImage;
    }

    void Context::SetScreenImage(const Mat& value) {
        auto lock = this->lockOnWrite();
        this->screenImage = value;
    }

    std::vector<std::shared_ptr<IShape>> Context::GetShapes(DetectorType dt) const {
        auto lock = this->lockOnRead();
        if (this->detectorShapes.find(dt) != detectorShapes.end())
            return this->detectorShapes.at(dt);
        else
            return std::vector<std::shared_ptr<IShape>>();
    };

    void Context::SetShapes(DetectorType dt, const std::vector<std::shared_ptr<IShape>>& shapes) {
        std::vector <std::shared_ptr<IShape>> prevShapes;

        {
            auto lock = this->lockOnWrite();
            prevShapes = this->detectorShapes[dt];
            this->detectorShapes[dt] = shapes;
        }
        if (prevShapes != shapes) this->SetShapesChanged(dt, true);
    }

    void Context::ClearShapes(DetectorType dt) {
        bool isShapesChanged = false;
        {
            auto lock = this->lockOnWrite();
            if (this->detectorShapes.find(dt) == detectorShapes.end()) return;

            if (std::size(this->detectorShapes.at(dt)) > 0) {
                isShapesChanged = true;
                this->detectorShapes.at(dt).clear();
            }
        }
        if (isShapesChanged) this->SetShapesChanged(dt, true);
    }

    bool Context::IsShapesChanged(DetectorType dt) const {
        auto lock = this->lockOnRead();
        if (this->detectorShapes.find(dt) != detectorShapes.end())
            return this->shapesChanged.at(dt);
        else
            return false;
    };

    void Context::SetShapesChanged(DetectorType dt, bool value) {
        auto lock = this->lockOnWrite();
        this->shapesChanged[dt] = value;
    };

    HWND Context::GetWindowHandle() const {
        auto lock = this->lockOnRead();
        return this->windowHandle;
    };

    void Context::SetWindowHandle(HWND value) {
        auto lock = this->lockOnWrite();
        this->windowHandle = value;
    };

    std::shared_lock<std::shared_mutex> Context::lockOnRead() const {
        return std::shared_lock(this->mutex);
    }

    std::unique_lock<std::shared_mutex> Context::lockOnWrite() const {
        return std::unique_lock(this->mutex);
    }
}
