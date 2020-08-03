#pragma once

#include "hook.hpp"
#include "types.hpp"

namespace YellowRectangleCyanCircle {
	class Screen : IHookCallbackReceiver {
	public:
		std::uint32_t GetWidth() const noexcept;
		std::uint32_t GetHeight() const noexcept;

		void OnWindowMoved(HWND hWnd) override final;
	private:
		std::uint32_t width = 0;
		std::uint32_t height = 0;
	};
}
