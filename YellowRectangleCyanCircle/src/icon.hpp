#pragma once

#include "resource.hpp"
#include "types.hpp"

#include <shellapi.h>

namespace YellowRectangleCyanCircle {
	class NotifyIcon {
	public:
		NotifyIcon(HINSTANCE hInstance, HWND hWnd);
		~NotifyIcon();

		void ShowMenu();
		void CheckMenuItem(int itemID, bool value);
	private:
		NOTIFYICONDATA data;
		HMENU menu;
	};
}
