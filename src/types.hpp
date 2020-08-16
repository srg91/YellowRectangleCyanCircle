#pragma once

#pragma comment(lib, "d2d1.lib")

#include <atlbase.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <Windows.h>

namespace YellowRectangleCyanCircle {
    enum class DetectorType {
        Area,
        Fingerprint,
        Keypad
    };

    using Mat = cv::Mat;

    namespace Rect {
        using Rect = cv::Rect;

        Rect FromRECT(const RECT& rect);
    }
}

namespace std {
    std::uint8_t* data(YellowRectangleCyanCircle::Mat mat);
}
