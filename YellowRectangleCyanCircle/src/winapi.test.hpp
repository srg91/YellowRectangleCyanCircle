#pragma once

#include "winapi.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace YellowRectangleCyanCircle;

struct MockWinAPI : public IWinAPI {
	MockWinAPI();
	virtual ~MockWinAPI();

	MOCK_METHOD(HWND, FindWindowByName, (std::wstring_view), (const, override));
	MOCK_METHOD(DWORD, GetWindowThreadProcessId, (HWND), (const, override));
	MOCK_METHOD(std::wstring, GetWindowText, (HWND), (const, override));
	MOCK_METHOD(Rect::Rect, GetWindowRect, (HWND), (const, override));

	MOCK_METHOD(HWINEVENTHOOK, SetWinEventHook, (DWORD, WINEVENTPROC, DWORD, DWORD), (const, override));
	MOCK_METHOD(bool, UnhookWinEvent, (HWINEVENTHOOK), (const, override));

	MOCK_METHOD(std::unique_ptr<IAccessibleObject>, AccessibleObjectFromEvent, (HWND, LONG, LONG), (const, override));
};

struct MockAccessibleObject : public IAccessibleObject {
	MockAccessibleObject();
	virtual ~MockAccessibleObject();

	MOCK_METHOD(bool, IsWindow, (), (const, override));
};
