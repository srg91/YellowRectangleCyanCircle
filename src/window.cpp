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
        auto prevDisplayRect = context->GetPreviousDisplayRect();
        // Convert previous game rect position from absolute to relative to previous display
        auto prevGameRect = Rect::ClampROI(context->GetPreviousGameRect(), prevDisplayRect);

        auto area = context->GetWorkingArea();
        auto displayRect = context->GetDisplayRect();
        // Convert game rect position from absolute to relative to current display
        auto gameRect = Rect::ClampROI(context->GetGameRect(), displayRect);

        auto prevX = prevDisplayRect.x + prevGameRect.x + prevArea.x;
        auto prevY = prevDisplayRect.y + prevGameRect.y + prevArea.y;

        auto newX = displayRect.x + gameRect.x + area.x;
        auto newY = displayRect.y + gameRect.y + area.y;

        int xDiff = std::abs(prevX - newX);
        int yDiff = std::abs(prevY - newY);

        int wDiff = std::abs(prevArea.width - area.width);
        int hDiff = std::abs(prevArea.height - area.height);

        bool shouldMoveWindow = xDiff > 5 || yDiff > 5 || wDiff > 5 || hDiff > 5;

        L(trace, "[WindowUpdater::Perform] working area: [{}] => [{}]", prevArea, area);
        L(trace, "[WindowUpdater::Perform] display rect: [{}] => [{}]", prevDisplayRect, displayRect);
        L(trace, "[WindowUpdater::Perform] game rect: [{}] => [{}]", prevGameRect, gameRect);

        if (shouldMoveWindow) {
            L(debug, "[WindowUpdater::Perform] move main window");
            ::MoveWindow(hWnd, newX, newY, area.width, area.height, false);
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
