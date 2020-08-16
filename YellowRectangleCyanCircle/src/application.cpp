#include "application.hpp"

namespace YellowRectangleCyanCircle {
	Application::Application(HINSTANCE hInstance) {
		this->d2d1API = std::make_shared<D2D1API>();
		this->direct = std::make_shared<Direct>();
		this->winAPI = std::make_shared<WinAPI>();

		wchar_t windowClassName[128];
		LoadString(
			hInstance, 
			IDS_APP_WINDOW_CLASSNAME, 
			windowClassName, 
			sizeof(windowClassName) / sizeof(wchar_t)
		);
		this->registerWindowClass(hInstance, windowClassName);

		this->window = std::make_unique<Window>(
			this->d2d1API,
			this->winAPI,
			hInstance,
			this,
			windowClassName
		);
		this->notifyIcon = std::make_unique<NotifyIcon>(
			hInstance, this->window->GetHandle()
		);

		this->controller = std::make_unique<Controller>();
	}

	void Application::OnCommand(HWND hWnd, int commandID) {
		switch (commandID) {
		case IDM_EXIT:
		{
			this->window = nullptr;
		}
		break;
		case IDM_FINGERPRINTSCANNER:
		case IDM_KEYPADCRACKER:
		{
			if (!this->controller) break;
			if (!this->notifyIcon) break;

			auto dt = (commandID == IDM_FINGERPRINTSCANNER) ? DetectorType::Fingerprint : DetectorType::Keypad;
			bool enabled = !this->controller->IsDetectorEnabled(dt);
			this->notifyIcon->CheckMenuItem(commandID, enabled);
			this->controller->EnableDetector(dt, enabled);
		}
		break;
		}
	}

	void Application::OnDestroy(HWND hWnd) {
		if (this->controller) {
			this->controller->EnableDetector(DetectorType::Fingerprint, false);
			this->controller->EnableDetector(DetectorType::Keypad, false);
		}

		this->notifyIcon = nullptr;
		this->controller = nullptr;
		::PostQuitMessage(0);
	}

	void Application::OnIconRightClick(HWND hWnd) {
		if (!this->notifyIcon) return;
		this->notifyIcon->ShowMenu();
	}

	void Application::OnResize(HWND hWnd, UINT width, UINT height) {
		if (!this->window) return;
		this->window->OnResize(width, height);
	}

	void Application::OnDisplayChnage(HWND hWnd) {
		if (!this->window) return;
		::InvalidateRect(hWnd, nullptr, false);
	}

	void Application::OnPaint(HWND hWnd) {
		if (!this->window) return;
		if (!this->controller) return;

		const auto context = this->controller->GetContext();
		auto lock = context->LockOnRead();
		this->window->OnPaint(context);

		ValidateRect(hWnd, nullptr);
	}

	int Application::RunMessageLoop() {
		MSG msg;

		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return static_cast<int>(msg.wParam);
	}

	ATOM Application::registerWindowClass(HINSTANCE hInstance, std::wstring_view windowClassName) {
		// TODO: Use winAPI
		WNDCLASSEXW wcex;
		RtlZeroMemory(&wcex, sizeof(WNDCLASSEXW));
		
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = this->wndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = std::data(windowClassName);
		wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

		return RegisterClassEx(&wcex);
	}

	LRESULT CALLBACK Application::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		Application* app = nullptr;

		if (message == WM_CREATE) {
			auto pCS = reinterpret_cast<LPCREATESTRUCT>(lParam);
			app = reinterpret_cast<Application*>(pCS->lpCreateParams);

			::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
			return 0;
		} else {
			app = reinterpret_cast<Application*>(static_cast<LONG_PTR>(
				::GetWindowLongPtr(hWnd, GWLP_USERDATA)
			));

			if (!app) return DefWindowProc(hWnd, message, wParam, lParam);
		}

		switch (message) 
		{
		case WM_COMMAND:
		{
			int cmdID = LOWORD(wParam);
			switch (cmdID) {
			case IDM_EXIT:
			case IDM_FINGERPRINTSCANNER:
			case IDM_KEYPADCRACKER:
			{
				app->OnCommand(hWnd, cmdID);
			}
			break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		case IDM_ICON:
		{
			switch (lParam) {
			case WM_RBUTTONDOWN:
			case WM_CONTEXTMENU:
			{
				app->OnIconRightClick(hWnd);
			}
			break;
			}
		}
		break;
		case WM_SIZE:
		{
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			app->OnResize(hWnd, width, height);
		}
		break;
		case WM_DISPLAYCHANGE:
		{
			app->OnDisplayChnage(hWnd);
		}
		break;
		case WM_PAINT:
		{
			app->OnPaint(hWnd);
		}
		break;
		case WM_DESTROY:
		{
			app->OnDestroy(hWnd);
		}
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	}
}
