#pragma once

#include "context.hpp"
#include "detector.hpp"
#include "game.hpp"
#include "interface.hpp"
#include "screen.hpp"

#include <thread>

namespace YellowRectangleCyanCircle {
	using ControllerActions = std::vector<std::shared_ptr<IAction>>;

	class Controller {
	public:
		Controller();

		std::shared_ptr<Context> GetContext();

		bool IsDetectorEnabled(DetectorType dt);
		void EnableDetector(DetectorType dt, bool value);
	private:
		ControllerActions actions;
		std::shared_ptr<Context> context;
	};
}
