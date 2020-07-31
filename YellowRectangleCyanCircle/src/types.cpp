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
	}
}