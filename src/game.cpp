#include "game.hpp"

namespace YellowRectangleCyanCircle {
    Game::Game(const std::wstring_view windowTitle, HWND hWnd) :
        hWnd(hWnd),
        rect(Rect::Rect()),
        windowTitle(windowTitle)
    {
        if (this->hWnd) {
            this->rect = WinAPI::GetWindowRect(this->hWnd);
        }
    }

    bool Game::IsFound() const noexcept {
        std::shared_lock lock(this->mutex);
        return this->hWnd ? true : false;
    }

    Rect::Rect Game::GetRect() const noexcept {
        std::shared_lock lock(this->mutex);
        return this->rect;
    }

    void Game::OnWindowCreated(HWND hWnd) {
        if (this->IsFound()) return;

        std::wstring title = WinAPI::GetWindowText_(hWnd);
        if (title.find(this->windowTitle) != std::string::npos) {
            std::unique_lock lock(this->mutex);
            this->hWnd = hWnd;
            this->rect = WinAPI::GetWindowRect(this->hWnd);
        }
    }

    void Game::OnWindowDestroyed(HWND hWnd) {
        if (!this->IsFound()) return;
        if (hWnd == this->hWnd) this->clear();
    }

    void Game::OnWindowMoved(HWND hWnd) {
        if (!this->IsFound()) return;
        if (hWnd != this->hWnd) return;
        std::unique_lock lock(this->mutex);
        this->rect = WinAPI::GetWindowRect(this->hWnd);
    }

    void Game::Perform(std::shared_ptr<IContext> context) {
        context->SetGameFound(this->IsFound());
        context->SetGameRect(this->GetRect());
    }

    void Game::clear() {
        std::unique_lock lock(this->mutex);
        this->hWnd = 0;
        this->rect = Rect::Rect();
    }
}
