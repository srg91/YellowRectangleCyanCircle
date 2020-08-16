#include "winapi.hpp"

namespace YellowRectangleCyanCircle {
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
            return Rect::FromRECT(r);
        else
            return Rect::Rect();
    };

    std::wstring GetWindowDisplayName(HWND hWnd) {
        if (!hWnd) return L"";

        HMONITOR m = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX mi;
        mi.cbSize = sizeof(mi);

        if (::GetMonitorInfo(m, &mi))
            return mi.szDevice;
        else
            return L"";
    }
}
