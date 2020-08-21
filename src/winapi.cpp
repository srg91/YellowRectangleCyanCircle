#include "winapi.hpp"

namespace YellowRectangleCyanCircle::WinAPI {
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

    std::unique_ptr<IAccessibleObject> AccessibleObject::FromEvent(HWND hWnd, LONG idObject, LONG idChild) {
        CComPtr<IAccessible> acc;
        VARIANT child;
        HRESULT hr = ::AccessibleObjectFromEvent(hWnd, idObject, idChild, &acc, &child);

        if (SUCCEEDED(hr) && acc) {
            return std::make_unique<AccessibleObject>(acc, child);
        }
        else {
            return nullptr;
        }
    }

    Rect::Rect GetClientRect(HWND hWnd) {
        RECT r;
        if (::GetClientRect(hWnd, &r)) {
            return Rect::FromRECT(r);
        }
        else {
            return Rect::Rect();
        }
    };

    Rect::Rect GetWindowRect(HWND hWnd) {
        RECT r;
        HRESULT hr = ::DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(RECT));
        if (SUCCEEDED(hr))
        {
            POINT v = { r.left, r.top };
            ClientToScreen(hWnd, &v);

            return Rect::FromRECT(r);
        }
        else
            return Rect::Rect();
    };

    DisplayInfo GetWindowDisplayInfo(HWND hWnd) {
        DisplayInfo di = {};

        HMONITOR m = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        if (!m) m = ::MonitorFromWindow(0, MONITOR_DEFAULTTONEAREST);

        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);

        if (::GetMonitorInfo(m, &mi)) {
            di.name = mi.szDevice;
            di.area = Rect::FromRECT(mi.rcMonitor);
        }
        return di;
    }

    std::wstring GetWindowText_(HWND hWnd) {
        wchar_t buf[256];
        if (::GetWindowText(hWnd, buf, 256)) {
            return std::wstring(buf);
        }
        else {
            return L"";
        }
    }
}
