#include "winapi.hpp"

namespace YellowRectangleCyanCircle {
	AccessibleObject::AccessibleObject(CComPtr<IAccessible> acc, VARIANT child) :
		acc(acc),
		child(child)
	{}

	AccessibleObject::~AccessibleObject() {
		this->acc = nullptr;
	}

	bool AccessibleObject::IsWindow() const {
		if (!this->acc) return false;

		VARIANT role;
		auto hr = acc->get_accRole(this->child, &role);
		return SUCCEEDED(hr) && role.lVal == ROLE_SYSTEM_WINDOW;
	}

	HWND WinAPI::CreateWnd(
		DWORD dwExStyle,
		LPCWSTR lpClassName,
		LPCWSTR lpWindowName,
		DWORD dwStyle,
		int X,
		int Y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		HMENU hMenu,
		HINSTANCE hInstance,
		LPVOID lpParam
	) const {
		return ::CreateWindowEx(
			dwExStyle,
			lpClassName,
			lpWindowName,
			dwStyle,
			X,
			Y,
			nWidth,
			nHeight,
			hWndParent,
			hMenu,
			hInstance,
			lpParam
		);
	}

	bool WinAPI::DestroyWindow(HWND hWnd) const {
		return ::DestroyWindow(hWnd);
	};

	HWND WinAPI::FindWindowByName(std::wstring_view name) const {
		return ::FindWindow(nullptr, std::data(name));
	}

	std::wstring WinAPI::GetApplicationDisplayName(HWND hWnd) const {
		if (!hWnd) return L"";

		HMONITOR m = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);

		if (GetMonitorInfo(m, &mi))
			return mi.szDevice;
		else
			return L"";
	}

	Rect::Rect WinAPI::GetWindowRect(HWND hWnd) const {
		RECT r;
		HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(RECT));
		if (SUCCEEDED(hr)) 
			return Rect::FromRECT(r);
		else
			return Rect::Rect();
	};

	Rect::Rect WinAPI::GetClientRect(HWND hWnd) const {
		RECT r;
		if (::GetClientRect(hWnd, &r)) {
			return Rect::FromRECT(r);
		} else {
			return Rect::Rect();
		}
	};

	DWORD WinAPI::GetWindowThreadProcessId(HWND hWnd) const {
		return ::GetWindowThreadProcessId(hWnd, nullptr);
	}

	std::wstring WinAPI::GetWndText(HWND hWnd) const {
		wchar_t buf[256];
		if (::GetWindowText(hWnd, buf, 256)) {
			return std::wstring(buf);
		}
		else {
			return L"";
		}
	}

	bool WinAPI::InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase) const {
		return ::InvalidateRect(hWnd, lpRect, bErase);
	}

	bool WinAPI::MoveWindow(
		HWND hWnd,
		int X,
		int Y,
		int nWidth,
		int nHeight,
		BOOL bRepaint
	) const {
		return ::MoveWindow(hWnd, X, Y, nWidth, nHeight, bRepaint);
	}

	bool WinAPI::SetLayeredWindowAttributes(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags) const {
		return ::SetLayeredWindowAttributes(hWnd, crKey, bAlpha, dwFlags);
	}

	bool WinAPI::ShowWindow(HWND hWnd, int nCmdShow) const {
		return ::ShowWindow(hWnd, nCmdShow);
	}

	bool WinAPI::UpdateWindow(HWND hWnd) const {
		return ::UpdateWindow(hWnd);
	}

	HWINEVENTHOOK WinAPI::SetWinEventHook(DWORD eventId, WINEVENTPROC winEventProc, DWORD threadId, DWORD flags) const {
		return ::SetWinEventHook(
			eventId, eventId,
			nullptr,
			winEventProc,
			0,
			threadId,
			flags
		);
	}

	bool WinAPI::UnhookWinEvent(HWINEVENTHOOK hook) const {
		return ::UnhookWinEvent(hook);
	}

	std::unique_ptr<IAccessibleObject> WinAPI::AccessibleObjectFromEvent(HWND hWnd, LONG idObject, LONG idChild) const {
		CComPtr<IAccessible> acc;
		VARIANT child;
		HRESULT hr = ::AccessibleObjectFromEvent(hWnd, idObject, idChild, &acc, &child);

		if (SUCCEEDED(hr) && acc) {
			return std::make_unique<AccessibleObject>(acc, child);
		} else {
			return nullptr;
		}
	}
}