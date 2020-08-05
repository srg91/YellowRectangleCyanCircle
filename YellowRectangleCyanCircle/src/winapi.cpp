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

	HWND WinAPI::FindWindowByName(std::wstring_view name) const {
		return ::FindWindow(nullptr, std::data(name));
	}

	DWORD WinAPI::GetWindowThreadProcessId(HWND hWnd) const {
		return ::GetWindowThreadProcessId(hWnd, nullptr);
	}

	std::wstring WinAPI::GetWindowText(HWND hWnd) const {
		wchar_t buf[256];
		if (::GetWindowText(hWnd, buf, 256)) {
			return std::wstring(buf);
		} else {
			return L"";
		}
	}

	Rect::Rect WinAPI::GetWindowRect(HWND hWnd) const {
		RECT r;
		HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(RECT));
		if (SUCCEEDED(hr)) 
			return Rect::FromRECT(r);
		else
			return Rect::Rect();
	};

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

}