#include "application.hpp"

namespace YellowRectangleCyanCircle {
    Application::Application(HINSTANCE hInstance) :
        hInstance(hInstance),
        hWnd(0)
    {
        ::CoInitialize(nullptr);

        wchar_t className[128];
        ::LoadString(this->hInstance, IDS_APP_WINDOW_CLASSNAME, className, sizeof(className) / sizeof(wchar_t));

        this->registerWindowClass(className);
        this->createWindow(className);
        this->createNotifyIcon();
        this->createNotifyIconMenu();
        this->createFactory();

        this->controller = std::make_shared<Controller>();
    }

    Application::~Application() {
        if (this->controller) this->controller = nullptr;

        ::CoUninitialize();
    }

    void Application::OnCommand(int commandID) {
        switch (commandID) {
        case IDM_EXIT:
        {
            ::DestroyWindow(this->hWnd);
            this->hWnd = 0;
        }
        break;
        case IDM_FINGERPRINTSCANNER:
        case IDM_KEYPADCRACKER:
        {
            if (!this->controller) break;

            auto dt = (commandID == IDM_FINGERPRINTSCANNER) ? DetectorType::Fingerprint : DetectorType::Keypad;
            bool enabled = !this->controller->IsDetectorEnabled(dt);
            this->notifyIconMenuCheck(commandID, enabled);
            this->controller->EnableDetector(dt, enabled);
        }
        break;
        }
    }

    void Application::OnDestroy() {
        if (this->controller) this->controller = nullptr;
        this->destroyNotifyIcon();
        this->clearResources();

        ::PostQuitMessage(0);
    }

    void Application::OnDisplayChange() {
        if (!this->hWnd) return;
        ::InvalidateRect(this->hWnd, nullptr, false);
    }

    void Application::OnNotifyIconRightClick() {
        this->notifyIconMenuShow();
    }

    void Application::OnResize(UINT width, UINT height) {
        if (!this->hWnd) return;
        if (!this->renderTarget) return;

        this->renderTarget->Resize(D2D1::SizeU(width, height));
    }

    void Application::OnPaint() {
        if (!this->hWnd) return;

        HRESULT hr = this->initializeResources();
        if (FAILED(hr)) {
            if (hr == D2DERR_RECREATE_TARGET) {
                this->clearResources();
            }
            // throw exception?
            return;
        }

        if (!this->renderTarget) return;

        this->renderTarget->BeginDraw();
        this->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        this->renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
            if (this->controller)
                this->controller->DrawShapes(
                    dt,
                    this->renderTarget,
                    dt == DetectorType::Fingerprint ? this->brushYellow : this->brushCyan
                );
        }

        // check hr and clear resource or throw?
        this->renderTarget->EndDraw();
        ::ValidateRect(hWnd, nullptr);
    }

    int Application::RunMessageLoop() {
        MSG message;

        while (::GetMessage(&message, nullptr, 0, 0))
        {
            ::TranslateMessage(&message);
            ::DispatchMessage(&message);
        }

        return static_cast<int>(message.wParam);
    }

    void Application::createNotifyIcon() {
        ::RtlZeroMemory(&this->notifyIcon, sizeof(NOTIFYICONDATA));

        this->notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
        this->notifyIcon.uID = 1;
        this->notifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        this->notifyIcon.hIcon = ::LoadIcon(this->hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
        this->notifyIcon.hWnd = hWnd;
        this->notifyIcon.uCallbackMessage = IDM_ICON;
        ::LoadString(hInstance, IDS_APP_TITLE, this->notifyIcon.szTip, 128);

        ::Shell_NotifyIcon(NIM_ADD, &this->notifyIcon);
        ::Shell_NotifyIcon(NIM_SETVERSION, &this->notifyIcon);
    }

    void Application::createNotifyIconMenu() {
        auto hMenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_APP_MENU));
        this->notifyIconMenu = GetSubMenu(hMenu, 0);
    }

    void Application::createWindow(std::wstring_view className) {
        this->hWnd = ::CreateWindowEx(
            // WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
            WS_EX_TOPMOST | WS_EX_LAYERED,
            std::data(className),
            L"",
            // WS_POPUP,
            // 0,
            // 0,
            // 0,
            // 0
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            nullptr,
            nullptr,
            this->hInstance,
            this
        );
        if (!this->hWnd) throw UnableToCreateWindow();

        ::SetLayeredWindowAttributes(
            this->hWnd,
            0x00000000,
            50,
            LWA_COLORKEY | LWA_ALPHA
        );

        ::ShowWindow(this->hWnd, SW_SHOWNORMAL);
        ::UpdateWindow(this->hWnd);
    }

    void Application::destroyNotifyIcon() {
        ::Shell_NotifyIcon(NIM_DELETE, &this->notifyIcon);
    }

    void Application::notifyIconMenuCheck(int itemID, bool value) {
        ::CheckMenuItem(this->notifyIconMenu, itemID, value ? MF_CHECKED : MF_UNCHECKED);
    }

    void Application::notifyIconMenuShow() {
        if (!this->hWnd) return;

        POINT pt;
        ::GetCursorPos(&pt);

        ::SetForegroundWindow(this->hWnd);
        ::TrackPopupMenu(
            this->notifyIconMenu,
            TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON,
            pt.x,
            pt.y,
            0,
            this->hWnd,
            nullptr
        );
    }

    void Application::registerWindowClass(std::wstring_view className) const {
        WNDCLASSEXW wcex;
        ::RtlZeroMemory(&wcex, sizeof(WNDCLASSEXW));

        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = this->wndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = this->hInstance;
        wcex.hIcon = ::LoadIcon(this->hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
        wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = std::data(className);

        if (!::RegisterClassEx(&wcex)) {
            throw UnableToRegisterWindowClass();
        }
    }

    LRESULT CALLBACK Application::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        Application* app = nullptr;

        if (message == WM_CREATE) {
            auto pCS = reinterpret_cast<LPCREATESTRUCT>(lParam);
            app = reinterpret_cast<Application*>(pCS->lpCreateParams);

            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
            return 0;
        }
        else {
            app = reinterpret_cast<Application*>(static_cast<LONG_PTR>(::GetWindowLongPtr(hWnd, GWLP_USERDATA)));
            if (!app) return ::DefWindowProc(hWnd, message, wParam, lParam);
        }

        switch (message) {
        case WM_COMMAND:
        {
            int commandID = LOWORD(wParam);
            switch (commandID) {
            case IDM_EXIT:
            case IDM_FINGERPRINTSCANNER:
            case IDM_KEYPADCRACKER:
            {
                app->OnCommand(commandID);
            }
            break;
            default:
                return ::DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
        case IDM_ICON:
        {
            switch (lParam) {
            case WM_RBUTTONDOWN:
            case WM_CONTEXTMENU:
            {
                app->OnNotifyIconRightClick();
            }
            break;
            }
        }
        break;
        case WM_SIZE:
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            app->OnResize(width, height);
        }
        break;
        case WM_DISPLAYCHANGE:
        {
            app->OnDisplayChange();
        }
        break;
        case WM_PAINT:
        {
            app->OnPaint();
        }
        break;
        case WM_DESTROY:
        {
            app->OnDestroy();
        }
        break;
        default:
            return ::DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    void Application::clearResources() {
        this->renderTarget = nullptr;
        this->brushCyan = nullptr;
        this->brushYellow = nullptr;
    }

    void Application::createFactory() {
        CComPtr<ID2D1Factory> f;

        HRESULT hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &f);
        if (FAILED(hr)) throw COMException(hr);

        this->factory = f;
    }

    HRESULT Application::initializeResources() {
        HRESULT hr = S_OK;
        if (!this->hWnd) return hr;
        if (!this->factory) return hr;
        // if already initialized - do nothing
        if (this->renderTarget) return hr;

        auto rc = GetClientRect(this->hWnd);
        D2D1_SIZE_U size = D2D1::SizeU(rc.width, rc.height);

        hr = this->factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hWnd, size),
            &this->renderTarget
        );
        if (FAILED(hr)) return hr;

        hr = this->renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Cyan),
            &this->brushCyan
        );
        if (FAILED(hr)) return hr;

        hr = this->renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Yellow),
            &this->brushYellow
        );
        return hr;
    }
}
