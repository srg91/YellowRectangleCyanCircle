#include "types.hpp"

namespace YellowRectangleCyanCircle {
    namespace Rect {
        Rect ClampROI(const Rect& roi, const Rect& clamp) {
            auto x = std::clamp(roi.x - clamp.x, 0, clamp.width);
            auto y = std::clamp(roi.y - clamp.y, 0, clamp.height);
            auto xr = std::clamp(roi.x + roi.width - clamp.x, 0, clamp.width);
            auto yr = std::clamp(roi.y + roi.height - clamp.y, 0, clamp.height);
            return FromPoints(x, y, xr, yr);
        }

        Rect FromRECT(const RECT& rect) {
            return Rect(
                rect.left,
                rect.top,
                rect.right - rect.left,
                rect.bottom - rect.top
            );
        }

        Rect FromPoints(int x, int y, int rx, int ry) {
            return Rect(
                cv::Point(x, y),
                cv::Point(rx, ry)
            );
        }
    }
}

namespace std
{
    uint8_t* data(YellowRectangleCyanCircle::Mat& mat) {
        return mat.data;
    }

    bool empty(const YellowRectangleCyanCircle::Mat& mat) {
        return mat.empty();
    }

    bool empty(const YellowRectangleCyanCircle::Rect::Rect& r) {
        return r.empty();
    }

    std::ostream& operator<<(std::ostream& os, const YellowRectangleCyanCircle::Rect::Rect& r) {
        return os << "Rect(" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << ")";
    }

    std::wostream& operator<<(std::wostream& os, const YellowRectangleCyanCircle::Rect::Rect& r) {
        return os << "Rect(" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << ")";
    }
}
