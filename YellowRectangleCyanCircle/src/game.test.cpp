#include "game.test.hpp"

namespace TestGame {
	TEST(TestGame, OnWindowCreated) {
		auto winAPI = std::make_shared<MockWinAPI>();

		std::wstring unknownTitle = L"unknown game";

		EXPECT_CALL(*winAPI, GetWindowText).
			Times(2).
			WillOnce(Return(unknownTitle)).
			WillRepeatedly(Return(Defaults::WindowTitle));
		EXPECT_CALL(*winAPI, GetWindowRect).
			Times(1).
			WillRepeatedly(Return(Defaults::ExampleRect));

		Game game(winAPI, Defaults::WindowTitle);

		game.OnWindowCreated(Defaults::NonZeroHandle);
		EXPECT_FALSE(game.IsFound());
		EXPECT_EQ(game.GetRect(), Rect::Rect());

		game.OnWindowCreated(Defaults::NonZeroHandle);
		EXPECT_TRUE(game.IsFound());
		EXPECT_EQ(game.GetRect(), Defaults::ExampleRect);

		// If found - we shouldn't call anything
		game.OnWindowCreated(Defaults::NonZeroHandle);
	}

	TEST(TestGame, OnWindowDestroyed) {
		auto winAPI = std::make_shared<MockWinAPI>();
		
		Game game(winAPI, Defaults::WindowTitle, Defaults::NonZeroHandle, Defaults::ExampleRect);
		
		// Do not destroy if handles not equal
		game.OnWindowDestroyed(Defaults::ZeroHandle);
		EXPECT_TRUE(game.IsFound());

		game.OnWindowDestroyed(Defaults::NonZeroHandle);
		EXPECT_FALSE(game.IsFound());
		EXPECT_EQ(game.GetRect(), Rect::Rect());
	}

	TEST(TestGame, OnWindowMoved) {
		auto winAPI = std::make_shared<MockWinAPI>();

		EXPECT_CALL(*winAPI, GetWindowRect).
			WillRepeatedly(Return(Defaults::ExampleRect));

		Game game(winAPI, Defaults::WindowTitle, Defaults::NonZeroHandle);
		game.OnWindowMoved(Defaults::ZeroHandle);
		EXPECT_EQ(game.GetRect(), Rect::Rect());

		game.OnWindowMoved(Defaults::NonZeroHandle);
		EXPECT_EQ(game.GetRect(), Defaults::ExampleRect);
	}
}
