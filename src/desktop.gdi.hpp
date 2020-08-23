#pragma once

#include "interface.hpp"
#include "logging.hpp"

namespace YellowRectangleCyanCircle {
    class GDIDesktop : public IDesktopDuplicator {
    public:
        GDIDesktop();
        ~GDIDesktop();

        DesktopInfo GetDesktopInfo() const override final;
        bool IsAvailable() const override final;

        bool Duplicate(void* output) override final;
        bool SetDisplay(std::wstring_view displayName) override final;
    private:
        HDC dc;

        bool available;
        DesktopInfo desktopInfo;

        HRESULT duplicate(void* output);
        HRESULT initialize();
        void uninitialize();
    };
}
