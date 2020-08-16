#pragma once

#include "d2d1.hpp"
#include "interface.hpp"
#include "winapi.hpp"

namespace YellowRectangleCyanCircle {
	class Application;

	class Window {
	public:
		Window(
			const std::shared_ptr<D2D1API> d2d1API,
			const std::shared_ptr<WinAPI>& winAPI,
			HINSTANCE hInstance, Application* app,
			std::wstring_view windowClassName
		);
		~Window();
		void Destory();

		HWND GetHandle() const;

		HRESULT OnPaint(const std::shared_ptr<IContext>& context);
		void OnResize(UINT width, UINT height) const;
	private:
		HWND hWnd;
		HINSTANCE hInstance;

		std::shared_ptr<D2D1API> d2d1API;
		std::shared_ptr<D2D1Factory> factory;
		std::shared_ptr<D2D1HwndRenderTarget> renderTarget;
		std::shared_ptr<D2D1SolidColorBrush> brushCyan;
		std::shared_ptr<D2D1SolidColorBrush> brushYellow;

		std::shared_ptr<WinAPI> winAPI;

		void initialize(Application* app, std::wstring_view windowClassName);
		HRESULT initializeResources();
		void clearResources();
	};

	class WindowUpdater : public IAction {
	public:
		WindowUpdater(const std::shared_ptr<WinAPI>& winAPI);

		void Perform(IContext& context) override final;
	private:
		std::shared_ptr<WinAPI> winAPI;
	};
}