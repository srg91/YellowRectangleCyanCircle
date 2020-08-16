#include "d2d1.hpp"

namespace YellowRectangleCyanCircle {
	HRESULT D2D1Factory::CreateHwndRenderTarget(
		HWND hWnd,
		D2D1_SIZE_U size,
		std::shared_ptr<D2D1HwndRenderTarget>& hwndRenderTarget
	) const {
		CComPtr<ID2D1HwndRenderTarget> rt;
		HRESULT hr = this->object->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hWnd, size),
			&rt
		);
		if (FAILED(hr)) return hr;
		hwndRenderTarget = std::make_shared<D2D1HwndRenderTarget>(rt);
		return hr;
	}

	void D2D1HwndRenderTarget::BeginDraw() const {
		this->object->BeginDraw();
	}

	void D2D1HwndRenderTarget::Clear(D2D1_COLOR_F color) const {
		this->object->Clear(color);
	}

	HRESULT D2D1HwndRenderTarget::CreateSolidColorBrush(
		D2D1_COLOR_F color, 
		std::shared_ptr<D2D1SolidColorBrush>& brush
	) const {
		CComPtr<ID2D1SolidColorBrush> cb;
		HRESULT hr = this->object->CreateSolidColorBrush(color, &cb);
		if (FAILED(hr)) return hr;
		brush = std::make_shared<D2D1SolidColorBrush>(cb);
		return hr;
	}

	HRESULT D2D1HwndRenderTarget::EndDraw() const {
		return this->object->EndDraw();
	}

	void D2D1HwndRenderTarget::FillEllipse(
		D2D1_ELLIPSE* ellipse,
		const std::shared_ptr<D2D1SolidColorBrush>& brush
	) const {
		this->object->FillEllipse(ellipse, brush->Get());
	}

	void D2D1HwndRenderTarget::FillRectangle(
		D2D1_RECT_F* rect,
		const std::shared_ptr<D2D1SolidColorBrush>& brush
	) const {
		this->object->FillRectangle(rect, brush->Get());
	}

	HRESULT D2D1HwndRenderTarget::Resize(const D2D1_SIZE_U& size) const {
		return this->object->Resize(size);
	}

	void D2D1HwndRenderTarget::SetTransform(const D2D1_MATRIX_3X2_F& transform) const {
		this->object->SetTransform(transform);
	}

	HRESULT D2D1API::CreateFactory(
		D2D1_FACTORY_TYPE factoryType, 
		std::shared_ptr<D2D1Factory>& factory
	) const {
		CComPtr<ID2D1Factory> f;
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &f);
		if (FAILED(hr)) return hr;
		factory = std::make_shared<D2D1Factory>(f);
		return hr;
	};
}
