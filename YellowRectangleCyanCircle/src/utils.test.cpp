#include "utils.hpp"

#include <gtest/gtest.h>

using namespace YellowRectangleCyanCircle;

namespace TestUtils {
	TEST(TestUtils, Utf8EncodeAscii) {
		std::wstring value = L"Hello, world";
		std::string expected = "Hello, world";

		auto result = utf8_encode(value);
		EXPECT_EQ(result, expected);
	}

	TEST(TestUtils, Utf8EncodeCyrillic) {
		std::wstring value = L"Привет, мир";
		std::string expected = "\xd0\x9f\xd1\x80\xd0\xb8\xd0\xb2\xd0\xb5\xd1\x82, \xd0\xbc\xd0\xb8\xd1\x80";

		auto result = utf8_encode(value);
		EXPECT_EQ(result, expected);
	}
}