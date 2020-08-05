#include "shapes.hpp"

namespace YellowRectangleCyanCircle {
	Circle::Circle(int x, int y, int radius) :
		x(x), y(y), radius(radius)
	{}

	void Circle::OnDraw() const {

	}

	Rectangle::Rectangle(Rect::Rect rect) :
		rect(rect)
	{}

	void Rectangle::OnDraw() const {

	}
}