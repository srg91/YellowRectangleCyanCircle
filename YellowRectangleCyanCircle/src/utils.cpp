#include "utils.hpp"

namespace YellowRectangleCyanCircle {
	std::string utf8_encode(std::wstring_view value) {
		if (std::empty(value)) return std::string();

		int size = WideCharToMultiByte(
			CP_UTF8,
			0,
			std::data(value),
			static_cast<int>(std::size(value)),
			nullptr,
			0,
			nullptr,
			nullptr
		);

		std::string res(size, 0);
		WideCharToMultiByte(
			CP_UTF8,
			0,
			std::data(value),
			static_cast<int>(std::size(value)),
			std::data(res),
			static_cast<int>(std::size(res)),
			nullptr,
			nullptr
		);

		return res;
	}
}