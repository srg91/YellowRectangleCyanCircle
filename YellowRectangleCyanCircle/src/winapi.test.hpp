#pragma once

#include "winapi.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace YellowRectangleCyanCircle;

namespace TestWinAPI {
	struct MockWinAPI : public IWinAPI {
		MockWinAPI();
		virtual ~MockWinAPI();

		MOCK_METHOD(
			HWND, 
			CreateWnd, 
			(
				DWORD,
				LPCWSTR,
				LPCWSTR,
				DWORD,
				int,
				int,
				int,
				int,
				HWND,
				HMENU,
				HINSTANCE,
				LPVOID
			),
			(const, override)
		);
		MOCK_METHOD(bool, DestroyWindow, (HWND), (const, override));
		MOCK_METHOD(HWND, FindWindowByName, (std::wstring_view), (const, override));
		MOCK_METHOD(std::wstring, GetApplicationDisplayName, (HWND), (const, override));
		MOCK_METHOD(Rect::Rect, GetClientRect, (HWND), (const, override));
		MOCK_METHOD(Rect::Rect, GetWindowRect, (HWND), (const, override));
		MOCK_METHOD(DWORD, GetWindowThreadProcessId, (HWND), (const, override));
		MOCK_METHOD(std::wstring, GetWndText, (HWND), (const, override));
		MOCK_METHOD(bool, InvalidateRect, (HWND, const RECT*, BOOL), (const, override));
		MOCK_METHOD(bool, MoveWindow, (HWND, int, int, int, int, BOOL), (const, override));
		MOCK_METHOD(bool, SetLayeredWindowAttributes, (HWND, COLORREF, BYTE, DWORD), (const, override));
		MOCK_METHOD(bool, ShowWindow, (HWND, int), (const, override));
		MOCK_METHOD(bool, UpdateWindow, (HWND), (const, override));

		MOCK_METHOD(HWINEVENTHOOK, SetWinEventHook, (DWORD, WINEVENTPROC, DWORD, DWORD), (const, override));
		MOCK_METHOD(bool, UnhookWinEvent, (HWINEVENTHOOK), (const, override));

		MOCK_METHOD(std::unique_ptr<IAccessibleObject>, AccessibleObjectFromEvent, (HWND, LONG, LONG), (const, override));
	};

	struct MockAccessibleObject : public IAccessibleObject {
		MockAccessibleObject();
		virtual ~MockAccessibleObject();

		MOCK_METHOD(bool, IsWindow, (), (const, override));
	};
}
