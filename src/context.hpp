#pragma once

#include "interface.hpp"
#include "logging.hpp"

#include <unordered_map>
#include <vector>

namespace YellowRectangleCyanCircle {
    class Context : public IContext {
    public:
        Context();

        // Clear on-time variables
        void ClearOnTick() override final;

        // Game information
        bool IsGameFound() const override final;
        void SetGameFound(bool value) override final;

        const Rect::Rect& GetGameRect() const override final;
        void SetGameRect(const Rect::Rect& rect) override final;
        const Rect::Rect& GetPreviousGameRect() const override final;

        // Display information
        const Rect::Rect& GetDisplayRect() const override final;
        void SetDisplayRect(const Rect::Rect& rect) override final;
        const Rect::Rect& GetPreviousDisplayRect() const override final;

        // Detector information
        bool IsDetectorEnabled(DetectorType dt) const override final;
        void SetDetectorEnabled(DetectorType dt, bool value) override final;

        DetectorType GetCurrentDetector() const override final;
        void SetCurrentDetector(DetectorType dt) override final;

        const Rect::Rect& GetWorkingArea() const override final;
        void SetWorkingArea(const Rect::Rect& value) override final;
        const Rect::Rect& GetPreviousWorkingArea() const override final;

        // Keypad detector specials
        unsigned int KeypadGetEmptyRunCounter() const override final;
        void KeypadRegisterEmptyRun() override final;
        void KeypadClearEmptyRunCounter() override final;

        const std::vector<std::shared_ptr<IShape>>& KeypadGetShapesCache() const override final;
        void KeypadSetShapesCache(const std::vector<std::shared_ptr<IShape>>& value) override final;
        void KeypadClearShapesCache() override final;

        // Screen infoction
        const Mat& GetScreenImage() const override final;
        void SetScreenImage(const Mat& value) override final;

        // Shape information
        std::vector<std::shared_ptr<IShape>> GetShapes(DetectorType dt) const override final;
        void SetShapes(DetectorType dt, const std::vector<std::shared_ptr<IShape>>& shapes) override final;
        void ClearShapes(DetectorType dt) override final;

        bool IsShapesChanged(DetectorType dt) const override final;
        void SetShapesChanged(DetectorType dt, bool value) override final;

        // Main application window
        HWND GetWindowHandle() const override final;
        void SetWindowHandle(HWND value) override final;
    private:
        mutable std::shared_mutex mutex;

        bool isGameFound = false;
        Rect::Rect gameRect;
        Rect::Rect prevGameRect;

        Rect::Rect displayRect;
        Rect::Rect prevDisplayRect;

        std::unordered_map<DetectorType, bool> detectorStates;

        unsigned int keypadEmptyRunCounter = 0;
        std::vector<std::shared_ptr<IShape>> keypadShapesCache;

        DetectorType currentDetector;
        Rect::Rect workingArea;
        Rect::Rect prevWorkingArea;

        Mat screenImage;

        std::unordered_map<DetectorType, std::vector<std::shared_ptr<IShape>>> detectorShapes;
        std::unordered_map<DetectorType, bool> shapesChanged;

        HWND windowHandle;

        std::shared_lock<std::shared_mutex> lockOnRead() const;
        std::unique_lock<std::shared_mutex> lockOnWrite() const;
    };
}
