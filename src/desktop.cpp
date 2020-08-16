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
            HRESULT hr = S_OK;

            std::shared_ptr<DirectAdapter> adapter;
            if (output) output->GetParent(adapter);

            std::vector<D3D_DRIVER_TYPE> driverTypes;
            if (adapter)
                driverTypes.push_back(D3D_DRIVER_TYPE_UNKNOWN);
            else
                std::copy(
                    std::begin(DriverTypes),
                    std::end(DriverTypes),
                    std::back_inserter(driverTypes)
                );

            D3D_FEATURE_LEVEL featureLevel;
            for (auto driverType : driverTypes) {
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
                if (SUCCEEDED(hr)) break;
            }
            if (FAILED(hr)) return hr;
            if (!device) return E_FAIL;

            context->SetMultithreadProtected();
            return hr;
        }

        HRESULT InitializeOnDevice(
            const std::shared_ptr<Direct11Device>& device,
            std::shared_ptr<DirectOutput> output,
            std::shared_ptr<DirectOutputDuplication>& deskDupl,
            std::shared_ptr<DXGI_OUTDUPL_DESC>& deskDuplDesc
        ) {
            HRESULT hr = S_OK;

            deskDuplDesc = std::make_shared<DXGI_OUTDUPL_DESC>();
            RtlZeroMemory(deskDuplDesc.get(), sizeof(DXGI_OUTDUPL_DESC));

            std::shared_ptr<DirectDevice> d;
            if (!output) {
                hr = device->QueryInterface(d);
                if (FAILED(hr)) return hr;

                std::shared_ptr<DirectAdapter> adapter;
                hr = d->GetParent(adapter);
                if (FAILED(hr)) return hr;

                hr = adapter->EnumOutputs(0, output);
                if (FAILED(hr)) return hr;
            }

            std::shared_ptr<DirectOutput1> output1;
            hr = output->QueryInterface(output1);
            if (FAILED(hr)) return hr;

            hr = output1->DuplicateOutput(device, deskDupl);
            if (FAILED(hr)) return hr;

            deskDuplDesc = deskDupl->GetDesc();

            return hr;
        }

        void FormatFrameDescription(
            const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc,
            std::shared_ptr<D3D11_TEXTURE2D_DESC>& frameDesc,
            std::shared_ptr<D3D11_TEXTURE2D_DESC>& cpuframeDesc
        ) {
            if (!desc) return;

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
        }

        std::vector<std::shared_ptr<DirectAdapter>> EnumAdapters(const std::shared_ptr<IDirect>& direct) {
            std::vector<std::shared_ptr<DirectAdapter>> adapters;
            std::shared_ptr<DirectFactory1> factory;
            HRESULT hr = direct->CreateDXGIFactory1(factory);
            if (!factory) return adapters;

            UINT i = 0;
            std::shared_ptr<DirectAdapter> adapter;
            while (i < 100 && factory->EnumAdapters(i++, adapter) != DXGI_ERROR_NOT_FOUND)
                adapters.push_back(adapter);
            return adapters;
        }

        HRESULT OutputByDeviceName(
            const std::shared_ptr<IDirect>& direct,
            std::wstring_view deviceName,
            std::shared_ptr<DirectOutput>& output
        ) {
            HRESULT hr = S_OK;
            if (std::empty(deviceName)) return hr;

            auto adapters = DesktopDuplication::EnumAdapters(direct);
            for (auto& adapter : adapters) {
                std::shared_ptr<DirectOutput> o;

                UINT i = 0;
                while (i < 100 && adapter->EnumOutputs(i++, o) != DXGI_ERROR_NOT_FOUND) {
                    DXGI_OUTPUT_DESC desc;
                    hr = o->GetDesc(desc);
                    if (FAILED(hr)) return hr;

                    if (desc.DeviceName == deviceName) {
                        output = o;
                        break;
                    }
                }
            }
            return hr;
        }
    }

    Desktop::Desktop(std::shared_ptr<IDirect> direct) :
        Desktop(direct, L"")
    {}

    Desktop::Desktop(std::shared_ptr<IDirect> direct, std::wstring_view displayName) :
        direct(direct)
    {
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
        if (!this->device) this->initialize();

        this->duplicate(output);
    }

    HRESULT Desktop::SwitchDisplay(std::wstring_view displayName) {
        this->output = nullptr;
        DesktopDuplication::OutputByDeviceName(this->direct, displayName, this->output);

        this->deskDupl = nullptr;
        this->deskDuplDesc = nullptr;
        HRESULT hr = DesktopDuplication::InitializeOnDevice(this->device, this->output, this->deskDupl, this->deskDuplDesc);
        if (FAILED(hr)) return hr;

        this->frameDesc = nullptr;
        this->cpuFrameDesc = nullptr;
        DesktopDuplication::FormatFrameDescription(this->deskDuplDesc, this->frameDesc, this->cpuFrameDesc);
        return hr;
    }

    HRESULT Desktop::initialize() {
        HRESULT hr = DesktopDuplication::CreateDevice(this->direct, this->output, this->device, this->context);
        if (FAILED(hr)) return hr;

        hr = DesktopDuplication::InitializeOnDevice(this->device, this->output, this->deskDupl, this->deskDuplDesc);
        if (FAILED(hr)) return hr;

        DesktopDuplication::FormatFrameDescription(this->deskDuplDesc, this->frameDesc, this->cpuFrameDesc);
        return hr;
    }

    HRESULT Desktop::duplicate(void* output) {
        HRESULT hr = S_OK;
        std::shared_ptr<DXGI_OUTDUPL_FRAME_INFO> frameInfo;
        std::shared_ptr<DirectResource> resource;

        if (!this->device) return E_NOT_VALID_STATE;

        if (this->deskDupl) {
            for (std::uint8_t i = 0; i < 4; i++) {
                this->deskDupl->ReleaseFrame();

                hr = this->deskDupl->AcquireNextFrame(0, frameInfo, resource);
                if (SUCCEEDED(hr)) break;

                // TODO: not so good way
                if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(100ns);
                    continue;
                }
                else if (FAILED(hr)) break;

                if (frameInfo->LastPresentTime.QuadPart) break;
            }
        }

        if (!this->deskDupl
            || hr == DXGI_ERROR_ACCESS_LOST
            || hr == DXGI_ERROR_INVALID_CALL) {
            if (this->deskDupl) {
                this->deskDupl->ReleaseFrame();
                this->deskDupl = nullptr;
            }

            hr = DesktopDuplication::InitializeOnDevice(this->device, this->output, this->deskDupl, this->deskDuplDesc);
            // Skip this shoot, just return zero frame
            return hr;
        }

        if (hr == DXGI_ERROR_DEVICE_REMOVED) {
            hr = this->device->GetDeviceRemovedReason();
            this->uninitialize();
            return hr;
        }

        if (!resource) return hr;

        std::shared_ptr<DirectTexture2D> frameCopy = nullptr;
        hr = this->device->CreateTexture2D(this->frameDesc, frameCopy);
        if (FAILED(hr)) return hr;

        std::shared_ptr<DirectTexture2D> frameDest = nullptr;
        hr = this->device->CreateTexture2D(this->cpuFrameDesc, frameDest);
        if (FAILED(hr)) return hr;

        std::shared_ptr<DirectTexture2D> desktopImage = nullptr;
        hr = resource->QueryInterface(desktopImage);
        if (FAILED(hr)) return hr;

        this->context->CopyTexture(frameCopy, desktopImage);
        this->context->CopyTexture(frameDest, frameCopy);

        std::shared_ptr<D3D11_MAPPED_SUBRESOURCE> ms;
        UINT subresource = D3D11CalcSubresource(0, 0, 0);
        this->context->Map(frameDest, subresource, D3D11_MAP_READ_WRITE, 0, ms);

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

        return hr;
    }

    void Desktop::uninitialize() {
        this->deskDupl = nullptr;
        this->deskDuplDesc = nullptr;

        this->device = nullptr;
        this->context = nullptr;
    }
}
