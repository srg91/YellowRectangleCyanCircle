#pragma once

#include "game.hpp"
#include "winapi.test.hpp"

#include <gtest/gtest.h>

namespace TestGame {
	using namespace YellowRectangleCyanCircle;

	using TestWinAPI::MockWinAPI;

	using ::testing::Return;

	namespace Defaults {
		auto NonZeroHandle = reinterpret_cast<HWND>(1);
		auto ZeroHandle = reinterpret_cast<HWND>(0);

		std::wstring WindowTitle = L"some game";

		Rect::Rect ExampleRect(100, 100, 200, 200);
	}
}
