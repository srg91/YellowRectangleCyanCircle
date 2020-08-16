#pragma once

#include <Windows.h>

#include <string>

namespace YellowRectangleCyanCircle {
    std::wstring utf8_decode(std::string_view value);
    std::string utf8_encode(std::wstring_view value);
}
