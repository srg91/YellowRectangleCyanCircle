#pragma once

#pragma comment(lib, "dwmapi.lib")

#include "types.hpp"

#include <dwmapi.h>

namespace YellowRectangleCyanCircle {
    Rect::Rect GetClientRect(HWND hWnd);
    Rect::Rect GetWindowRect(HWND hWnd);
}
