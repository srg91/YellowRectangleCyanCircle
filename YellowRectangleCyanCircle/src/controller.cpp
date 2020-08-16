#include "controller.hpp"

namespace YellowRectangleCyanCircle {
	Controller::Controller() :
		context(std::make_shared<Context>())
	{
		this->context->SetDetectorEnabled(DetectorType::Area, true);
		this->actions.clear();
	}

	std::shared_ptr<Context> Controller::GetContext() {
		return this->context;
	}

	bool Controller::IsDetectorEnabled(DetectorType dt) {
		auto lock = this->context->LockOnRead();
		return this->context->IsDetectorEnabled(dt);
	}

	void Controller::EnableDetector(DetectorType dt, bool value) {
		// TODO: check if not status
		// TODO: run update thread
		auto lock = this->context->LockOnWrite();
		this->context->SetDetectorEnabled(dt, value);
	}
}
