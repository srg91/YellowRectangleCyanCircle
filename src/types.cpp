#include "types.hpp"

namespace YellowRectangleCyanCircle {
	namespace Rect {
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
	std::uint8_t* data(YellowRectangleCyanCircle::Mat mat) {
		return mat.data;
	}
}
