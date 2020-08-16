#include "icon.hpp"

namespace YellowRectangleCyanCircle {
	// TODO: use WinAPI
	NotifyIcon::NotifyIcon(HINSTANCE hInstance, HWND hWnd) {
		RtlZeroMemory(&this->data, sizeof(NOTIFYICONDATA));

		this->data.cbSize = sizeof(NOTIFYICONDATA);
		this->data.uID = 1;
		this->data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		this->data.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
		this->data.hWnd = hWnd;
		this->data.uCallbackMessage = IDM_ICON;
		LoadString(hInstance, IDS_APP_TITLE, this->data.szTip, 128);

		Shell_NotifyIcon(NIM_ADD, &this->data);
		Shell_NotifyIcon(NIM_SETVERSION, &this->data);

		auto hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_APP_MENU));
		this->menu = GetSubMenu(hMenu, 0);
	}

	NotifyIcon::~NotifyIcon() {
		Shell_NotifyIcon(NIM_DELETE, &this->data);
	}

	void NotifyIcon::ShowMenu() {
		POINT pt;
		GetCursorPos(&pt);

		SetForegroundWindow(this->data.hWnd);
		TrackPopupMenu(
			this->menu,
			TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON,
			pt.x,
			pt.y,
			0,
			this->data.hWnd,
			nullptr
		);
	}

	void NotifyIcon::CheckMenuItem(int itemID, bool value) {
		::CheckMenuItem(this->menu, itemID, value ? MF_CHECKED : MF_UNCHECKED);
	}
}
