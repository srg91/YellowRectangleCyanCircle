#pragma once

#include "hook.hpp"
#include "types.hpp"
#include "winapi.hpp"

#include <shared_mutex>
#include <string>

namespace YellowRectangleCyanCircle {
	class Game : IHookCallbackReceiver {
	public:
		bool IsFound() const noexcept;
		Rect::Rect GetRect() const noexcept;

		Game(std::shared_ptr<IWinAPI> winAPI, const std::wstring_view windowTitle, HWND hWnd = 0, Rect::Rect rect = Rect::Rect());

		void OnWindowCreated(HWND hWnd) override final;
		void OnWindowDestroyed(HWND hWnd) override final;
		void OnWindowMoved(HWND hWnd) override final;
	private:
		HWND hWnd;
		Rect::Rect rect;
		mutable std::shared_mutex instanceMutex;

		// constexpr static const wchar_t* windowName = L"Fs`oe!Uidgu!@tun!W";

		std::wstring windowTitle;

		std::shared_ptr<IWinAPI> winAPI;

		void clear();
	};
}