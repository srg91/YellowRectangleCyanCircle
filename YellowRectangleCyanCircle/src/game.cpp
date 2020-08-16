#include "game.hpp"

namespace YellowRectangleCyanCircle {
	Game::Game(std::shared_ptr<IWinAPI> winAPI, const std::wstring_view windowTitle, HWND hWnd, Rect::Rect rect) : 
		hWnd(hWnd),
		rect(rect),
		winAPI(winAPI),
		windowTitle(windowTitle)
	{}

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
		
		std::wstring title = this->winAPI->GetWndText(hWnd);
		if (title == this->windowTitle) {
			std::unique_lock lock(this->mutex);
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
		std::unique_lock lock(this->mutex);
		this->rect = this->winAPI->GetWindowRect(this->hWnd);
	}

	void Game::Perform(IContext& context) {
		context.SetGameFound(this->IsFound());
		context.SetGameRect(this->GetRect());
	}

	void Game::clear() {
		std::unique_lock lock(this->mutex);
		this->hWnd = 0;
		this->rect = Rect::Rect();
	}

}