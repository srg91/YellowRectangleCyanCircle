#pragma once

#pragma comment(lib, "d2d1.lib")

#include <atlbase.h>
#include <comdef.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <spdlog/fmt/ostr.h>
#include <Windows.h>

namespace YellowRectangleCyanCircle {
    enum class DetectorType {
        None,
        Area,
        Fingerprint,
        Keypad
    };

    using Mat = cv::Mat;
    using Point = cv::Point;

    namespace Rect {
        using Rect = cv::Rect;

        // Clamp ROI to image correct bounds
        Rect ClampROI(const Rect& rect, const Rect& roi);
        Rect FromRECT(const RECT& rect);
        Rect FromPoints(int x, int y, int rx, int ry);
    }

    template <class T>
    class ComObject {
    public:
        ComObject() :
            ComObject<T>(nullptr)
        {}

        ComObject(CComPtr<T> object) :
            object(object)
        {}

        CComPtr<T> Get() const {
            return this->object;
        }

        void Set(CComPtr<T> value) {
            this->object = value;
        }

        operator bool() const { return bool(this->object); };
    protected:
        CComPtr<T> object;
    };

    struct DesktopInfo {
        std::wstring Name;
        Rect::Rect Rect;
        // In handreds, like 125 as 1.25% scale
        Point Scale;

        DesktopInfo(Rect::Rect rect = Rect::Rect(), Point scale = Point(100, 100));
        DesktopInfo(std::wstring_view name, Rect::Rect rect = Rect::Rect(), Point scale = Point(100, 100));
    };
}

namespace std {
    uint8_t* data(YellowRectangleCyanCircle::Mat& mat);

    bool empty(const YellowRectangleCyanCircle::Mat& mat);
    bool empty(const YellowRectangleCyanCircle::Rect::Rect& r);

    ostream& operator<<(ostream& os, const YellowRectangleCyanCircle::Rect::Rect& r);
    wostream& operator<<(wostream& os, const YellowRectangleCyanCircle::Rect::Rect& r);

    ostream& operator<<(ostream& os, const YellowRectangleCyanCircle::Point& p);
    wostream& operator<<(wostream& os, const YellowRectangleCyanCircle::Point& p);
}
