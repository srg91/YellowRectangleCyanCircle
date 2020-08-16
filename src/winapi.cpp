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
        HRESULT hr = DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &r, sizeof(RECT));
        if (SUCCEEDED(hr))
            return Rect::FromRECT(r);
        else
            return Rect::Rect();
    };
}
