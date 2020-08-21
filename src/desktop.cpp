#include "desktop.hpp"

namespace YellowRectangleCyanCircle {
    namespace DesktopDuplication {
        std::uint32_t CalcDesktopWidth(const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc) {
            if (!desc) return 0;

            if (desc->Rotation == DXGI_MODE_ROTATION_ROTATE90 || desc->Rotation == DXGI_MODE_ROTATION_ROTATE270)
                return desc->ModeDesc.Height;
            else
                return desc->ModeDesc.Width;
        }
        std::uint32_t CalcDesktopHeight(const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc) {
            if (!desc) return 0;

            if (desc->Rotation == DXGI_MODE_ROTATION_ROTATE90
                || desc->Rotation == DXGI_MODE_ROTATION_ROTATE270)
                return desc->ModeDesc.Width;
            else
                return desc->ModeDesc.Height;
        }

        HRESULT CreateDevice(
            const std::shared_ptr<IDirect>& direct,
            const std::shared_ptr<DirectOutput>& output,
            std::shared_ptr<Direct11Device>& device,
            std::shared_ptr<Direct11DeviceContext>& context
        ) {
            L(trace, "[DesktopDuplication::CreateDevice] called");
            HRESULT hr = S_OK;

            std::shared_ptr<DirectAdapter> adapter;
            if (output) {
                L(trace, "[DesktopDuplication::CreateDevice] output supplyed, ask for adapter");
                output->GetParent(adapter);
            }

            std::vector<D3D_DRIVER_TYPE> driverTypes;
            if (adapter) {
                L(debug, "[DesktopDuplication::CreateDevice] adapter found, use unknown driver type");
                driverTypes.push_back(D3D_DRIVER_TYPE_UNKNOWN);
            }
            else {
                L(debug, "[DesktopDuplication::CreateDevice] adapter not found, specify drive types");

                std::copy(
                    std::begin(DriverTypes),
                    std::end(DriverTypes),
                    std::back_inserter(driverTypes)
                );
            }

            D3D_FEATURE_LEVEL featureLevel;
            for (auto driverType : driverTypes) {
                L(trace, "[DesktopDuplication::CreateDevice] trying create device on driver type: {}", driverType);

                hr = direct->D3D11CreateDevice(
                    adapter,
                    driverType,
                    D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                    FeatureLevels,
                    ARRAYSIZE(FeatureLevels),
                    device,
                    &featureLevel,
                    context
                );
                if (SUCCEEDED(hr)) {
                    L(debug, "[DesktopDuplication::CreateDevice] success create device with driver type: {}", driverType);
                    break;
                }
            }
            if (FAILED(hr)) {
                L(debug, "[DesktopDuplication::CreateDevice] unable to create device: {}", hr);
                return hr;
            }
            if (!device) {
                L(debug, "[DesktopDuplication::CreateDevice] no one device type fit to create device");
                return E_FAIL;
            }

            context->SetMultithreadProtected();
            return hr;
        }

        HRESULT InitializeOnDevice(
            const std::shared_ptr<Direct11Device>& device,
            std::shared_ptr<DirectOutput> output,
            std::shared_ptr<DirectOutputDuplication>& deskDupl,
            std::shared_ptr<DXGI_OUTDUPL_DESC>& deskDuplDesc
        ) {
            L(trace, "[DesktopDuplication::InitializeOnDevice] called");
            HRESULT hr = S_OK;

            deskDuplDesc = std::make_shared<DXGI_OUTDUPL_DESC>();
            RtlZeroMemory(deskDuplDesc.get(), sizeof(DXGI_OUTDUPL_DESC));

            std::shared_ptr<DirectDevice> d;
            if (!output) {
                L(trace, "[DesktopDuplication::InitializeOnDevice] no output, query one");

                hr = device->QueryInterface(d);
                if (FAILED(hr)) {
                    L(debug, "[DesktopDuplication::InitializeOnDevice] d3d11 device failed to query direct device: {}", hr);
                    return hr;
                }

                std::shared_ptr<DirectAdapter> adapter;
                hr = d->GetParent(adapter);
                if (FAILED(hr)) {
                    L(debug, "[DesktopDuplication::InitializeOnDevice] direct device failed to get parent adapter: {}", hr);
                    return hr;
                }

                hr = adapter->EnumOutputs(0, output);
                if (FAILED(hr)) {
                    L(debug, "[DesktopDuplication::InitializeOnDevice] adapter failed to get output: {}", hr);
                    return hr;
                }
            }

            std::shared_ptr<DirectOutput1> output1;
            hr = output->QueryInterface(output1);
            if (FAILED(hr)) {
                L(debug, "[DesktopDuplication::InitializeOnDevice] output failed to query output v1: {}", hr);
                return hr;
            }

            hr = output1->DuplicateOutput(device, deskDupl);
            if (FAILED(hr)) {
                L(debug, "[DesktopDuplication::InitializeOnDevice] output v1 failed to duplicate output: {}", hr);
                return hr;
            }

            deskDuplDesc = deskDupl->GetDesc();

            L(debug, "[DesktopDuplication::InitializeOnDevice] success");
            return hr;
        }

        HRESULT FormatFrameDescription(
            const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc,
            std::shared_ptr<D3D11_TEXTURE2D_DESC>& frameDesc,
            std::shared_ptr<D3D11_TEXTURE2D_DESC>& cpuframeDesc
        ) {
            L(trace, "[DesktopDuplication::FormatFrameDescription] called");

            if (!desc) {
                L(debug, "[DesktopDuplication::FormatFrameDescription] duplication description not found");
                return E_FAIL;
            }

            auto fd = std::make_shared<D3D11_TEXTURE2D_DESC>();
            RtlZeroMemory(fd.get(), sizeof(D3D11_TEXTURE2D_DESC));
            fd->Width = CalcDesktopWidth(desc);
            fd->Height = CalcDesktopHeight(desc);
            fd->Format = desc->ModeDesc.Format;
            fd->ArraySize = 1;
            fd->BindFlags = D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
            fd->MiscFlags = 0;
            fd->SampleDesc.Count = 1;
            fd->SampleDesc.Quality = 0;
            fd->MipLevels = 1;
            fd->CPUAccessFlags = 0;
            fd->Usage = D3D11_USAGE_DEFAULT;

            frameDesc = fd;

            auto cfd = std::make_shared<D3D11_TEXTURE2D_DESC>(*fd);
            cfd->BindFlags = 0;
            cfd->CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            cfd->Usage = D3D11_USAGE_STAGING;

            cpuframeDesc = cfd;

            if (fd->Width == 0 || fd->Height == 0) {
                L(debug, "[DesktopDuplication::FormatFrameDescription] failed, width or height equals zero");
                return E_FAIL;
            }

            L(debug, "[DesktopDuplication::FormatFrameDescription] success");
            return S_OK;
        }

        std::vector<std::shared_ptr<DirectAdapter>> EnumAdapters(const std::shared_ptr<IDirect>& direct) {
            L(trace, "[DesktopDuplication::EnumAdapters] called");

            std::vector<std::shared_ptr<DirectAdapter>> adapters;
            std::shared_ptr<DirectFactory1> factory;
            HRESULT hr = direct->CreateDXGIFactory1(factory);
            if (!factory) {
                L(debug, "[DesktopDuplication::EnumAdapters] unable to create DXGI factory: {}", hr);
                return adapters;
            }

            UINT i = 0;
            std::shared_ptr<DirectAdapter> adapter;
            while (i < 100 && factory->EnumAdapters(i++, adapter) != DXGI_ERROR_NOT_FOUND)
                adapters.push_back(adapter);

            L(debug, "[DesktopDuplication::EnumAdapters] adapters count: {}", std::size(adapters));
            return adapters;
        }

        HRESULT OutputByDeviceName(
            const std::shared_ptr<IDirect>& direct,
            std::wstring_view deviceName,
            std::shared_ptr<DirectOutput>& output
        ) {
            L(trace, L"[DesktopDuplication::OutputByDeviceName] device name: {}", deviceName);
            output = nullptr;

            HRESULT hr = S_OK;
            if (std::empty(deviceName)) {
                L(debug, "[DesktopDuplication::OutputByDeviceName] empty name display = empty output");
                return hr;
            }

            auto adapters = DesktopDuplication::EnumAdapters(direct);
            for (auto& adapter : adapters) {
                std::shared_ptr<DirectOutput> o;

                UINT i = 0;
                while (i < 100 && adapter->EnumOutputs(i++, o) != DXGI_ERROR_NOT_FOUND) {
                    L(trace, "[DesktopDuplication::OutputByDeviceName] adapter output: {}", i);

                    DXGI_OUTPUT_DESC desc;
                    hr = o->GetDesc(desc);
                    if (FAILED(hr)) {
                        L(debug, "[DesktopDuplication::OutputByDeviceName] unable to get output description: {}", hr);
                        return hr;
                    }

                    L(trace, L"[DesktopDuplication::OutputByDeviceName] adapter output name: {}", desc.DeviceName);

                    if (desc.DeviceName == deviceName) {
                        L(debug, L"[DesktopDuplication::OutputByDeviceName] found output: {}", desc.DeviceName);
                        output = o;
                        return hr;
                    }
                }
            }

            L(debug, L"[DesktopDuplication::OutputByDeviceName] output not found: {}", deviceName);
            return hr;
        }
    }

    Desktop::Desktop(std::shared_ptr<IDirect> direct) :
        Desktop(direct, L"")
    {}

    Desktop::Desktop(std::shared_ptr<IDirect> direct, std::wstring_view displayName) :
        direct(direct)
    {
        L(trace, L"[Desktop::Desktop] called with display name: {}", displayName);

        DesktopDuplication::OutputByDeviceName(direct, displayName, this->output);
        this->initialize();
    }

    std::uint32_t Desktop::GetWidth() const {
        return DesktopDuplication::CalcDesktopWidth(this->deskDuplDesc);
    }

    std::uint32_t Desktop::GetHeight() const {
        return DesktopDuplication::CalcDesktopHeight(this->deskDuplDesc);
    }

    void Desktop::Duplicate(void* output) {
        L(trace, "[Desktop::Duplicate] called");
        if (!this->device) this->initialize();

        this->duplicate(output);
    }

    HRESULT Desktop::SwitchDisplay(std::wstring_view displayName) {
        L(trace, L"[Desktop::SwitchDisplay] called with display name: {}", displayName);

        this->output = nullptr;
        DesktopDuplication::OutputByDeviceName(this->direct, displayName, this->output);

        this->deskDupl = nullptr;
        this->deskDuplDesc = nullptr;
        HRESULT hr = DesktopDuplication::InitializeOnDevice(this->device, this->output, this->deskDupl, this->deskDuplDesc);
        if (SUCCEEDED(hr)) {
            this->frameDesc = nullptr;
            this->cpuFrameDesc = nullptr;

            hr = DesktopDuplication::FormatFrameDescription(this->deskDuplDesc, this->frameDesc, this->cpuFrameDesc);
        }
        if (FAILED(hr)) this->uninitialize();
        return hr;
    }

    HRESULT Desktop::initialize() {
        L(trace, "[Desktop::initialize] called");

        HRESULT hr = DesktopDuplication::CreateDevice(this->direct, this->output, this->device, this->context);
        if (SUCCEEDED(hr)) {
            hr = DesktopDuplication::InitializeOnDevice(this->device, this->output, this->deskDupl, this->deskDuplDesc);
            if (SUCCEEDED(hr)) {
                hr = DesktopDuplication::FormatFrameDescription(this->deskDuplDesc, this->frameDesc, this->cpuFrameDesc);
            }
        }

        if (FAILED(hr)) this->uninitialize();
        return hr;
    }

    HRESULT Desktop::duplicate(void* output) {
        L(trace, "[Desktop::duplicate] called");

        HRESULT hr = S_OK;
        std::shared_ptr<DXGI_OUTDUPL_FRAME_INFO> frameInfo;
        std::shared_ptr<DirectResource> resource;

        if (!this->device) {
            L(debug, "[Desktop::duplicate] duplicate called without device");
            return E_NOT_VALID_STATE;
        }

        if (this->deskDupl) {
            L(trace, "[Desktop::duplicate] desktop duplication found, trying to acquire next frame");

            for (std::uint8_t i = 0; i < 4; i++) {
                this->deskDupl->ReleaseFrame();

                hr = this->deskDupl->AcquireNextFrame(0, frameInfo, resource);
                if (SUCCEEDED(hr)) {
                    L(trace, "[Desktop::duplicate] next frame acquired successfully");
                    break;
                }

                // TODO: not so good way
                if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
                    L(trace, "[Desktop::duplicate] acquiring timeout, waiting some time");
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(100ns);
                    continue;
                }
                else if (FAILED(hr)) {
                    L(trace, "[Desktop::duplicate] acquiring failed: {}", hr);
                    break;
                }

                if (frameInfo->LastPresentTime.QuadPart) break;
            }
        }

        if (!this->deskDupl
            || hr == DXGI_ERROR_ACCESS_LOST
            || hr == DXGI_ERROR_INVALID_CALL) {
            if (this->deskDupl) {
                L(debug, "[Desktop::duplicate] recreating desktop duplication because it lost access or has invalid call: {}", hr);
                this->deskDupl->ReleaseFrame();
                this->deskDupl = nullptr;
            }
            else L(debug, "[Desktop::duplicate] desktop duplication not exists, recreating");

            hr = DesktopDuplication::InitializeOnDevice(this->device, this->output, this->deskDupl, this->deskDuplDesc);
            if (FAILED(hr)) {
                L(debug, "[Desktop::duplicate] desktop duplication initialization failed, ask to recreate device next time");
                this->uninitialize();
            }
            return hr;
        }

        if (hr == DXGI_ERROR_DEVICE_REMOVED) {
            hr = this->device->GetDeviceRemovedReason();
            L(debug, "[Desktop::duplicate] device was removed (reason: {}), ask to recreate device next time", hr);

            this->uninitialize();
            return hr;
        }

        if (!resource) {
            L(debug, "[Desktop::duplicate] AcquireNextFrame unexpectedly return null resource, try again next time");
            return hr;
        }

        std::shared_ptr<DirectTexture2D> frameCopy = nullptr;
        hr = this->device->CreateTexture2D(this->frameDesc, frameCopy);
        if (FAILED(hr)) {
            L(debug, "[Desktop::duplicate] failed to create duplication texture");
            return hr;
        }

        std::shared_ptr<DirectTexture2D> frameDest = nullptr;
        hr = this->device->CreateTexture2D(this->cpuFrameDesc, frameDest);
        if (FAILED(hr)) {
            L(debug, "[Desktop::duplicate] failed to create destination texture");
            return hr;
        }

        std::shared_ptr<DirectTexture2D> desktopImage = nullptr;
        hr = resource->QueryInterface(desktopImage);
        if (FAILED(hr)) {
            L(debug, "[Desktop::duplicate] failed to query desktop image");
            return hr;
        }

        this->context->CopyTexture(frameCopy, desktopImage);
        this->context->CopyTexture(frameDest, frameCopy);

        std::shared_ptr<D3D11_MAPPED_SUBRESOURCE> ms;
        UINT subresource = D3D11CalcSubresource(0, 0, 0);
        hr = this->context->Map(frameDest, subresource, D3D11_MAP_READ_WRITE, 0, ms);
        if (FAILED(hr)) {
            L(debug, "[Desktop::duplicate] failed to map destination texture");
            this->context->Unmap(frameDest, subresource);
            return hr;
        }

        // Width * Height * (B, G, R, A)
        std::size_t capacity =
            static_cast<std::size_t>(this->GetWidth()) *
            static_cast<std::size_t>(this->GetHeight()) *
            4;
        std::memcpy(
            output,
            ms->pData,
            capacity
        );

        this->context->Unmap(frameDest, subresource);
        this->deskDupl->ReleaseFrame();

        L(debug, "[Desktop::duplicate] success");
        return hr;
    }

    void Desktop::uninitialize() {
        L(trace, "[Desktop::uninitialize] called");

        this->deskDupl = nullptr;
        this->deskDuplDesc = nullptr;

        this->device = nullptr;
        this->context = nullptr;
    }
}
