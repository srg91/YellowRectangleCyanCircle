#pragma once

#include "direct.hpp"
#include "interface.hpp"
#include "logging.hpp"
#include "winapi.hpp"
#include "types.hpp"

#include <chrono>
#include <thread>
#include <vector>

namespace YellowRectangleCyanCircle {
    namespace DesktopDuplication {
        const D3D_DRIVER_TYPE DriverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };

        const D3D_FEATURE_LEVEL FeatureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_1
        };

        std::uint32_t CalcDesktopWidth(const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc);
        std::uint32_t CalcDesktopHeight(const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc);
        void CopyWithRotate(void* input, void* output, int width, int height, DXGI_MODE_ROTATION rotation);

        HRESULT CreateDevice(
            const std::shared_ptr<IDirect>& direct,
            const std::shared_ptr<DirectOutput>& output,
            std::shared_ptr<Direct11Device>& device,
            std::shared_ptr<Direct11DeviceContext>& context
        );

        HRESULT InitializeOnDevice(
            const std::shared_ptr<Direct11Device>& device,
            std::shared_ptr<DirectOutput> output,
            std::shared_ptr<DirectOutputDuplication>& deskDupl,
            std::shared_ptr<DXGI_OUTDUPL_DESC>& deskDuplDesc
        );

        HRESULT FormatFrameDescription(
            const std::shared_ptr<DXGI_OUTDUPL_DESC>& desc,
            std::shared_ptr<D3D11_TEXTURE2D_DESC>& frameDesc,
            std::shared_ptr<D3D11_TEXTURE2D_DESC>& cpuframeDesc
        );

        std::vector<std::shared_ptr<DirectAdapter>> EnumAdapters(
            const std::shared_ptr<IDirect>& direct
        );

        HRESULT OutputByDeviceName11(
            const std::shared_ptr<IDirect>& direct,
            std::wstring_view deviceName,
            std::shared_ptr<DirectOutput>& output
        );
    }

    class D3D11Desktop : public IDesktopDuplicator {
    public:
        D3D11Desktop(std::shared_ptr<IDirect> direct);

        DesktopInfo GetDesktopInfo() const override final;
        bool IsAvailable() const override final;

        bool Duplicate(void* output) override final;
        bool SetDisplay(std::wstring_view displayName) override final;
    private:
        bool available;
        DesktopInfo desktopInfo;

        std::shared_ptr<IDirect> direct;

        std::shared_ptr<Direct11Device> device;
        std::shared_ptr<Direct11DeviceContext> context;
        std::shared_ptr<DirectOutputDuplication> deskDupl;
        std::shared_ptr<DXGI_OUTDUPL_DESC> deskDuplDesc;
        std::shared_ptr<DirectOutput> output;

        std::shared_ptr<D3D11_TEXTURE2D_DESC> frameDesc;
        std::shared_ptr<D3D11_TEXTURE2D_DESC> cpuFrameDesc;

        HRESULT initializeDevice();
        HRESULT initializeDuplication();
        HRESULT initializeOutput(std::wstring_view displayName);
        HRESULT updateDesktopInfo();

        HRESULT duplicate(void* output);

        void uninitialize();
    };
}
