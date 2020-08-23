#include "desktop.d3d9.hpp"

namespace YellowRectangleCyanCircle {
    namespace DesktopDuplication {
        HRESULT CreateDevice9(CComPtr<IDirect3D9> direct, UINT output, CComPtr<IDirect3DDevice9>& device, std::shared_ptr<D3DPRESENT_PARAMETERS>& params) {
            L(trace, "[DesktopDuplication::CreateDevice9] called");
            HRESULT hr = S_OK;

            D3DDISPLAYMODE mode;
            hr = direct->GetAdapterDisplayMode(output, &mode);
            if (FAILED(hr)) {
                L(debug, "[DesktopDuplication::CreateDevice9] failed to get adapter display mode: {:#x}", static_cast<unsigned int>(hr));
                return hr;
            }

            device = nullptr;
            RtlZeroMemory(params.get(), sizeof(D3DPRESENT_PARAMETERS));
            params->Windowed = true;
            params->SwapEffect = D3DSWAPEFFECT_DISCARD;
            params->BackBufferFormat = mode.Format;
            params->BackBufferCount = 1;
            params->BackBufferWidth = mode.Width;
            params->BackBufferHeight = mode.Height;
            params->MultiSampleType = D3DMULTISAMPLE_NONE;
            params->MultiSampleQuality = 0;
            params->EnableAutoDepthStencil = true;
            params->AutoDepthStencilFormat = D3DFMT_D16;
            params->hDeviceWindow = 0;
            params->Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
            params->FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
            params->PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

            hr = direct->CreateDevice(output, D3DDEVTYPE_HAL, 0, D3DCREATE_HARDWARE_VERTEXPROCESSING, params.get(), &device);
            if (FAILED(hr)) L(debug, "[DesktopDuplication::CreateDevice9] failed to create device: {:#x}", static_cast<unsigned int>(hr));
            return hr;
        }

        HRESULT OutputByDeviceName9(CComPtr<IDirect3D9> direct, std::wstring_view deviceName, UINT& output) {
            L(trace, L"[DesktopDuplication::OutputByDeviceName9] called with device name: {}", deviceName);
            output = 0;

            HRESULT hr = S_OK;
            if (std::empty(deviceName)) {
                L(debug, "[DesktopDuplication::OutputByDeviceName9] empty name display = first adapter");
                return hr;
            }

            if (!direct) {
                L(debug, L"[DesktopDuplication::OutputByDeviceName9] failed, no direct");
                return E_NOT_VALID_STATE;
            }

            auto count = direct->GetAdapterCount();
            if (count == 0) {
                L(debug, "[DesktopDuplication::OutputByDeviceName9] failed, no one adapter found");
                return E_FAIL;
            }

            for (UINT i = 0; i < count; i++) {
                L(trace, "[DesktopDuplication::OutputByDeviceName9] check adapter: {}", i);

                HMONITOR adapterMonitor = direct->GetAdapterMonitor(i);
                if (!adapterMonitor) {
                    L(debug, "[DesktopDuplication::OutputByDeviceName9] not found monitor for adapter [{}], skip", i);
                    continue;
                }

                MONITORINFOEX monitorInfo;
                monitorInfo.cbSize = sizeof(monitorInfo);

                if (::GetMonitorInfo(adapterMonitor, &monitorInfo)) {
                    if (monitorInfo.szDevice == deviceName) {
                        L(debug, "[DesktopDuplication::OutputByDeviceName9] found output: {}", i);
                        output = i;
                        return S_OK;
                    }
                }
                else {
                    L(debug, "[DesktopDuplication::OutputByDeviceName9] unable to get monitor info for adapter: {}", i);
                    continue;
                }
            }

            L(debug, "[DesktopDuplication::OutputByDeviceName9] failed, output not found");
            return E_FAIL;
        }
    }

    D3D9Desktop::D3D9Desktop() :
        available(false),
        direct(nullptr),
        params(nullptr),
        output(0)
    {}

    DesktopInfo D3D9Desktop::GetDesktopInfo() const {
        return this->desktopInfo;
    }

    bool D3D9Desktop::IsAvailable() const {
        L(trace, "[D3D9Desktop::IsAvailable] called, return: {}", this->available);
        return this->available;
    }

    bool D3D9Desktop::Duplicate(void* output) {
        L(trace, "[D3D9Desktop::Duplicate] called");

        HRESULT hr = S_OK;

        if (!this->device) {
            L(debug, "[D3D9Desktop::Duplicate] has no device, reinitialize");

            hr = this->initializeDevice();
            if (FAILED(hr)) return false;
        }

        hr = this->duplicate(output);
        if (FAILED(hr)) {
            this->uninitialize();
            return false;
        }

        return true;
    }

    bool D3D9Desktop::SetDisplay(std::wstring_view displayName) {
        L(trace, L"[D3D9Desktop::SetDisplay] called with display name: {}", displayName);

        this->available = false;

        auto hr = this->initializeOutput(displayName);
        if (SUCCEEDED(hr)) hr = this->initializeDevice();

        if (SUCCEEDED(hr)) {
            L(debug, "[D3D9Desktop::SetDisplay] success");
            this->available = true;
        }
        else {
            L(debug, "[D3D9Desktop::SetDisplay] failed, mark unavailable");
            this->uninitialize();
        }
        return this->available;
    }

    HRESULT D3D9Desktop::duplicate(void* output) {
        L(trace, "[D3D9Desktop::duplicate] called");

        HRESULT hr = S_OK;
        if (!this->device) {
            L(debug, "[D3D9Desktop::duplicate] duplicate called without device");
            return E_NOT_VALID_STATE;
        }

        CComPtr<IDirect3DSurface9> surface;
        hr = this->device->CreateOffscreenPlainSurface(this->params->BackBufferWidth, this->params->BackBufferHeight, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface, nullptr);
        if (FAILED(hr)) {
            L(debug, "[D3D9Desktop::duplicate] failed to create surface: {:#x}", static_cast<unsigned int>(hr));
            return hr;
        }

        hr = this->device->GetFrontBufferData(0, surface);
        if (FAILED(hr)) {
            L(debug, "[D3D9Desktop::duplicate] failed to get front buffer data: {:#x}", static_cast<unsigned int>(hr));
            return hr;
        }

        D3DLOCKED_RECT lockedRect;
        hr = surface->LockRect(&lockedRect, nullptr, D3DLOCK_READONLY);
        if (FAILED(hr)) {
            L(debug, "[D3D9Desktop::duplicate] failed to lock rect: {:#x}", static_cast<unsigned int>(hr));
            return hr;
        }

        std::size_t capacity =
            static_cast<std::size_t>(this->params->BackBufferWidth) *
            static_cast<std::size_t>(this->params->BackBufferHeight) *
            4;
        std::memcpy(output, lockedRect.pBits, capacity);

        hr = surface->UnlockRect();
        if (FAILED(hr)) {
            L(debug, "[D3D9Desktop::duplicate] failed to unlock rect: {:#x}", static_cast<unsigned int>(hr));
            return hr;
        }

        L(debug, "[D3D9Desktop::duplicate] success");
        return hr;
    }

    HRESULT D3D9Desktop::initializeDevice() {
        L(trace, "[D3D9Desktop::initializeDevice] called");

        this->initilizeDirect();
        this->params = std::make_shared<D3DPRESENT_PARAMETERS>();

        this->device = nullptr;
        HRESULT hr = DesktopDuplication::CreateDevice9(
            this->direct,
            this->output,
            this->device,
            this->params
        );
        if (FAILED(hr)) return hr;

        return this->updateDesktopInfo();
    }

    void D3D9Desktop::initilizeDirect() {
        L(trace, "[D3D9Desktop::initilizeDirect] called");

        if (this->direct) return;
        this->direct = Direct3DCreate9(D3D_SDK_VERSION);
    }

    HRESULT D3D9Desktop::initializeOutput(std::wstring_view displayName) {
        L(trace, "[D3D9Desktop::initializeOutput] called");

        this->initilizeDirect();
        this->output = 0;
        this->desktopInfo = DesktopInfo(displayName);
        return DesktopDuplication::OutputByDeviceName9(
            this->direct,
            displayName,
            this->output
        );
    }

    void D3D9Desktop::uninitialize() {
        L(trace, "[D3D9Desktop::uninitialize] called");

        this->device = nullptr;
        this->direct = nullptr;
        this->params = nullptr;
    }

    HRESULT D3D9Desktop::updateDesktopInfo() {
        L(trace, "[D3D9Desktop::updateDesktopInfo] called");

        if (!this->direct) {
            L(debug, "[D3D9Desktop::updateDesktopInfo] failed, direct empty");
            return E_FAIL;
        }

        D3DDISPLAYMODE mode;
        HRESULT hr = this->direct->GetAdapterDisplayMode(this->output, &mode);
        if (FAILED(hr)) {
            L(debug, "[D3D9Desktop::updateDesktopInfo] failed to get adapter [{}] display mode: {:#x}", this->output, static_cast<unsigned int>(hr));
            return hr;
        }

        HMONITOR m = this->direct->GetAdapterMonitor(this->output);
        if (!m) {
            L(debug, "[D3D9Desktop::updateDesktopInfo] failed to get adapter [{}] monitor", this->output);
            return E_FAIL;
        }

        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);

        if (!::GetMonitorInfo(m, &mi)) {
            L(debug, "[D3D9Desktop::updateDesktopInfo] failed to get monitor info");
            return E_FAIL;
        }

        auto x = mi.rcMonitor.left;
        auto y = mi.rcMonitor.top;

        auto width = this->params->BackBufferWidth;
        auto height = this->params->BackBufferHeight;

        auto scaledWidth = mi.rcMonitor.right - x;
        auto scaledHeight = mi.rcMonitor.bottom - y;

        this->desktopInfo.Rect = Rect::Rect(x, y, width, height);

        this->desktopInfo.Scale = Point(
            (width * 100) / (scaledWidth > 0 ? scaledWidth : width),
            (height * 100) / (scaledHeight > 0 ? scaledHeight : height)
        );

        L(debug, L"[D3D9Desktop::updateDesktopInfo] success: rect({}), scale({})", this->desktopInfo.Rect, this->desktopInfo.Scale);

        return S_OK;
    }
}
