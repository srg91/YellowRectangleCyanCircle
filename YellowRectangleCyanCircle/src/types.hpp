#pragma once

#include <opencv2/core/types.hpp>
#include <Windows.h>

namespace YellowRectangleCyanCircle {
	using Handle = HWND;
	using HookEventId = DWORD;

	namespace Rect {
		using Rect = cv::Rect;

		Rect FromRECT(const RECT& rect);
	}
}