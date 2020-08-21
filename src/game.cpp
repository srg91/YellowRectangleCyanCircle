#include "game.hpp"

namespace YellowRectangleCyanCircle {
    Game::Game(const std::wstring_view windowTitle, HWND hWnd) :
        hWnd(hWnd),
        rect(Rect::Rect()),
        windowTitle(windowTitle)
    {
        L(trace, L"[Game::Game] called with window title [{}] and handle [{}]", windowTitle, reinterpret_cast<std::size_t>(hWnd));

        if (this->hWnd) {
            std::unique_lock lock(this->mutex);
            this->rect = WinAPI::GetWindowRect(this->hWnd);
            L(trace, L"[Game::Game] game rect: {}", this->rect);
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
        if (title.find(this->windowTitle) == std::string::npos) return;

        std::unique_lock lock(this->mutex);
        L(debug, "[Game::OnWindowCreated] Found game window: {}", reinterpret_cast<std::size_t>(hWnd));
        this->hWnd = hWnd;
        this->rect = WinAPI::GetWindowRect(this->hWnd);
    }

    void Game::OnWindowDestroyed(HWND hWnd) {
        if (!this->IsFound()) return;
        if (hWnd != this->hWnd) return;

        L(debug, "[Game::OnWindowDestroyed] Game window closed");
        this->clear();
    }

    void Game::OnWindowMoved(HWND hWnd) {
        if (!this->IsFound()) return;
        if (hWnd != this->hWnd) return;

        std::unique_lock lock(this->mutex);
        this->rect = WinAPI::GetWindowRect(this->hWnd);
    }

    void Game::Perform(std::shared_ptr<IContext> context) {
        L(trace, "[Game::Perform] called");

        if (!context) {
            L(debug, "[Game::Perform] failed, no context");
            return;
        }

        context->SetGameFound(this->IsFound());
        context->SetGameRect(this->GetRect());
    }

    void Game::clear() {
        L(trace, "[Game::clear] called");

        std::unique_lock lock(this->mutex);
        this->hWnd = 0;
        this->rect = Rect::Rect();
    }
}
