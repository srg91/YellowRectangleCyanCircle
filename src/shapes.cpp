#include "shapes.hpp"

namespace YellowRectangleCyanCircle {
    Circle::Circle(int x, int y, int radius) :
        x(x), y(y), radius(radius)
    {}

    void Circle::OnDraw(
        const CComPtr<ID2D1HwndRenderTarget>& target,
        const CComPtr<ID2D1SolidColorBrush>& brush
    ) const {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(
            D2D1::Point2F(
                static_cast<float>(this->x),
                static_cast<float>(this->y)
            ),
            static_cast<float>(this->radius),
            static_cast<float>(this->radius)
        );
        target->FillEllipse(&ellipse, brush);
    }

    Rectangle::Rectangle(Rect::Rect rect) :
        rect(rect)
    {}

    void Rectangle::OnDraw(
        const CComPtr<ID2D1HwndRenderTarget>& target,
        const CComPtr<ID2D1SolidColorBrush>& brush
    ) const {
        D2D1_RECT_F rectangle = D2D1::RectF(
            static_cast<float>(this->rect.x),
            static_cast<float>(this->rect.y),
            static_cast<float>(this->rect.x + this->rect.width),
            static_cast<float>(this->rect.y + this->rect.height)
        );
        target->FillRectangle(&rectangle, brush);
    }
}
