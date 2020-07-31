#include "game.hpp"

#include <gtest/gtest.h>

using namespace YellowRectangleCyanCircle;

TEST(TestGame, DeathIfAlreadyConstructed) {
	auto winAPI = std::make_shared<WinAPI>();
	Game game(winAPI);

	ASSERT_THROW({ Game secondGame(winAPI); }, GameAlreadyConstructed );
}