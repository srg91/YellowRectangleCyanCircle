#include "screen.hpp"

namespace YellowRectangleCyanCircle {
    Screen::Screen(
        std::shared_ptr<IDirect> direct,
        HWND hWnd
    ) : direct(direct) {
        L(trace, "[Screen::Screen] called with handle: {}", reinterpret_cast<std::size_t>(hWnd));

        this->currentDisplay = WinAPI::GetWindowDisplayInfo(hWnd);
        L(trace, L"[Screen::Screen] current display: [{}] [{}]", this->currentDisplay.name, this->currentDisplay.area);

        this->desktop = std::make_unique<Desktop>(
            this->direct,
            this->currentDisplay.name);
    }

    void Screen::OnWindowMoved(HWND hWnd) {
        L(trace, "[Screen::OnWindowMoved] called with handle: {}", reinterpret_cast<std::size_t>(hWnd));

        auto display = WinAPI::GetWindowDisplayInfo(hWnd);

        {
            std::shared_lock lock(this->desktopMutex);

            auto isNameSame = display.name == this->currentDisplay.name;
            auto isSizeSame = (display.area.width == display.area.width) &&
                (display.area.height == this->currentDisplay.area.height);

            if (isNameSame && isSizeSame) {
                L(trace, "[Screen::OnWindowMoved] display is not changed");
                return;
            }
        }

        std::unique_lock lock(this->desktopMutex);

        L(debug, L"[Screen::OnWindowMoved] Window moved to new display: [{}] [{}]", display.name, display.area);
        this->currentDisplay = display;
        this->desktop->SwitchDisplay(display.name);
    }

    void Screen::Perform(std::shared_ptr<IContext> context) {
        L(trace, "[Screen::Perform] called");

        if (!context) {
            L(debug, "[Screen::Perform] failed, there is no context");
            return;
        }

        std::shared_lock read(this->desktopMutex);

        auto desktopInfo = this->desktop->GetInfo();
        context->SetDesktopInfo(desktopInfo);

        L(trace, "[Screen::Perform] current duplication desktop: {}", desktopInfo.Rect);

        Mat mat(desktopInfo.Rect.height, desktopInfo.Rect.width, CV_8UC4);
        if (!this->desktop->Duplicate(std::data(mat))) {
            L(debug, "[Screen::Perform] unable to perform desktop duplication");
            return;
        }

        if (context->IsGameFound())
        {
            auto gameRect = context->GetGameRect();
            L(trace, "[Screen::Perform] current game rect: {}", gameRect);

            // Clamp current game window to display
            auto roi = Rect::ClampROI(gameRect, desktopInfo.Rect);
            L(trace, "[Screen::Perform] roi from game and display intersection: {}", roi);

            if (!roi.empty()) mat = mat(roi);
        }

        cv::cvtColor(mat, mat, cv::COLOR_BGRA2GRAY);
        context->SetScreenImage(mat);
        L(debug, "[Screen::Perform] set screen image with size: {}x{}", mat.cols, mat.rows);
    }
}
