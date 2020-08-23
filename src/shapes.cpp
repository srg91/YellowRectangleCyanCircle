#include "shapes.hpp"

namespace YellowRectangleCyanCircle {
    Circle::Circle(int x, int y, int radius) :
        x(x), y(y), radius(radius)
    {}

    void Circle::OnDraw(
        const CComPtr<ID2D1HwndRenderTarget>& target,
        const CComPtr<ID2D1SolidColorBrush>& brush,
        Point scale
    ) const {
        float sx = scale.x / 100.0f;
        float sy = scale.y / 100.0f;

        D2D1_ELLIPSE ellipse = D2D1::Ellipse(
            D2D1::Point2F(
                static_cast<float>(this->x) / sx,
                static_cast<float>(this->y) / sy
            ),
            static_cast<float>(this->radius) / sx,
            static_cast<float>(this->radius) / sy
        );
        target->FillEllipse(&ellipse, brush);
    }

    Rectangle::Rectangle(Rect::Rect rect) :
        rect(rect)
    {}

    void Rectangle::OnDraw(
        const CComPtr<ID2D1HwndRenderTarget>& target,
        const CComPtr<ID2D1SolidColorBrush>& brush,
        Point scale
    ) const {
        float sx = scale.x / 100.0f;
        float sy = scale.y / 100.0f;

        auto left = static_cast<float>(this->rect.x);
        auto top = static_cast<float>(this->rect.y);

        auto right = left + static_cast<float>(this->rect.width);
        auto bottom = top + static_cast<float>(this->rect.height);

        D2D1_RECT_F rectangle = D2D1::RectF(
            left / sx,
            top / sy,
            right / sx,
            bottom / sy
        );
        target->FillRectangle(&rectangle, brush);
    }
}
