#pragma once

#include "controller.hpp"
#include "d2d1.hpp"
#include "direct.hpp"
#include "icon.hpp"
#include "resource.hpp"
#include "window.hpp"
#include "winapi.hpp"

#include <memory>

namespace YellowRectangleCyanCircle {
	class Application {
	public:
		Application(HINSTANCE hInstance);

		void OnCommand(HWND hWnd, int commandID);
		void OnDestroy(HWND hWnd);
		void OnIconRightClick(HWND hWnd);
		void OnResize(HWND hWnd, UINT width, UINT height);
		void OnDisplayChnage(HWND hWnd);
		void OnPaint(HWND hWnd);

		int RunMessageLoop();
	private:
		// API
		std::shared_ptr<D2D1API> d2d1API;
		std::shared_ptr<Direct> direct;
		std::shared_ptr<WinAPI> winAPI;

		// Parts of application
		std::unique_ptr<Controller> controller;
		std::unique_ptr<NotifyIcon> notifyIcon;
		std::unique_ptr<Window> window;

		ATOM registerWindowClass(HINSTANCE hInstance, std::wstring_view windowClassName);
		static LRESULT CALLBACK wndProc(
			HWND hWnd,
			UINT message,
			WPARAM wParam,
			LPARAM lParam
		);
	};
}
