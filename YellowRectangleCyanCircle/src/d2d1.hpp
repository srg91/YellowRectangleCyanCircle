#pragma once

#pragma comment(lib, "d2d1.lib")

#include <d2d1.h>
#include <d2d1helper.h>

#include "types.hpp"

namespace YellowRectangleCyanCircle {
	// ID2D1Factory
	class D2D1HwndRenderTarget;

	class D2D1Factory : public ComObject<ID2D1Factory> {
	public:
		using ComObject<ID2D1Factory>::ComObject;

		virtual HRESULT CreateHwndRenderTarget(
			HWND hWnd,
			D2D1_SIZE_U size,
			std::shared_ptr<D2D1HwndRenderTarget>& hwndRenderTarget
		) const;
	};

	// ID2D1HwndRenderTarget
	class D2D1SolidColorBrush;

	class D2D1HwndRenderTarget : public ComObject<ID2D1HwndRenderTarget> {
	public:
		using ComObject<ID2D1HwndRenderTarget>::ComObject;

		virtual void BeginDraw() const;
		virtual void Clear(D2D1_COLOR_F color) const;
		virtual HRESULT CreateSolidColorBrush(
			D2D1_COLOR_F color,
			std::shared_ptr<D2D1SolidColorBrush>& brush
		) const;
		virtual HRESULT EndDraw() const;
		virtual void FillEllipse(
			D2D1_ELLIPSE* ellipse,
			const std::shared_ptr<D2D1SolidColorBrush>& brush
		) const;
		virtual void FillRectangle(
			D2D1_RECT_F* rect, 
			const std::shared_ptr<D2D1SolidColorBrush>& brush
		) const;
		virtual HRESULT Resize(const D2D1_SIZE_U& size) const;
		virtual void SetTransform(const D2D1_MATRIX_3X2_F& transform) const;
	};

	// ID2D1SolidColorBrush
	class D2D1SolidColorBrush : public ComObject<ID2D1SolidColorBrush> {
	public:
		using ComObject<ID2D1SolidColorBrush>::ComObject;
	};

	// API
	struct ID2D1API {
		virtual HRESULT CreateFactory(
			D2D1_FACTORY_TYPE factoryType,
			std::shared_ptr<D2D1Factory>& factory
		) const = 0;
	};

	struct D2D1API : public ID2D1API {
		HRESULT CreateFactory(
			D2D1_FACTORY_TYPE factoryType,
			std::shared_ptr<D2D1Factory>& factory
		) const override final;
	};
}
