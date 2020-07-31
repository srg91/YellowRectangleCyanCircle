#include "winapi.hpp"

namespace YellowRectangleCyanCircle {
	Handle WinAPI::FindWindowByName(std::wstring_view name) {
		return FindWindow(nullptr, std::data(name));
	}

	HWINEVENTHOOK SetWinEventHook(HookEventId eventId, WINEVENTPROC winEventProc, DWORD threadId = 0, DWORD flags = 0) {
		return SetWinEventHook(
			eventId, eventId,
			nullptr,
			winEventProc,
			0,
			threadId,
			flags
		);
	}
}