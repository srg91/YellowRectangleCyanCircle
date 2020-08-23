#include "window.hpp"

namespace YellowRectangleCyanCircle {
    void WindowUpdater::Perform(std::shared_ptr<IContext> context) {
        L(trace, "[WindowUpdater::Perform] called");

        if (!context) {
            L(debug, "[WindowUpdater::Perform] failed, no context");
            return;
        }

        auto hWnd = context->GetWindowHandle();
        if (!hWnd) {
            L(debug, "[WindowUpdater::Perform] failed, no main window");
            return;
        }

        bool isKeypadEnabled = context->IsDetectorEnabled(DetectorType::Keypad);
        bool isFingerprintEnabled = context->IsDetectorEnabled(DetectorType::Fingerprint);
        if (!(isKeypadEnabled || isFingerprintEnabled)) {
            L(trace, "[WindowUpdater::Perform] skip, no one detector enabled");
            return;
        }

        auto prevArea = context->GetPreviousWorkingArea();
        auto prevDesktop = context->GetPreviousDesktopInfo();
        // Convert previous game rect position from absolute to relative to previous display
        auto prevGameRect = Rect::ClampROI(context->GetPreviousGameRect(), prevDesktop.Rect);

        auto area = context->GetWorkingArea();
        auto desktop = context->GetDesktopInfo();
        // Convert game rect position from absolute to relative to current display
        auto gameRect = Rect::ClampROI(context->GetGameRect(), desktop.Rect);

        auto prevX = prevDesktop.Rect.x + prevGameRect.x + prevArea.x;
        auto prevY = prevDesktop.Rect.y + prevGameRect.y + prevArea.y;

        auto newX = desktop.Rect.x + gameRect.x + area.x;
        auto newY = desktop.Rect.y + gameRect.y + area.y;

        int xDiff = std::abs(prevX - newX);
        int yDiff = std::abs(prevY - newY);

        int wDiff = std::abs(prevArea.width - area.width);
        int hDiff = std::abs(prevArea.height - area.height);

        bool shouldMoveWindow = xDiff > 5 || yDiff > 5 || wDiff > 5 || hDiff > 5;

        L(trace, "[WindowUpdater::Perform] working area: [{}] => [{}]", prevArea, area);
        L(trace, "[WindowUpdater::Perform] display rect: [{}] => [{}]", prevDesktop.Rect, desktop.Rect);
        L(trace, "[WindowUpdater::Perform] game rect: [{}] => [{}]", prevGameRect, gameRect);

        if (shouldMoveWindow) {
            float sx = desktop.Scale.x && desktop.Scale.x != 100 ? (desktop.Scale.x / 100.0f) : 1;
            float sy = desktop.Scale.y && desktop.Scale.y != 100 ? (desktop.Scale.y / 100.0f) : 1;

            L(trace, "[WindowUpdater::Perform] position: [{}, {}] => [{}, {}]", prevX, prevY, newX, newY);
            L(trace, "[WindowUpdater::Perform] use scale: {}x{}", sx, sy);

            int x = static_cast<int>(newX / sx);
            int y = static_cast<int>(newY / sy);
            int w = static_cast<int>(area.width / sx);
            int h = static_cast<int>(area.height / sy);

            L(debug, "[WindowUpdater::Perform] move main window to Rect({}, {}, {}, {})", x, y, w, h);

            ::MoveWindow(hWnd, x, y, w, h, false);
        }

        bool shouldRedraw = shouldMoveWindow;
        if (!shouldRedraw) {
            for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
                if (context->IsDetectorEnabled(dt) && context->IsShapesChanged(dt)) {
                    L(trace, "[WindowUpdater::Perform] detector [{}] has shapes changed", dt);
                    shouldRedraw = true;
                    break;
                }
            }
        }

        if (shouldRedraw) {
            L(debug, "[WindowUpdater::Perform] redraw main window");
            ::InvalidateRect(hWnd, nullptr, false);
        }
    }
}
