#include "screen.hpp"

namespace YellowRectangleCyanCircle {
    Screen::Screen(
        std::shared_ptr<IDirect> direct,
        HWND hWnd
    ) : direct(direct) {
        this->currentDisplay = WinAPI::GetWindowDisplayInfo(hWnd);
        this->desktop = std::make_unique<Desktop>(
            this->direct,
            this->currentDisplay.name);
    }

    void Screen::OnWindowMoved(HWND hWnd) {
        auto display = WinAPI::GetWindowDisplayInfo(hWnd);
        if (display.name == this->currentDisplay.name) return;

        std::unique_lock lock(this->desktopMutex);
        this->currentDisplay = display;
        this->desktop->SwitchDisplay(display.name);
    }

    void Screen::Perform(std::shared_ptr<IContext> context) {
        if (!context) return;

        std::shared_lock read(this->desktopMutex);

        auto desktopWidth = this->desktop->GetWidth();
        auto desktopHeight = this->desktop->GetHeight();
        context->SetDisplayRect(this->currentDisplay.area);

        Mat mat(this->desktop->GetHeight(), this->desktop->GetWidth(), CV_8UC4);
        this->desktop->Duplicate(std::data(mat));

        if (context->IsGameFound())
        {
            auto gameRect = context->GetGameRect();

            // Clamp current game window to display
            auto roi = Rect::ClampROI(gameRect, this->currentDisplay.area);
            if (!roi.empty()) mat = mat(roi);
        }

        if (!mat.empty()) cv::cvtColor(mat, mat, cv::COLOR_BGRA2GRAY);
        context->SetScreenImage(mat);
    }
}
