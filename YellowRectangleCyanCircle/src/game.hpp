#pragma once

#include "types.hpp"
#include "winapi.hpp"

#include <exception>
#include <memory>
#include <mutex>

namespace YellowRectangleCyanCircle {
	struct GameAlreadyConstructed : public std::exception {
		const char* what() const noexcept;
	};

	class Game {
	public:
		bool IsFound() const noexcept;
		Rect::Rect GetRect() const noexcept;

		Game(std::shared_ptr<IWinAPI> winAPI);

		void EnableHook(bool value);
		bool IsHookEnabled() const noexcept;
	private:
		bool isFound;
		Rect::Rect rect;

		bool isHookEnabled;
		
		Handle handle;
		const wchar_t* windowName = L"Fs`oe!Uidgu!@tun!W";

		std::shared_ptr<IWinAPI> winAPI;

		static Game* instance;
		static std::mutex instanceMutex;

		Game(const Game&) = delete;
	};
}