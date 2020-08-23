#include "desktop.d3d11.hpp"

namespace YellowRectangleCyanCircle {
    namespace DesktopDuplication {
        std::uint32_t CalcDesktopWidth(const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc) {
            L(trace, "[DesktopDuplication::CalcDesktopWidth] called");

            if (!desc) {
                L(debug, "[DesktopDuplication::CalcDesktopWidth] called with desc");
                return 0;
            }

            if (desc->Rotation == DXGI_MODE_ROTATION_ROTATE90 || desc->Rotation == DXGI_MODE_ROTATION_ROTATE270) {
                L(trace, "[DesktopDuplication::CalcDesktopWidth] rotated, return height");
                return desc->ModeDesc.Height;
            }
            else
                return desc->ModeDesc.Width;
        }

        std::uint32_t CalcDesktopHeight(const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc) {
            L(trace, "[DesktopDuplication::CalcDesktopHeight] called");

            if (!desc) {
                L(debug, "[DesktopDuplication::CalcDesktopHeight] called with desc");
                return 0;
            }

            if (desc->Rotation == DXGI_MODE_ROTATION_ROTATE90
                || desc->Rotation == DXGI_MODE_ROTATION_ROTATE270)
            {
                L(trace, "[DesktopDuplication::CalcDesktopHeight] rotated, return width");
                return desc->ModeDesc.Width;
            }
            else return desc->ModeDesc.Height;
        }

        void CopyWithRotate(void* input, void* output, int width, int height, DXGI_MODE_ROTATION rotation) {
            L(trace, "[DesktopDuplication::CopyWithRotate] called params: [{}, {}, {}]", width, height, rotation);

            Mat om(height, width, CV_8UC4, output);

            switch (rotation) {
            case DXGI_MODE_ROTATION_ROTATE90:
            {
                Mat im(width, height, CV_8UC4, input);
                cv::rotate(im, om, cv::ROTATE_90_CLOCKWISE);
            }
            break;
            case DXGI_MODE_ROTATION_ROTATE180:
            {
                Mat im(height, width, CV_8UC4, input);
                cv::rotate(im, om, cv::ROTATE_180);
            }
            break;
            case DXGI_MODE_ROTATION_ROTATE270:
            {
                Mat im(width, height, CV_8UC4, input);
                cv::rotate(im, om, cv::ROTATE_90_COUNTERCLOCKWISE);
            }
            break;
            default:
            {
                // Width * Height * (B, G, R, A)
                std::size_t capacity =
                    static_cast<std::size_t>(width) *
                    static_cast<std::size_t>(height) *
                    4;
                std::memcpy(
                    output,
                    input,
                    capacity
                );
            }
            break;
            }
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
                L(debug, "[DesktopDuplication::CreateDevice] unable to create device: {:#x}", static_cast<unsigned int>(hr));
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
                    L(debug, "[DesktopDuplication::InitializeOnDevice] d3d11 device failed to query direct device: {:#x}", static_cast<unsigned int>(hr));
                    return hr;
                }

                std::shared_ptr<DirectAdapter> adapter;
                hr = d->GetParent(adapter);
                if (FAILED(hr)) {
                    L(debug, "[DesktopDuplication::InitializeOnDevice] direct device failed to get parent adapter: {:#x}", static_cast<unsigned int>(hr));
                    return hr;
                }

                hr = adapter->EnumOutputs(0, output);
                if (FAILED(hr)) {
                    L(debug, "[DesktopDuplication::InitializeOnDevice] adapter failed to get output: {:#x}", static_cast<unsigned int>(hr));
                    return hr;
                }
            }

            std::shared_ptr<DirectOutput1> output1;
            hr = output->QueryInterface(output1);
            if (FAILED(hr)) {
                L(debug, "[DesktopDuplication::InitializeOnDevice] output failed to query output v1: {:#x}", static_cast<unsigned int>(hr));
                return hr;
            }

            hr = output1->DuplicateOutput(device, deskDupl);
            if (FAILED(hr)) {
                L(debug, "[DesktopDuplication::InitializeOnDevice] output v1 failed to duplicate output: {:#x}", static_cast<unsigned int>(hr));
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
                L(debug, "[DesktopDuplication::EnumAdapters] unable to create DXGI factory: {:#x}", static_cast<unsigned int>(hr));
                return adapters;
            }

            UINT i = 0;
            std::shared_ptr<DirectAdapter> adapter;
            while (i < 100 && factory->EnumAdapters(i++, adapter) != DXGI_ERROR_NOT_FOUND)
                adapters.push_back(adapter);

            L(debug, "[DesktopDuplication::EnumAdapters] adapters count: {}", std::size(adapters));
            return adapters;
        }

        HRESULT OutputByDeviceName11(
            const std::shared_ptr<IDirect>& direct,
            std::wstring_view deviceName,
            std::shared_ptr<DirectOutput>& output
        ) {
            L(trace, L"[DesktopDuplication::OutputByDeviceName11] called with device name: {}", deviceName);
            output = nullptr;

            HRESULT hr = S_OK;
            if (std::empty(deviceName)) {
                L(debug, "[DesktopDuplication::OutputByDeviceName11] empty name display = empty output");
                return hr;
            }

            auto adapters = EnumAdapters(direct);
            for (auto& adapter : adapters) {
                std::shared_ptr<DirectOutput> o;

                UINT i = 0;
                while (i < 100 && adapter->EnumOutputs(i++, o) != DXGI_ERROR_NOT_FOUND) {
                    L(trace, "[DesktopDuplication::OutputByDeviceName11] adapter output: {}", i);

                    DXGI_OUTPUT_DESC desc;
                    hr = o->GetDesc(desc);
                    if (FAILED(hr)) {
                        L(debug, "[DesktopDuplication::OutputByDeviceName11] unable to get output description: {:#x}", static_cast<unsigned int>(hr));
                        return hr;
                    }

                    L(trace, L"[DesktopDuplication::OutputByDeviceName11] adapter output name: {}", desc.DeviceName);

                    if (desc.DeviceName == deviceName) {
                        L(debug, L"[DesktopDuplication::OutputByDeviceName11] found output: {}", desc.DeviceName);
                        output = o;
                        return hr;
                    }
                }
            }

            L(debug, L"[DesktopDuplication::OutputByDeviceName11] output not found: {}", deviceName);
            return E_FAIL;
        }
    }

    D3D11Desktop::D3D11Desktop(std::shared_ptr<IDirect> direct) :
        available(false),
        direct(direct)
    {}

    DesktopInfo D3D11Desktop::GetDesktopInfo() const {
        return this->desktopInfo;
    }

    bool D3D11Desktop::IsAvailable() const {
        L(trace, "[D3D11Desktop::IsAvailable] called, return: {}", this->available);
        return this->available;
    }

    bool D3D11Desktop::Duplicate(void* output) {
        L(trace, "[D3D11Desktop::Duplicate] called");

        HRESULT hr = S_OK;

        if (!this->device) {
            L(debug, "[D3D11Desktop::Duplicate] has no device, reinitialize");

            hr = this->initializeDevice();
            if (SUCCEEDED(hr)) hr = this->initializeDuplication();

            if (FAILED(hr)) return false;
        }

        hr = this->duplicate(output);
        if (FAILED(hr)) return false;

        return true;
    }

    bool D3D11Desktop::SetDisplay(std::wstring_view displayName) {
        L(trace, L"[D3D11Desktop::SetDisplay] called with display name: {}", displayName);

        this->available = false;

        auto hr = this->initializeOutput(displayName);
        if (SUCCEEDED(hr)) {
            if (!this->device) hr = this->initializeDevice();
            if (this->device) hr = this->initializeDuplication();
        }

        if (SUCCEEDED(hr)) {
            L(debug, "[D3D11Desktop::SetDisplay] success");
            this->available = true;
        }
        else {
            L(debug, "[D3D11Desktop::SetDisplay] failed, mark unavailable");
            this->uninitialize();
        }
        return this->available;
    }

    HRESULT D3D11Desktop::initializeOutput(std::wstring_view displayName) {
        L(trace, "[D3D11Desktop::initializeOutput] called");
        this->output = nullptr;
        this->desktopInfo = DesktopInfo(displayName);
        return DesktopDuplication::OutputByDeviceName11(this->direct, displayName, this->output);
    }

    HRESULT D3D11Desktop::initializeDevice() {
        L(trace, "[D3D11Desktop::initializeDevice] called");

        this->device = nullptr;
        this->context = nullptr;
        return DesktopDuplication::CreateDevice(this->direct, this->output, this->device, this->context);
    }

    HRESULT D3D11Desktop::initializeDuplication() {
        L(trace, "[D3D11Desktop::initializeDuplication] called");

        this->deskDupl = nullptr;
        this->deskDuplDesc = nullptr;

        this->frameDesc = nullptr;
        this->cpuFrameDesc = nullptr;

        HRESULT hr = DesktopDuplication::InitializeOnDevice(this->device, this->output, this->deskDupl, this->deskDuplDesc);
        if (FAILED(hr)) return hr;

        hr = DesktopDuplication::FormatFrameDescription(this->deskDuplDesc, this->frameDesc, this->cpuFrameDesc);
        if (FAILED(hr)) return hr;

        return this->updateDesktopInfo();
    }

    HRESULT D3D11Desktop::updateDesktopInfo() {
        L(trace, "[D3D11Desktop::updateDesktopInfo] called");

        if (!this->output) {
            L(debug, "[D3D11Desktop::updateDesktopInfo] failed, output empty");
            return E_FAIL;
        }

        if (!this->deskDuplDesc) {
            L(debug, "[D3D11Desktop::updateDesktopInfo] failed, desktop duplication empty");
            return E_FAIL;
        }

        DXGI_OUTPUT_DESC d;
        HRESULT hr = this->output->GetDesc(d);
        if (FAILED(hr)) {
            L(debug, "[D3D11Desktop::updateDesktopInfo] get output desc failed with: {:#x}", static_cast<unsigned int>(hr));
            return hr;
        }

        auto width = this->deskDuplDesc->ModeDesc.Width;
        auto height = this->deskDuplDesc->ModeDesc.Height;

        auto x = d.DesktopCoordinates.left;
        auto y = d.DesktopCoordinates.top;

        this->desktopInfo.Rect = Rect::Rect(x, y, width, height);

        auto scaledWidth = d.DesktopCoordinates.right - x;
        auto scaledHeight = d.DesktopCoordinates.bottom - y;

        this->desktopInfo.Scale = Point(
            (width * 100) / (scaledWidth > 0 ? scaledWidth : width),
            (height * 100) / (scaledHeight > 0 ? scaledHeight : height)
        );

        L(debug, L"[D3D11Desktop::updateDesktopInfo] success: rect({}), scale({})", this->desktopInfo.Rect, this->desktopInfo.Scale);

        return S_OK;
    }

    HRESULT D3D11Desktop::duplicate(void* output) {
        L(trace, "[D3D11Desktop::duplicate] called");

        HRESULT hr = S_OK;
        std::shared_ptr<DXGI_OUTDUPL_FRAME_INFO> frameInfo;
        std::shared_ptr<DirectResource> resource;

        if (!this->device) {
            L(debug, "[D3D11Desktop::duplicate] duplicate called without device");
            return E_NOT_VALID_STATE;
        }

        if (this->deskDupl) {
            L(trace, "[D3D11Desktop::duplicate] desktop duplication found, trying to acquire next frame");

            for (std::uint8_t i = 0; i < 4; i++) {
                this->deskDupl->ReleaseFrame();

                hr = this->deskDupl->AcquireNextFrame(0, frameInfo, resource);
                if (SUCCEEDED(hr)) {
                    L(trace, "[D3D11Desktop::duplicate] next frame acquired successfully");
                    break;
                }

                // TODO: not so good way
                if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
                    L(trace, "[D3D11Desktop::duplicate] acquiring timeout, waiting some time");
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(100ns);
                    continue;
                }
                else if (FAILED(hr)) {
                    L(trace, "[D3D11Desktop::duplicate] acquiring failed: {:#x}", static_cast<unsigned int>(hr));
                    break;
                }

                if (frameInfo->LastPresentTime.QuadPart) break;
            }
        }

        if (!this->deskDupl
            || hr == DXGI_ERROR_ACCESS_LOST
            || hr == DXGI_ERROR_INVALID_CALL) {
            if (this->deskDupl) {
                L(debug, "[D3D11Desktop::duplicate] recreating desktop duplication because it lost access or has invalid call: {:#x}", static_cast<unsigned int>(hr));
                this->deskDupl->ReleaseFrame();
                this->deskDupl = nullptr;
            }
            else L(debug, "[D3D11Desktop::duplicate] desktop duplication not exists, recreating");

            hr = this->initializeDuplication();
            if (FAILED(hr)) {
                L(debug, "[D3D11Desktop::duplicate] desktop duplication initialization failed, ask to recreate device next time");
                this->uninitialize();
                return hr;
            }
            // Fail, cause we can try to use another duplicator
            return E_FAIL;
        }

        if (hr == DXGI_ERROR_DEVICE_REMOVED) {
            hr = this->device->GetDeviceRemovedReason();
            L(debug, "[D3D11Desktop::duplicate] device was removed (reason: {}), ask to recreate device next time", hr);

            this->uninitialize();
            return hr;
        }

        if (!resource) {
            L(debug, "[D3D11Desktop::duplicate] AcquireNextFrame unexpectedly return null resource, try again next time");
            return hr;
        }

        std::shared_ptr<DirectTexture2D> frameCopy = nullptr;
        hr = this->device->CreateTexture2D(this->frameDesc, frameCopy);
        if (FAILED(hr)) {
            L(debug, "[D3D11Desktop::duplicate] failed to create duplication texture");
            return hr;
        }

        std::shared_ptr<DirectTexture2D> frameDest = nullptr;
        hr = this->device->CreateTexture2D(this->cpuFrameDesc, frameDest);
        if (FAILED(hr)) {
            L(debug, "[D3D11Desktop::duplicate] failed to create destination texture");
            return hr;
        }

        std::shared_ptr<DirectTexture2D> desktopImage = nullptr;
        hr = resource->QueryInterface(desktopImage);
        if (FAILED(hr)) {
            L(debug, "[D3D11Desktop::duplicate] failed to query desktop image");
            return hr;
        }

        this->context->CopyTexture(frameCopy, desktopImage);
        this->context->CopyTexture(frameDest, frameCopy);

        std::shared_ptr<D3D11_MAPPED_SUBRESOURCE> ms;
        UINT subresource = D3D11CalcSubresource(0, 0, 0);
        hr = this->context->Map(frameDest, subresource, D3D11_MAP_READ_WRITE, 0, ms);
        if (FAILED(hr)) {
            L(debug, "[D3D11Desktop::duplicate] failed to map destination texture");
            this->context->Unmap(frameDest, subresource);
            return hr;
        }

        DesktopDuplication::CopyWithRotate(
            ms->pData,
            output,
            this->desktopInfo.Rect.width,
            this->desktopInfo.Rect.height,
            this->deskDuplDesc->Rotation
        );

        this->context->Unmap(frameDest, subresource);
        this->deskDupl->ReleaseFrame();

        L(debug, "[D3D11Desktop::duplicate] success");
        return hr;
    }

    void D3D11Desktop::uninitialize() {
        L(trace, "[D3D11Desktop::uninitialize] called");

        this->deskDupl = nullptr;
        this->deskDuplDesc = nullptr;

        this->device = nullptr;
        this->context = nullptr;
    }
}
