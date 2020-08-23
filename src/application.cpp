#include "application.hpp"

namespace YellowRectangleCyanCircle {
    Application::Application(HINSTANCE hInstance) :
        hInstance(hInstance),
        hWnd(0)
    {
        L(trace, "[Application::Application] called");

        ::CoInitialize(nullptr);

        wchar_t className[128];
        ::LoadString(this->hInstance, IDS_APP_WINDOW_CLASSNAME, className, sizeof(className) / sizeof(wchar_t));

        this->registerWindowClass(className);
        this->createWindow(className);
        this->preloadIcons();
        this->createNotifyIcon();
        this->createNotifyIconMenu();
        this->createFactory();

        this->controller = std::make_shared<Controller>(this->hWnd, this->loadGameWindowName());

        L(info, "[Application::Application] initialized");
    }

    Application::~Application() {
        L(trace, "[Application::~Application] called");

        this->controller = nullptr;
        ::CoUninitialize();

        L(info, "[Application::Application] destroyed");
    }

    void Application::OnCommand(int commandID) {
        switch (commandID) {
        case IDM_EXIT:
        {
            L(debug, "[Application::OnCommand] exit menu item pressed");
            ::DestroyWindow(this->hWnd);
            this->hWnd = 0;
        }
        break;
        case IDM_FINGERPRINTSCANNER:
        case IDM_KEYPADCRACKER:
        {
            L(debug, "[Application::OnCommand] enable detector menu item pressed");

            auto controller = this->controller;
            if (!controller) {
                L(debug, "[Application::OnCommand] failed, no controller");
                break;
            }

            auto dt = (commandID == IDM_FINGERPRINTSCANNER) ? DetectorType::Fingerprint : DetectorType::Keypad;
            bool enabled = !controller->IsDetectorEnabled(dt);
            this->notifyIconMenuCheck(commandID, enabled);
            controller->EnableDetector(dt, enabled);
            this->switchIcon(
                controller->IsDetectorEnabled(DetectorType::Keypad),
                controller->IsDetectorEnabled(DetectorType::Fingerprint));
        }
        break;
        }
    }

    void Application::OnDestroy() {
        L(trace, "[Application::OnDestroy] called");

        this->controller = nullptr;

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
        L(trace, "[Application::OnResize] called");

        if (!this->hWnd) {
            L(debug, "[Application::OnResize] failed, no main window");
            return;
        }

        auto target = this->renderTarget;
        if (!target) {
            L(debug, "[Application::OnResize] failed, no render target");
            return;
        }

        target->Resize(D2D1::SizeU(width, height));
    }

    void Application::OnPaint() {
        L(trace, "[Application::OnPaint] called");

        if (!this->hWnd) {
            L(debug, "[Application::OnPaint] failed, no main window");
            return;
        }

        HRESULT hr = this->initializeResources();
        if (FAILED(hr)) {
            this->clearResources();
            return;
        }

        this->renderTarget->BeginDraw();
        this->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        this->renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        auto controller = this->controller;
        if (controller) {
            auto scale = this->getCurrentScale();

            for (auto dt : { DetectorType::Fingerprint, DetectorType::Keypad }) {
                controller->DrawShapes(
                    dt,
                    this->renderTarget,
                    dt == DetectorType::Fingerprint ? this->brushYellow : this->brushCyan,
                    scale
                );
            }
        }

        hr = this->renderTarget->EndDraw();
        if (FAILED(hr)) {
            if (hr == D2DERR_RECREATE_TARGET) {
                L(debug, "[Application::OnPaint] D2D1 asked to recreate target");
                this->clearResources();
            }
            else {
                L(debug, "[Application::OnPaint] end draw failed: {:#x}", static_cast<unsigned int>(hr));
            }
        }

        if (SUCCEEDED(hr) || hr == D2DERR_RECREATE_TARGET) ::ValidateRect(hWnd, nullptr);
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
        L(trace, L"[Application::createNotifyIcon] called");

        ::RtlZeroMemory(&this->notifyIcon, sizeof(NOTIFYICONDATA));

        this->notifyIcon.cbSize = sizeof(NOTIFYICONDATA);
        this->notifyIcon.uID = 1;
        this->notifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        this->notifyIcon.hIcon = this->notifyIconStateIcons.at(IDI_ICON_CDRD);
        this->notifyIcon.hWnd = hWnd;
        this->notifyIcon.uCallbackMessage = IDM_ICON;
        ::LoadString(hInstance, IDS_APP_TITLE, this->notifyIcon.szTip, 128);

        ::Shell_NotifyIcon(NIM_ADD, &this->notifyIcon);
        ::Shell_NotifyIcon(NIM_SETVERSION, &this->notifyIcon);
    }

    void Application::createNotifyIconMenu() {
        L(trace, L"[Application::createNotifyIconMenu] called");

        auto hMenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(IDR_APP_MENU));
        this->notifyIconMenu = GetSubMenu(hMenu, 0);
    }

    void Application::createWindow(std::wstring_view className) {
        L(trace, L"[Application::createWindow] called with: {}", className);

        this->hWnd = ::CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
            std::data(className),
            L"",
            WS_POPUP,
            0,
            0,
            0,
            0,
            nullptr,
            nullptr,
            this->hInstance,
            this
        );
        if (!this->hWnd) {
            L(error, "Unable to create window");
            throw UnableToCreateWindow();
        }

        ::SetLayeredWindowAttributes(
            this->hWnd,
            0x00000000,
            50,
            LWA_COLORKEY | LWA_ALPHA
        );

        ::ShowWindow(this->hWnd, SW_SHOWNORMAL);
        ::UpdateWindow(this->hWnd);

        L(debug, L"[Application::createWindow] success");
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
        L(trace, L"[Application::registerWindowClass] called with: {}", className);

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
            L(error, "unable to register window class");
            throw UnableToRegisterWindowClass();
        }

        L(debug, L"[Application::registerWindowClass] success");
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
        L(trace, "[Application::clearResources] called");

        this->renderTarget = nullptr;
        this->brushCyan = nullptr;
        this->brushYellow = nullptr;

        L(trace, "[Application::clearResources] success");
    }

    void Application::createFactory() {
        L(trace, L"[Application::createFactory] called");

        CComPtr<ID2D1Factory> f;

        HRESULT hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &f);
        if (FAILED(hr)) {
            L(error, "Unable to create d2d1 factory");
            throw COMException(hr);
        }

        this->factory = f;
    }

    HRESULT Application::initializeResources() {
        L(trace, "[Application::initializeResources] called");

        HRESULT hr = S_OK;
        if (!this->hWnd) {
            L(debug, "[Application::initializeResources] failed, no main window");
            return hr;
        }

        if (!this->factory) {
            L(debug, "[Application::initializeResources] failed, no factory");
            this->createFactory();
            return E_FAIL;
        }

        // if already initialized - do nothing
        if (this->renderTarget) {
            L(trace, "[Application::initializeResources] render target exists, do nothing");
            return hr;
        }

        auto rc = WinAPI::GetClientRect(this->hWnd);
        L(trace, "[Application::initializeResources] main window rect: {}", rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.width, rc.height);

        hr = this->factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hWnd, size),
            &this->renderTarget
        );
        if (FAILED(hr)) {
            L(debug, "[Application::initializeResources] failed to create render target: {:#x}", static_cast<unsigned int>(hr));
            return hr;
        }

        hr = this->renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Cyan),
            &this->brushCyan
        );
        if (FAILED(hr)) {
            L(debug, "[Application::initializeResources] failed to create cyan brush: {:#x}", static_cast<unsigned int>(hr));
            return hr;
        }

        hr = this->renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::Yellow),
            &this->brushYellow
        );
        if (FAILED(hr)) {
            L(debug, "[Application::initializeResources] failed to create yellow brush: {:#x}", static_cast<unsigned int>(hr));
            return hr;
        }

        L(debug, "[Application::initializeResources] success");
        return hr;
    }

    Point Application::getCurrentScale() {
        L(trace, "[Application::getCurrentScale] called");

        auto scale = Point(100, 100);

        if (!this->renderTarget) {
            L(debug, "[Application::getCurrentScale] failed, no render target, return default scale");
            return scale;
        }

        float dx = 0, dy = 0;
        this->renderTarget->GetDpi(&dx, &dy);
        if (!dx) dx = 96;
        if (!dy) dy = 96;

        scale = Point(
            static_cast<int>(100 * dx) / 96,
            static_cast<int>(100 * dy) / 96
        );

        L(debug, "[Application::getCurrentScale] success, scale: {}", scale);

        return scale;
    }

    std::wstring Application::loadGameWindowName() {
        L(trace, "[Application::loadGameWindowName] called");

        wchar_t buf[128];
        ::LoadString(this->hInstance, IDS_GAME_WINDOW_NAME, buf, sizeof(buf) / sizeof(wchar_t));

        std::wstring name(buf);
        for (auto& c : name) {
            c ^= 1;
        }
        return name;
    }

    void Application::preloadIcons() {
        L(trace, L"[Application::preloadIcons] called");

        for (auto iconId : { IDI_ICON_CDRD, IDI_ICON_CDRE, IDI_ICON_CERD, IDI_ICON_CERE }) {
            this->notifyIconStateIcons[iconId] = ::LoadIcon(this->hInstance, MAKEINTRESOURCE(iconId));
        }
    }

    void Application::switchIcon(bool keypadEnabled, bool fpEnabled) {
        int iconId;
        if (!keypadEnabled && !fpEnabled)
            iconId = IDI_ICON_CDRD;
        else if (!keypadEnabled && fpEnabled)
            iconId = IDI_ICON_CDRE;
        else if (keypadEnabled && !fpEnabled)
            iconId = IDI_ICON_CERD;
        else
            iconId = IDI_ICON_CERE;

        this->notifyIcon.hIcon = this->notifyIconStateIcons[iconId];
        ::Shell_NotifyIcon(NIM_MODIFY, &this->notifyIcon);
    }
}
