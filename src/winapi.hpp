#pragma once

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Oleacc.lib")

#include "types.hpp"

#include <dwmapi.h>
#include <oleacc.h>

namespace YellowRectangleCyanCircle {
    namespace WinAPI {
        struct IAccessibleObject {
            virtual ~IAccessibleObject() {};

            virtual bool IsWindow() const = 0;
        };

        class AccessibleObject : public IAccessibleObject {
        public:
            AccessibleObject(CComPtr<IAccessible> acc, VARIANT child);
            ~AccessibleObject() override final;

            bool IsWindow() const override final;
            static std::unique_ptr<IAccessibleObject> FromEvent(HWND hWnd, LONG idObject, LONG idChild);
        private:
            CComPtr<IAccessible> acc;
            VARIANT child;
        };

        Rect::Rect GetClientRect(HWND hWnd);
        Rect::Rect GetWindowRect(HWND hWnd);
        std::wstring GetWindowDisplayName(HWND hWnd);
        std::wstring GetWindowText_(HWND hWnd);
    }
}
