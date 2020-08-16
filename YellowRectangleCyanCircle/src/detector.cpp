#include "detector.hpp"

namespace YellowRectangleCyanCircle {
	void AreaDetector::Perform(IContext& context) {
		if (!context.IsDetectorEnabled(this->type)) return;

		bool isKeypadEnabled = context.IsDetectorEnabled(DetectorType::Keypad);
		bool isFingerprintEnabled = context.IsDetectorEnabled(DetectorType::Fingerprint);

		if (!(isKeypadEnabled || isFingerprintEnabled)) return;

		auto image = Mat(context.GetScreenImage()).clone();
		cv::blur(image, image, cv::Size(2, 4), cv::Point(0, 0));
		cv::threshold(image, image, 20, 255, cv::THRESH_BINARY);

		auto imageRect = image.size();

		auto halfWidth = imageRect.width / 2;
		auto halfHeight = imageRect.height / 2;
		auto sixthHeightPart = imageRect.height / 6;

		cv::Rect keypadRect, fpRect, fppRect;

		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(image, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

		for (const auto& origContour : contours) {
			bool isFingerWindowsFound =
				!isFingerprintEnabled ||
				(isFingerprintEnabled && !fpRect.empty() && !fppRect.empty());

			bool isKeypadWindowFound =
				!isKeypadEnabled ||
				(isKeypadEnabled && !keypadRect.empty());

			if (isFingerWindowsFound && isKeypadWindowFound) break;

			std::vector<cv::Point> contour;

			cv::approxPolyDP(
				origContour,
				contour,
				0.01f * cv::arcLength(origContour, true),
				true
			);

			if (std::size(contour) != 4) continue;

			auto contourRect = cv::boundingRect(contour);

			// Keypad cracker window
			if (isKeypadEnabled && keypadRect.empty()) {
				bool xThreshold = 
					(contourRect.x < halfWidth) && 
					(contourRect.br().x > halfWidth);

				bool yThreshold =
					(contourRect.y > sixthHeightPart) &&
					(contourRect.y < sixthHeightPart * 2) &&
					(contourRect.br().y > imageRect.height - sixthHeightPart);

				if (xThreshold && yThreshold) {
					keypadRect = contourRect;
					continue;
				}
			}

			if (isFingerprintEnabled) {
				// Fingerprint window
				if (fpRect.empty()) {
					bool xThreshold = 
						(contourRect.x < halfWidth) && 
						(contourRect.br().x > halfWidth);

					bool yThreshold =
						(contourRect.y < sixthHeightPart) &&
						(contourRect.br().y > sixthHeightPart * 2) &&
						(contourRect.br().y < imageRect.height - sixthHeightPart);

					if (xThreshold && yThreshold) {
						fpRect = contourRect;
						continue;
					}
				}

				// Fingerprint parts window
				if (fppRect.empty()) {
					bool xThreshold = contourRect.x < halfWidth&& contourRect.br().x < halfWidth;
					bool yThreshold =
						contourRect.y > sixthHeightPart &&
						contourRect.y < sixthHeightPart * 2 &&
						contourRect.br().y > imageRect.height - sixthHeightPart;

					if (xThreshold && yThreshold) {
						fppRect = contourRect;
						continue;
					}
				}
			}
		}

		if (isKeypadEnabled && !keypadRect.empty()) {
			context.SetWorkingArea(keypadRect);
		}
		else if (isFingerprintEnabled && !(fpRect.empty() || fppRect.empty())) {
			context.SetWorkingArea(cv::Rect(
				cv::Point(fppRect.x, fpRect.y),
				cv::Point(fpRect.br().x, fppRect.br().y)
			));
		}
		else {
			context.ClearShapes(DetectorType::Fingerprint);
			context.ClearShapes(DetectorType::Keypad);

			context.KeypadClearEmptyRunCounter();
			context.KeypadClearShapesCache();
		}
	}

	void FingerprintDetector::Perform(IContext& context) {
		if (!context.IsDetectorEnabled(this->type)) return;
		if (context.GetWorkingArea().empty()) return;
		if (std::size(context.GetShapes(this->type)) > 0) return;

		auto image = Mat(context.GetScreenImage());
		image = image(context.GetWorkingArea()).clone();
		cv::blur(image, image, cv::Size(2, 2), cv::Point(0, 0));
		cv::threshold(image, image, 20, 255, cv::THRESH_BINARY);

		auto imageRect = image.size();
		auto height = imageRect.height;
		
		auto halfWidth = imageRect.width / 2;
		auto sixthHeightPart = imageRect.height / 6;

		std::vector<Rect::Rect> foundParts;

		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(image, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

		for (const auto& origContour : contours) {
			if (std::size(foundParts) >= 8) break;

			std::vector<cv::Point> contour;

			cv::approxPolyDP(
				origContour,
				contour,
				0.01f * cv::arcLength(origContour, true),
				true
			);

			if (std::size(contour) != 4) continue;
			auto contourRect = cv::boundingRect(contour);

			if (std::size(foundParts) < 8) {
				bool xThreshold = 
					(contourRect.x < halfWidth) && 
					(contourRect.br().x < halfWidth);
				bool wThreshold =
					(contourRect.width > 50) && 
					(contourRect.width < 150);
				bool areaThreshold = std::abs(contourRect.width - contourRect.height) <= 5;

				if (xThreshold && wThreshold && areaThreshold) {
					unsigned int point = (contourRect.y + 2) * image.cols + contourRect.x + 2;
					if (image.data[point] < 200) {
						foundParts.push_back(contourRect);
						continue;
					}
				}
			}
		}

		if (std::size(foundParts) < 8) return;

		std::vector<std::shared_ptr<IShape>> resultParts;
		for (const auto& part : foundParts) {
			auto fppImage = image(part).clone();
			cv::resize(fppImage, fppImage, cv::Size(0, 0), 1.285, 1.285, cv::INTER_LINEAR);

			cv::Mat matchResult;
			cv::matchTemplate(image, fppImage, matchResult, cv::TemplateMatchModes::TM_CCORR_NORMED);

			double minVal = 0, maxVal = 0;
			cv::Point minLoc, maxLoc;
			cv::minMaxLoc(matchResult, &minVal, &maxVal, &minLoc, &maxLoc);

			if (maxVal > 0.75f) {
				resultParts.push_back(
					std::make_shared<Rectangle>(part)
				);
			}
		}

		if (std::size(resultParts) == 4) context.SetShapes(this->type, resultParts);
	}

	const std::array<std::unordered_set<int>, 2> KeypadDetector::table({{
		{14, 15, 16, 17, 18, 28, 29, 30, 31, 32, 41, 42, 43, 44, 45, 55, 56, 57, 58, 59, 68, 69, 70, 71, 72, 82, 83, 84, 85, 86},
		{15, 16, 17, 18, 19, 29, 30, 31, 32, 33, 42, 43, 44, 45, 46, 56, 57, 58, 59, 60, 70, 71, 72, 73, 74},
	}});

	void KeypadDetector::Perform(IContext& context) {
		if (!context.IsDetectorEnabled(this->type)) return;
		if (context.GetWorkingArea().empty()) return;
		if (std::size(context.GetShapes(this->type)) > 0) return;

		if (context.KeypadGetEmptyRunCounter() >= context.KeypadMaxEmptyCirclesInRow) {
			context.SetShapes(this->type, context.KeypadGetShapesCache());
			context.KeypadClearShapesCache();
			context.KeypadClearEmptyRunCounter();
			return;
		}

		cv::Mat image = context.GetScreenImage()(context.GetWorkingArea()).clone();
		cv::blur(image, image, cv::Size(3, 3), cv::Point(0, 0));
		cv::threshold(image, image, 100, 255, cv::THRESH_BINARY);

		std::vector<cv::Vec3f> circles;
		cv::HoughCircles(image, circles, cv::HOUGH_GRADIENT, 1, 80, 100, 6, 15, 50);

		std::uint8_t* imagePtr = image.data;

		std::vector<std::shared_ptr<IShape>> foundCircles;
		for (const auto& circle : circles) {
			unsigned int x = cvRound(circle[0]), y = cvRound(circle[1]);
			unsigned int r = cvRound(circle[2]);

			cv::Point posPercent(
				cvRound(100. * static_cast<float>(x) / static_cast<float>(image.cols)),
				cvRound(100. * static_cast<float>(y) / static_cast<float>(image.rows))
			);

			if (this->table[0].find(posPercent.x) == this->table[0].end()) continue;
			if (this->table[1].find(posPercent.y) == this->table[1].end()) continue;

			// Check color at 4 points
			unsigned int
				leftPoint = (y - 1) * image.cols + x - r / 2 - 1,
				rightPoint = (y - 1) * image.cols + x + r / 2 - 1,
				topPoint = (y - r / 2 - 1) * image.cols + x - 1,
				bottomPoint = (y + r / 2 - 1) * image.cols + x - 1;

			if (imagePtr[leftPoint] > 200 && imagePtr[rightPoint] > 200 &&
				imagePtr[topPoint] > 200 && imagePtr[bottomPoint] > 200) {
				foundCircles.push_back(std::make_shared<Circle>(x, y, r));
			}
		}

		auto foundCount = std::size(foundCircles);
		if (foundCount == 0)
			context.KeypadRegisterEmptyRun();
		else if (foundCount >= 6)
			context.KeypadSetShapesCache(foundCircles);
	}
}