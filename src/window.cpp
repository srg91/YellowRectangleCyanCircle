#include "window.hpp"

namespace YellowRectangleCyanCircle {
    void WindowUpdater::Perform(std::shared_ptr<IContext> context) {
        auto hWnd = context->GetWindowHandle();
        if (!hWnd) return;

        bool isKeypadEnabled = context->IsDetectorEnabled(DetectorType::Keypad);
        bool isFingerprintEnabled = context->IsDetectorEnabled(DetectorType::Fingerprint);
        if (!(isKeypadEnabled || isFingerprintEnabled)) return;

        auto prevArea = context->GetPreviousWorkingArea();
        auto prevGameRect = context->GetPreviousGameRect();

        auto area = context->GetWorkingArea();
        auto gameRect = context->GetGameRect();

        int xDiff = std::abs(
            (prevGameRect.x + prevArea.x) - (gameRect.x  + area.x)
        );
        int yDiff = std::abs(
            (prevGameRect.y + prevArea.y) - (gameRect.y + area.y)
        );

        int wDiff = std::abs(prevArea.width - area.width);
        int hDiff = std::abs(prevArea.height - area.height);

        bool shouldMoveWindow = xDiff > 5 || yDiff > 5 || wDiff > 5 || hDiff > 5;

        if (shouldMoveWindow)
            ::MoveWindow(hWnd, gameRect.x + area.x, gameRect.y + area.y, area.width, area.height, false);

        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            if (context->IsShapesChanged(dt)) {
                ::InvalidateRect(hWnd, nullptr, false);
                break;
            }
        }
    }
}
