#include "types.hpp"

#include <gtest/gtest.h>

using namespace YellowRectangleCyanCircle;

TEST(TestTypes, FromRect) {
	RECT initial = { 100, 200, 300, 400 };
	Rect::Rect expected = Rect::Rect(100, 200, 200, 200);
		
	auto result = Rect::FromRECT(initial);
	EXPECT_EQ(result, expected);
}
