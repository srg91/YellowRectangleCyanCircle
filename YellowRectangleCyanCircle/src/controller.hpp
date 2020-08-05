#pragma once

#include "game.hpp"
#include "screen.hpp"

namespace YellowRectangleCyanCircle {
	//struct IController {
	//	virtual void OnTick() = 0;
	//	//virtual void OnResize() = 0;
	//	//virtual void OnPaint() = 0;
	//};

	class Controller {
	public:

	private:
		std::shared_ptr<Game> game;
		std::shared_ptr<Screen> screen;
	};
}