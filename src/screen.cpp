#include "screen.hpp"

namespace YellowRectangleCyanCircle {
    Screen::Screen(
        std::shared_ptr<IDirect> direct,
        HWND hWnd
    ) : direct(direct) {
        this->currentDisplay = WinAPI::GetWindowDisplayName(hWnd);
        this->desktop = std::make_unique<Desktop>(
            this->direct,
            this->currentDisplay);
    }

    void Screen::OnWindowMoved(HWND hWnd) {
        auto display = WinAPI::GetWindowDisplayName(hWnd);
        if (display == this->currentDisplay) return;

        std::unique_lock lock(this->desktopMutex);
        this->currentDisplay = display;
        this->desktop->SwitchDisplay(display);
    }

    void Screen::Perform(std::shared_ptr<IContext> context) {
        if (!context) return;

        std::shared_lock read(this->desktopMutex);

        auto desktopWidth = this->desktop->GetWidth();
        auto desktopHeight = this->desktop->GetHeight();

        Mat mat(this->desktop->GetHeight(), this->desktop->GetWidth(), CV_8UC4);
        this->desktop->Duplicate(std::data(mat));

        if (context->IsGameFound())
        {
            auto gameRect = context->GetGameRect();
            if (!gameRect.empty()) {
                auto x = std::min<int>(desktopWidth, std::max(0, gameRect.x));
                auto y = std::min<int>(desktopHeight, std::max(0, gameRect.y));

                auto rx = std::max(0, std::min<int>(desktopWidth, gameRect.x + gameRect.width));

                auto ry = std::max(0, std::min<int>(desktopHeight, gameRect.y + gameRect.height));

                auto rect = Rect::FromPoints(x, y, rx, ry);
                if (!rect.empty()) mat = mat(rect);
            }
        }

        if (!mat.empty()) cv::cvtColor(mat, mat, cv::COLOR_BGRA2GRAY);
        context->SetScreenImage(mat);
    }
}
