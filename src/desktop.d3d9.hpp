#pragma once

#pragma comment (lib, "d3d9.lib")

#include "interface.hpp"
#include "logging.hpp"
#include "types.hpp"

#include <d3d9.h>

namespace YellowRectangleCyanCircle {
    namespace DesktopDuplication {
        HRESULT CreateDevice9(CComPtr<IDirect3D9> direct, UINT output, CComPtr<IDirect3DDevice9>& device, std::shared_ptr<D3DPRESENT_PARAMETERS>& params);
        HRESULT OutputByDeviceName9(CComPtr<IDirect3D9> direct, std::wstring_view deviceName, UINT& output);
    }

    class D3D9Desktop : public IDesktopDuplicator {
    public:
        D3D9Desktop();

        DesktopInfo GetDesktopInfo() const override final;
        bool IsAvailable() const override final;

        bool Duplicate(void* output) override final;
        bool SetDisplay(std::wstring_view displayName) override final;
    private:
        bool available;
        DesktopInfo desktopInfo;

        CComPtr<IDirect3D9> direct;
        CComPtr<IDirect3DDevice9> device;
        std::shared_ptr<D3DPRESENT_PARAMETERS> params;
        UINT output;

        HRESULT duplicate(void* output);
        HRESULT initializeDevice();
        void initilizeDirect();
        HRESULT initializeOutput(std::wstring_view displayName);
        void uninitialize();
        HRESULT updateDesktopInfo();
    };
}
