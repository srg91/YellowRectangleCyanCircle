#include "game.hpp"

namespace YellowRectangleCyanCircle {
	const char* GameAlreadyConstructed::what() const noexcept {
		return "Only one instance of the Game class can exist";
	}

	Game* Game::instance = nullptr;
	std::mutex Game::instanceMutex;

	Game::Game(std::shared_ptr<IWinAPI> winAPI) : 
		isFound(false), 
		isHookEnabled(false),
		handle(0)
	{
		std::lock_guard<std::mutex> lock(this->instanceMutex);
		if (this->instance) throw GameAlreadyConstructed();

		this->instance = this;
		this->winAPI = winAPI;
	}

	void Game::EnableHook(bool value) {
		if (this->IsHookEnabled() == value) return;

		if (this->IsHookEnabled()) {
			// UnhookWinEvent(hook_on_create)
			// UnhookWinEvent(hook_on_destroy)
			// UnhookWinEvent(hook_on_move)
		} else {
			Handle h = FindWindow(nullptr, L"WindowsProject1");
			if (h) {
				this->isFound = true;
			}
			else {
				this->isFound = false;
			}

		}



		// FindWindow
		// If not found, set hook to create objects
		// If found, set several hooks: 
		// - To move / size 
		// - To destroy
	}

	bool Game::IsHookEnabled() const noexcept {
		return this->isHookEnabled;
	}

	bool Game::IsFound() const noexcept {
		return this->isFound;
	}

	Rect::Rect Game::GetRect() const noexcept {
		return this->rect;
	}
}