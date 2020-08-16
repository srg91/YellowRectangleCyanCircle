#include "context.hpp"

namespace YellowRectangleCyanCircle {
    Context::Context() :
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

    std::shared_lock<std::shared_mutex> Context::LockOnRead() {
        return std::shared_lock(this->mutex);
    }

    std::unique_lock<std::shared_mutex> Context::LockOnWrite() {
        return std::unique_lock(this->mutex);
    }

    void Context::ClearOnTick() {
        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            this->shapesChanged[dt] = false;
        }
    };

    bool Context::IsGameFound() const {
        return this->isGameFound;
    }

    void Context::SetGameFound(bool value) {
        this->isGameFound = value;
    }

    const Rect::Rect& Context::GetGameRect() const {
        return this->gameRect;
    }

    void Context::SetGameRect(const Rect::Rect& rect) {
        this->gameRect = rect;
    }

    bool Context::IsDetectorEnabled(DetectorType dt) const {
        if (this->detectorStates.find(dt) != this->detectorStates.end())
            return this->detectorStates.at(dt);
        else
            return false;
    }

    void Context::SetDetectorEnabled(DetectorType dt, bool value) {
        this->detectorStates[dt] = value;
    }

    unsigned int Context::KeypadGetEmptyRunCounter() const {
        return this->keypadEmptyRunCounter;
    }

    void Context::KeypadRegisterEmptyRun() {
        this->keypadEmptyRunCounter++;
    }

    void Context::KeypadClearEmptyRunCounter() {
        this->keypadEmptyRunCounter = 0;
    }

    const std::vector<std::shared_ptr<IShape>>& Context::KeypadGetShapesCache() const {
        return this->keypadShapesCache;
    }

    void Context::KeypadSetShapesCache(const std::vector<std::shared_ptr<IShape>>& value) {
        this->keypadShapesCache = value;
    }

    void Context::KeypadClearShapesCache() {
        this->keypadShapesCache.clear();
    }

    const Rect::Rect& Context::GetWorkingArea() const {
        return this->workingArea;
    }

    const Rect::Rect& Context::GetPreviousWorkingArea() const {
        return this->prevWorkingArea;
    }

    void Context::SetWorkingArea(const Rect::Rect& rect) {
        this->prevWorkingArea = this->workingArea;
        this->workingArea = rect;
    }

    const Mat& Context::GetScreenImage() const {
        return this->screenImage;
    }

    void Context::SetScreenImage(const Mat& value) {
        this->screenImage = value;
    }

    std::vector<std::shared_ptr<IShape>> Context::GetShapes(DetectorType dt) const {
        if (this->detectorShapes.find(dt) != detectorShapes.end())
            return this->detectorShapes.at(dt);
        else
            return std::vector<std::shared_ptr<IShape>>();
    };

    void Context::SetShapes(DetectorType dt, const std::vector<std::shared_ptr<IShape>>& shapes) {
        this->detectorShapes[dt] = shapes;
        this->SetShapesChanged(dt, true);
    }

    void Context::ClearShapes(DetectorType dt) {
        if (this->detectorShapes.find(dt) == detectorShapes.end()) return;
        this->detectorShapes.at(dt).clear();
        this->SetShapesChanged(dt, true);
    }

    bool Context::IsShapesChanged(DetectorType dt) const {
        if (this->detectorShapes.find(dt) != detectorShapes.end())
            return this->shapesChanged.at(dt);
        else
            return false;
    };

    void Context::SetShapesChanged(DetectorType dt, bool value) {
        this->shapesChanged[dt] = value;
    };

    HWND Context::GetWindowHandle() const {
        return this->windowHandle;
    };

    void Context::SetWindowHandle(HWND value) {
        this->windowHandle = value;
    };
}
