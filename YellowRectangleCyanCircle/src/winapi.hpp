#pragma once

#include "types.hpp"

#include <Windows.h>

#include <string_view>

namespace YellowRectangleCyanCircle {
	struct IWinAPI {
		virtual ~IWinAPI() {};
		virtual Handle FindWindowByName(std::wstring_view name) = 0;
		virtual HWINEVENTHOOK SetWinEventHook(HookEventId eventId, WINEVENTPROC winEventProc, DWORD idThread = 0, DWORD dwFlags = 0);
	};

	struct WinAPI: public IWinAPI {
		~WinAPI() override = default;
		Handle FindWindowByName(std::wstring_view name) override final;
		HWINEVENTHOOK SetWinEventHook(HookEventId eventId, WINEVENTPROC winEventProc, DWORD idThread, DWORD dwFlags) override final;
	};
}
