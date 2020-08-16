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
		
		virtual HWND CreateWnd(
			DWORD dwExStyle,
			LPCWSTR lpClassName,
			LPCWSTR lpWindowName,
			DWORD dwStyle,
			int X,
			int Y,
			int nWidth,
			int nHeight,
			HWND hWndParent,
			HMENU hMenu,
			HINSTANCE hInstance,
			LPVOID lpParam
		) const = 0;
		virtual bool DestroyWindow(HWND hWnd) const = 0;
		virtual HWND FindWindowByName(std::wstring_view name) const = 0;
		virtual std::wstring GetApplicationDisplayName(HWND hWnd) const = 0;
		virtual Rect::Rect GetClientRect(HWND hWnd) const = 0;
		virtual Rect::Rect GetWindowRect(HWND hWnd) const = 0;
		virtual DWORD GetWindowThreadProcessId(HWND hWnd) const = 0;
		virtual std::wstring GetWndText(HWND hWnd) const = 0;
		virtual bool InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase) const = 0;
		virtual bool MoveWindow(
			HWND hWnd,
			int X,
			int Y,
			int nWidth,
			int nHeight,
			BOOL bRepaint
		) const = 0;
		virtual bool SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags) const = 0;
		virtual bool ShowWindow(HWND hWnd, int nCmdShow) const = 0;
		virtual bool UpdateWindow(HWND hWnd) const = 0;

		virtual HWINEVENTHOOK SetWinEventHook(DWORD eventId, WINEVENTPROC winEventProc, DWORD idThread = 0, DWORD dwFlags = 0) const = 0;
		virtual bool UnhookWinEvent(HWINEVENTHOOK hook) const = 0;

		virtual std::unique_ptr<IAccessibleObject> AccessibleObjectFromEvent(HWND hWnd, LONG idObject, LONG idChild) const = 0;
	};

	struct WinAPI: public IWinAPI {
		~WinAPI() override final = default;

		HWND CreateWnd(
			DWORD dwExStyle,
			LPCWSTR lpClassName,
			LPCWSTR lpWindowName,
			DWORD dwStyle,
			int X,
			int Y,
			int nWidth,
			int nHeight,
			HWND hWndParent,
			HMENU hMenu,
			HINSTANCE hInstance,
			LPVOID lpParam
		) const override final;
		bool DestroyWindow(HWND hWnd) const override final;
		HWND FindWindowByName(std::wstring_view name) const override final;
		std::wstring GetApplicationDisplayName(HWND hWnd) const override final;
		Rect::Rect GetClientRect(HWND hWnd) const override final;
		Rect::Rect GetWindowRect(HWND hWnd) const override final;
		DWORD GetWindowThreadProcessId(HWND hWnd) const override final;
		std::wstring GetWndText(HWND hWnd) const override final;
		bool InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase) const override final;
		bool MoveWindow(
			HWND hWnd,
			int X,
			int Y,
			int nWidth,
			int nHeight,
			BOOL bRepaint
		) const override final;
		bool SetLayeredWindowAttributes(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags) const override final;
		bool ShowWindow(HWND hWnd, int nCmdShow) const override final;
		bool UpdateWindow(HWND hWnd) const override final;

		HWINEVENTHOOK SetWinEventHook(DWORD eventId, WINEVENTPROC winEventProc, DWORD idThread = 0, DWORD dwFlags = 0) const override final;
		bool UnhookWinEvent(HWINEVENTHOOK hook) const override final;

		std::unique_ptr<IAccessibleObject> AccessibleObjectFromEvent(HWND hWnd, LONG idObject, LONG idChild) const override final;
	};
}
