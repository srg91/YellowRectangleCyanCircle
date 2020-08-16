#include "window.hpp"

namespace YellowRectangleCyanCircle {
	Window::Window(
		const std::shared_ptr<D2D1API> d2d1API,
		const std::shared_ptr<WinAPI>& winAPI, 
		HINSTANCE hInstance,
		Application* app,
		std::wstring_view windowClassName
	) :
		hInstance(hInstance),
		hWnd(0),
		d2d1API(d2d1API),
		winAPI(winAPI)
	{
		this->initialize(app, windowClassName);
	}

	Window::~Window() {
		this->Destory();
	}

	void Window::Destory() {
		this->winAPI->DestroyWindow(this->hWnd);
	}

	HWND Window::GetHandle() const {
		return this->hWnd;
	}

	HRESULT Window::OnPaint(const std::shared_ptr<IContext>& context) {
		HRESULT hr = this->initializeResources();
		if (FAILED(hr)) {
			if (hr == D2DERR_RECREATE_TARGET) {
				hr = S_OK;
				this->clearResources();
			}
			return hr;
		}

		this->renderTarget->BeginDraw();
		this->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		this->renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
			if (context->IsDetectorEnabled(dt)) {
				for (const auto& shape : context->GetShapes(dt)) {
					shape->OnDraw(
						this->renderTarget, 
						dt == DetectorType::Fingerprint ? this->brushYellow : this->brushCyan
					);
				}
			}
		}

		hr = this->renderTarget->EndDraw();
		return hr;
	}

	void Window::OnResize(UINT width, UINT height) const {
		if (!this->renderTarget) return;
		this->renderTarget->Resize(D2D1::SizeU(width, height));
	}

	void Window::initialize(Application* app, std::wstring_view windowClassName) {
		HWND hWnd = this->winAPI->CreateWnd(
			WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
			std::data(windowClassName),
			L"",
			WS_POPUP,
			0,
			0,
			0,
			0,
			nullptr,
			nullptr,
			this->hInstance,
			app);
		if (!hWnd) return;

		this->winAPI->SetLayeredWindowAttributes(
			this->hWnd,
			0x00000000,
			50,
			LWA_COLORKEY | LWA_ALPHA
		);
		this->winAPI->ShowWindow(this->hWnd, SW_SHOWNORMAL);
		this->winAPI->UpdateWindow(this->hWnd);

		this->d2d1API->CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, this->factory);
	}

	HRESULT Window::initializeResources() {
		HRESULT hr = S_OK;
		if (this->renderTarget) return hr;

		auto rc = this->winAPI->GetClientRect(this->hWnd);
		D2D1_SIZE_U size = D2D1::SizeU(rc.width, rc.height);
		hr = this->factory->CreateHwndRenderTarget(this->hWnd, size, this->renderTarget);
		if (FAILED(hr)) return hr;

		hr = this->renderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Cyan),
			this->brushCyan
		);
		if (FAILED(hr)) return hr;

		hr = this->renderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Yellow),
			this->brushYellow
		);
		return hr;
	}

	void Window::clearResources() {
		this->renderTarget = nullptr;
		this->brushCyan = nullptr;
		this->brushYellow = nullptr;
	}

	WindowUpdater::WindowUpdater(const std::shared_ptr<WinAPI>& winAPI) :
		winAPI(winAPI)
	{}

	void WindowUpdater::Perform(IContext& context) {
		auto hWnd = context.GetWindowHandle();
		if (!hWnd) return;

		const auto& prevArea = context.GetPreviousWorkingArea();
		const auto& area = context.GetWorkingArea();

		int xDiff = std::abs(prevArea.x - area.x);
		int yDiff = std::abs(prevArea.y - area.y);

		int wDiff = std::abs(prevArea.width - area.width);
		int hDiff = std::abs(prevArea.height - area.height);

		bool shouldMoveWindow = xDiff > 5 || yDiff > 5 || wDiff > 5 || hDiff > 5;

		if (shouldMoveWindow) 
			this->winAPI->MoveWindow(hWnd, area.x, area.y, area.width, area.height, false);

		for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
			if (context.IsShapesChanged(dt)) {
				this->winAPI->InvalidateRect(hWnd, nullptr, false);
				break;
			}
		}
	}
}
