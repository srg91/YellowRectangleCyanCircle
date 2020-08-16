#include "window.hpp"

namespace YellowRectangleCyanCircle {
    void WindowUpdater::Perform(std::shared_ptr<IContext> context) {
        auto hWnd = context->GetWindowHandle();
        if (!hWnd) return;

        bool isKeypadEnabled = context->IsDetectorEnabled(DetectorType::Keypad);
        bool isFingerprintEnabled = context->IsDetectorEnabled(DetectorType::Fingerprint);
        if (!(isKeypadEnabled || isFingerprintEnabled)) return;

        const auto& prevArea = context->GetPreviousWorkingArea();
        const auto& area = context->GetWorkingArea();

        int xDiff = std::abs(prevArea.x - area.x);
        int yDiff = std::abs(prevArea.y - area.y);

        int wDiff = std::abs(prevArea.width - area.width);
        int hDiff = std::abs(prevArea.height - area.height);

        bool shouldMoveWindow = xDiff > 5 || yDiff > 5 || wDiff > 5 || hDiff > 5;

        if (shouldMoveWindow)
            ::MoveWindow(hWnd, area.x, area.y, area.width, area.height, false);

        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            if (context->IsShapesChanged(dt)) {
                ::InvalidateRect(hWnd, nullptr, false);
                break;
            }
        }
    }
}
