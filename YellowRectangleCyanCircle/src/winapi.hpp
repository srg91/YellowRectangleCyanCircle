#pragma once

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "Oleacc.lib")

#include "types.hpp"

#include <atlbase.h>
#include <dwmapi.h>
#include <oleacc.h>
#include <Windows.h>

#include <memory>
#include <string_view>
#include <string>
#include <vector>

namespace YellowRectangleCyanCircle {
	struct IAccessibleObject {
		virtual ~IAccessibleObject() {};

		virtual bool IsWindow() const = 0;
	};

	class AccessibleObject: public IAccessibleObject {
	public:
		AccessibleObject(CComPtr<IAccessible> acc, VARIANT child);
		~AccessibleObject() override final;

		bool IsWindow() const override final;
	private:
		CComPtr<IAccessible> acc;
		VARIANT child;
	};

	struct IWinAPI {
		virtual ~IWinAPI() {};
		
		virtual HWND FindWindowByName(std::wstring_view name) const = 0;
		virtual DWORD GetWindowThreadProcessId(HWND hWnd) const = 0;
		virtual std::wstring GetWindowText(HWND hWnd) const = 0;
		virtual Rect::Rect GetWindowRect(HWND hWnd) const = 0;

		virtual HWINEVENTHOOK SetWinEventHook(DWORD eventId, WINEVENTPROC winEventProc, DWORD idThread = 0, DWORD dwFlags = 0) const = 0;
		virtual bool UnhookWinEvent(HWINEVENTHOOK hook) const = 0;

		virtual std::unique_ptr<IAccessibleObject> AccessibleObjectFromEvent(HWND hWnd, LONG idObject, LONG idChild) const = 0;
	};

	struct WinAPI: public IWinAPI {
		~WinAPI() override final = default;

		HWND FindWindowByName(std::wstring_view name) const override final;
		DWORD GetWindowThreadProcessId(HWND hWnd) const override final;
		std::wstring GetWindowText(HWND hWnd) const override final;
		virtual Rect::Rect GetWindowRect(HWND hWnd) const override final;

		HWINEVENTHOOK SetWinEventHook(DWORD eventId, WINEVENTPROC winEventProc, DWORD idThread = 0, DWORD dwFlags = 0) const override final;
		bool UnhookWinEvent(HWINEVENTHOOK hook) const override final;

		std::unique_ptr<IAccessibleObject> AccessibleObjectFromEvent(HWND hWnd, LONG idObject, LONG idChild) const override final;
	};
}
