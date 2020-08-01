#include "game.hpp"

namespace YellowRectangleCyanCircle {
	Game::Game(std::shared_ptr<IWinAPI> winAPI, const std::wstring_view windowTitle, HWND hWnd, Rect::Rect rect) : 
		hWnd(hWnd),
		rect(rect),
		winAPI(winAPI),
		windowTitle(windowTitle)
	{}

	void Game::OnWindowCreated(HWND hWnd) {
		if (this->IsFound()) return;
		
		std::wstring title = this->winAPI->GetWindowText(hWnd);
		if (title == this->windowTitle) {
			std::unique_lock lock(this->instanceMutex);
			this->hWnd = hWnd;
			this->rect = this->winAPI->GetWindowRect(this->hWnd);
		}
	}
	
	void Game::OnWindowDestroyed(HWND hWnd) {
		if (!this->IsFound()) return;
		if (hWnd == this->hWnd) this->clear();
	}

	void Game::OnWindowMoved(HWND hWnd) {
		if (!this->IsFound()) return;
		if (hWnd != this->hWnd) return;
		std::unique_lock lock(this->instanceMutex);
		this->rect = this->winAPI->GetWindowRect(this->hWnd);
	}

	bool Game::IsFound() const noexcept {
		std::shared_lock lock(this->instanceMutex);
		return this->hWnd ? true : false;
	}

	Rect::Rect Game::GetRect() const noexcept {
		std::shared_lock lock(this->instanceMutex);
		return this->rect;
	}

	void Game::clear() {
		std::unique_lock lock(this->instanceMutex);
		this->hWnd = 0;
		this->rect = Rect::Rect();
	}

}