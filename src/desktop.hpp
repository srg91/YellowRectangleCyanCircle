#pragma once

#include "desktop.d3d9.hpp"
#include "desktop.d3d11.hpp"
#include "desktop.gdi.hpp"
#include "direct.hpp"
#include "interface.hpp"
#include "logging.hpp"

#include <memory>
#include <vector>

namespace YellowRectangleCyanCircle {
    class Desktop {
    public:
        Desktop(std::shared_ptr<IDirect> direct);
        Desktop(std::shared_ptr<IDirect> direct, std::wstring_view displayName);

        DesktopInfo GetInfo() const;

        bool Duplicate(void* output);
        void SwitchDisplay(std::wstring_view displayName);
    private:
        std::vector<std::shared_ptr<IDesktopDuplicator>> duplicators;

        HRESULT duplicate(void* output);
    };
}
