#pragma once

#include <opencv2/core/types.hpp>
#include <Windows.h>

namespace YellowRectangleCyanCircle {
	//using HWND = HWND;
	//using DWORD = DWORD;
	//using HWINEVENTHOOK = HWINEVENTHOOK;
	//using LONG = LONG;

	namespace Rect {
		using Rect = cv::Rect;

		Rect FromRECT(const RECT& rect);
	}
}