#pragma once

#include "hook.hpp"
#include "winapi.test.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace TestHook {
	using namespace YellowRectangleCyanCircle;

	using ::testing::AnyNumber;
	using ::testing::ByMove;
	using ::testing::DoAll;
	using ::testing::Return;
	using ::testing::Pointee;
	using ::testing::SaveArg;

	namespace Defaults {
		auto ExampleLambda = [](HWND hWnd, LONG idObject) { return; };

		auto NonZeroHookHandle = reinterpret_cast<HWINEVENTHOOK>(1);
		auto ZeroHookHandle = reinterpret_cast<HWINEVENTHOOK>(0);

		auto NonZeroHandle = reinterpret_cast<HWND>(1);
		auto ZeroHandle = reinterpret_cast<HWND>(0);

		DWORD EventIdCreate = EVENT_OBJECT_CREATE;
		DWORD EventIdDestroy = EVENT_OBJECT_DESTROY;

		Hook CreateExampleHook(std::shared_ptr<IWinAPI> winAPI, DWORD eventId = EVENT_OBJECT_CREATE);
	}
}
