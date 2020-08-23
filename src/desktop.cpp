#include "desktop.hpp"

namespace YellowRectangleCyanCircle {
    Desktop::Desktop(std::shared_ptr<IDirect> direct) :
        Desktop(direct, L"")
    {}

    Desktop::Desktop(std::shared_ptr<IDirect> direct, std::wstring_view displayName)
    {
        L(trace, L"[Desktop::Desktop] called with display name: {}", displayName);
        this->duplicators = {
            std::make_shared<D3D11Desktop>(direct),
            std::make_shared<D3D9Desktop>(),
            std::make_shared<GDIDesktop>(),
        };
        this->SwitchDisplay(displayName);
    }

    DesktopInfo Desktop::GetInfo() const {
        for (auto& duplicator : this->duplicators) {
            if (!duplicator->IsAvailable()) continue;
            return duplicator->GetDesktopInfo();
        }
        L(debug, "[Desktop::GetInfo] not found any available duplicator, return empty desktop info");
        return DesktopInfo();
    }

    bool Desktop::Duplicate(void* output) {
        L(trace, "[Desktop::Duplicate] called");

        for (auto& duplicator : this->duplicators) {
            if (!duplicator->IsAvailable()) continue;

            if (duplicator->Duplicate(output)) return true;
        }

        L(debug, "[Desktop::Duplicate] failed");
        return false;
    }

    void Desktop::SwitchDisplay(std::wstring_view displayName) {
        L(trace, L"[Desktop::SwitchDisplay] called with display name: {}", displayName);

        for (auto& duplicator : this->duplicators) {
            duplicator->SetDisplay(displayName);
        }
    }
}
