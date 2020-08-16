#include "screen.hpp"

namespace YellowRectangleCyanCircle {
    Screen::Screen(
        std::shared_ptr<IDirect> direct,
        HWND hWnd
    ) : direct(direct) {
        this->currentDisplay = WinAPI::GetWindowDisplayName(hWnd);
        this->desktop = std::make_unique<Desktop>(
            this->direct,
            this->currentDisplay
            );
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

        Mat mat(this->desktop->GetHeight(), this->desktop->GetWidth(), CV_8UC4);
        this->desktop->Duplicate(std::data(mat));

        if (context->IsGameFound())
        {
            auto gameRect = context->GetGameRect();
            mat = mat(gameRect);
        }

        cv::cvtColor(mat, mat, cv::COLOR_BGRA2GRAY);
        context->SetScreenImage(mat);
    }
}
