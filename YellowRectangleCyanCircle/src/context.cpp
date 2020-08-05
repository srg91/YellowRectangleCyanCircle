#include "context.hpp"

namespace YellowRectangleCyanCircle {
	Context::Context()
	{
		this->KeypadClearEmptyRunCounter();
		this->KeypadClearShapesCache();

		for (auto value : { DetectorType::Fingerprint, DetectorType::Keypad }) {
			detectorStates[value] = false;
			detectorShapes[value] = {};
		}
	}

	bool Context::IsGameFound() const {
		return this->isGameFound;
	}

	void Context::SetGameFound(bool value) {
		this->isGameFound = value;
	}

	const Rect::Rect& Context::GetGameRect() const {
		return this->gameRect;
	}

	void Context::SetGameRect(const Rect::Rect& rect) {
		this->gameRect = rect;
	}

	bool Context::IsDetectorEnabled(DetectorType dt) const {
		if (this->detectorStates.find(dt) != this->detectorStates.end())
			return this->detectorStates.at(dt);
		else
			return false;
	}

	void Context::SetDetectorEnabled(DetectorType dt, bool value) {
		this->detectorStates[dt] = value;
	}

	unsigned int Context::KeypadGetEmptyRunCounter() const {
		return this->keypadEmptyRunCounter;
	}

	void Context::KeypadRegisterEmptyRun() {
		this->keypadEmptyRunCounter++;
	}

	void Context::KeypadClearEmptyRunCounter() {
		this->keypadEmptyRunCounter = 0;
	}

	const std::vector<std::shared_ptr<IShape>>& Context::KeypadGetShapesCache() const {
		return this->keypadShapesCache;
	}

	void Context::KeypadSetShapesCache(const std::vector<std::shared_ptr<IShape>>& value) {
		this->keypadShapesCache = value;
	}

	void Context::KeypadClearShapesCache() {
		this->keypadShapesCache.clear();
	}

	const Rect::Rect& Context::GetWorkingArea() const {
		return this->workingArea;
	}

	void Context::SetWorkingArea(const Rect::Rect& rect) {
		this->workingArea = rect;
	}

	const Mat& Context::GetScreenImage() const {
		return this->screenImage;
	}

	void Context::SetScreenImage(const Mat& value) {
		this->screenImage = value;
	}

	std::vector<std::shared_ptr<IShape>> Context::GetShapes(DetectorType dt) const {
		if (this->detectorShapes.find(dt) != detectorShapes.end())
			return this->detectorShapes.at(dt);
		else
			return std::vector<std::shared_ptr<IShape>>();
	};

	void Context::SetShapes(DetectorType dt, const std::vector<std::shared_ptr<IShape>>& shapes) {
		this->detectorShapes[dt] = shapes;
	}

	void Context::ClearShapes(DetectorType dt) {
		if (this->detectorShapes.find(dt) == detectorShapes.end()) return;
		this->detectorShapes.at(dt).clear();
	}
}
