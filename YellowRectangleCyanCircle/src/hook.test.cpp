#include "hook.test.hpp"

namespace TestHook {
	Hook Defaults::CreateExampleHook(std::shared_ptr<IWinAPI> winAPI, DWORD eventId) {
		Hook hook(winAPI, eventId);
		hook.SetCallback(ExampleLambda);
		return hook;
	};

	TEST(TestHook, DeathIfAlreadyRegistered) {
		auto winAPI = std::make_shared<MockWinAPI>();

		auto hook1 = Hook(winAPI, Defaults::EventIdCreate);
		auto hook2 = Hook(winAPI, Defaults::EventIdDestroy);

		EXPECT_THROW(
			{ auto hook3 = Hook(winAPI, Defaults::EventIdCreate); },
			HookAlreadyRegistered
		);
	}

	TEST(TestHook, DisableBeforeDeconstruction) {
		auto winAPI = std::make_shared<MockWinAPI>();

		EXPECT_CALL(*winAPI, SetWinEventHook).
			WillRepeatedly(Return(Defaults::NonZeroHookHandle));
		EXPECT_CALL(*winAPI, UnhookWinEvent).Times(1);

		{
			auto hook = Defaults::CreateExampleHook(winAPI);
			hook.Enable();
		}
	}

	TEST(TestHook, UnregisterBeforeDeconstruction) {
		auto winAPI = std::make_shared<WinAPI>();
		{
			Defaults::CreateExampleHook(winAPI);
		}
		EXPECT_NO_THROW({
			Defaults::CreateExampleHook(winAPI);
			});
	}

	TEST(TestHook, Enable) {
		auto winAPI = std::make_shared<MockWinAPI>();

		EXPECT_CALL(*winAPI, SetWinEventHook).
			Times(2).
			WillRepeatedly(Return(Defaults::NonZeroHookHandle));

		{
			auto hook = Hook(winAPI, Defaults::EventIdCreate);
			hook.Enable();
			EXPECT_FALSE(hook.IsEnabled());

			hook.SetCallback(Defaults::ExampleLambda);
			hook.Enable();
			EXPECT_TRUE(hook.IsEnabled());
		}
		{
			auto hook = Hook(winAPI, Defaults::EventIdCreate, Defaults::ZeroHandle, true);
			hook.SetCallback(Defaults::ExampleLambda);
			hook.Enable();
			EXPECT_FALSE(hook.IsEnabled());

			hook.SetHandle(Defaults::NonZeroHandle);
			hook.Enable();
			EXPECT_TRUE(hook.IsEnabled());

			// Should not try to enable, if already enabled
			hook.Enable();
		}
	}

	TEST(TestHook, DisableIfUnableToRegisterCallback) {
		auto winAPI = std::make_shared<MockWinAPI>();

		EXPECT_CALL(*winAPI, SetWinEventHook).
			WillOnce(Return(Defaults::ZeroHookHandle)).
			WillRepeatedly(Return(Defaults::NonZeroHookHandle));

		auto hook = Defaults::CreateExampleHook(winAPI);

		hook.Enable();
		EXPECT_FALSE(hook.IsEnabled());

		hook.Enable();
		EXPECT_TRUE(hook.IsEnabled());
	}

	bool CallbackTestFunc(std::shared_ptr<MockWinAPI> winAPI, Hook&& hook) {
		WINEVENTPROC staticCallback = nullptr;

		EXPECT_CALL(*winAPI, SetWinEventHook).
			WillOnce(DoAll(
				SaveArg<1>(&staticCallback),
				Return(Defaults::NonZeroHookHandle)
			));

		bool isCalled = false;
		auto func = [&isCalled](HWND hWnd, LONG idObject) { isCalled = true; };

		hook.SetCallback(func);
		hook.Enable();
		EXPECT_NE(staticCallback, nullptr);
		EXPECT_TRUE(hook.IsEnabled());

		std::invoke(
			staticCallback,
			Defaults::NonZeroHookHandle,
			Defaults::EventIdCreate,
			Defaults::NonZeroHandle,
			0, 0, 0, 0
		);

		return isCalled;
	}

	TEST(TestHook, CallbackSimple) {
		auto winAPI = std::make_shared<MockWinAPI>();
		EXPECT_TRUE(
			CallbackTestFunc(
				winAPI,
				Hook(winAPI, Defaults::EventIdCreate)
			)
		);
	}

	TEST(TestHook, CallbackWithWindow) {
		for (auto isWindow : { true, false }) {
			auto obj = std::make_unique<MockAccessibleObject>();
			EXPECT_CALL(*obj, IsWindow).WillRepeatedly(Return(isWindow));

			auto winAPI = std::make_shared<MockWinAPI>();
			EXPECT_CALL(*winAPI, AccessibleObjectFromEvent).
				WillOnce(
					Return(ByMove(std::move(obj)))
				);

			bool result = CallbackTestFunc(
				winAPI,
				Hook(winAPI, Defaults::EventIdCreate, Defaults::ZeroHandle, false, true)
			);

			if (isWindow)
				EXPECT_TRUE(result);
			else
				EXPECT_FALSE(result);
		}
	}
}
