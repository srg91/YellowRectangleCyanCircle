#include "detector.hpp"

namespace YellowRectangleCyanCircle {
    void AreaDetector::Perform(std::shared_ptr<IContext> context) {
        L(trace, "[AreaDetector::Perform] called");

        if (!context) {
            L(debug, "[AreaDetector::Perform] failed, no context");
            return;
        }

        if (!context->IsDetectorEnabled(this->type)) {
            L(trace, "[AreaDetector::Perform] skip disabled");
            return;
        }

        bool isKeypadEnabled = context->IsDetectorEnabled(DetectorType::Keypad);
        bool isFingerprintEnabled = context->IsDetectorEnabled(DetectorType::Fingerprint);
        if (!(isKeypadEnabled || isFingerprintEnabled)) {
            L(trace, "[AreaDetector::Perform] skip, no one detector enabled");
            return;
        }

        auto image = context->GetScreenImage();
        if (std::empty(image)) {
            L(debug, "[AreaDetector::Perform] got empty screen, do nothing");
            return;
        }

        image = image.clone();
        auto imageRect = image.size();

        auto gameHeight = context->GetGameRect().height;
        if (gameHeight <= 0) gameHeight = imageRect.height;

        int hBlurCoef = std::lround(0.00487f * gameHeight - 0.852f);
        L(trace, "[AreaDetector::Perform] game height [{}] blur coef [{}]", gameHeight, hBlurCoef);

        cv::blur(image, image, cv::Size(hBlurCoef, 4), cv::Point(0, 0));
        cv::threshold(image, image, 20, 255, cv::THRESH_BINARY);

        auto halfWidth = imageRect.width / 2;
        auto halfHeight = imageRect.height / 2;
        auto sixthHeightPart = imageRect.height / 6;

        cv::Rect keypadRect, fpRect, fppRect;

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(image, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        L(trace, "[AreaDetector::Perform] found countours count: {}", std::size(contours));

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

        L(trace, "[AreaDetector::Perform] keypad rect: [{}]", keypadRect);
        L(trace, "[AreaDetector::Perform] fingerprint rects: [{}] [{}]", fpRect, fppRect);

        if (isKeypadEnabled && !keypadRect.empty()) {
            L(debug, "[AreaDetector::Perform] found keypad working area: {}", keypadRect);

            context->SetCurrentDetector(DetectorType::Keypad);
            context->SetWorkingArea(keypadRect);
        }
        else if (isFingerprintEnabled && !(fpRect.empty() || fppRect.empty())) {
            auto area = cv::Rect(
                cv::Point(fppRect.x, fpRect.y),
                cv::Point(fpRect.br().x, fppRect.br().y)
            );

            L(debug, "[AreaDetector::Perform] found fingeprint working area: {}", area);

            context->SetCurrentDetector(DetectorType::Fingerprint);
            context->SetWorkingArea(area);
        }
        else {
            L(debug, "[AreaDetector::Perform] not found any area, clear shapes and counters");

            context->ClearShapes(DetectorType::Fingerprint);
            context->ClearShapes(DetectorType::Keypad);

            context->KeypadClearEmptyRunCounter();
            context->KeypadClearShapesCache();
        }
    }

    void FingerprintDetector::Perform(std::shared_ptr<IContext> context) {
        L(trace, "[FingerprintDetector::Perform] called");

        if (!context) {
            L(debug, "[FingerprintDetector::Perform] failed, no context");
            return;
        }

        if (!context->IsDetectorEnabled(this->type)) {
            L(trace, "[FingerprintDetector::Perform] skip disabled");
            return;
        }

        if (context->GetCurrentDetector() != this->type) {
            L(trace, "[FingerprintDetector::Perform] skip not current");
            return;
        }

        if (context->GetWorkingArea().empty()) {
            L(debug, "[FingerprintDetector::Perform] failed, empty working area");
            return;
        }

        if (std::size(context->GetShapes(this->type)) > 0) {
            L(trace, "[FingerprintDetector::Perform] skip already found");
            return;
        }

        auto image = context->GetScreenImage();
        if (std::empty(image)) {
            L(debug, "[FingerprintDetector::Perform] got empty screen, do nothing");
            return;
        }

        image = image(context->GetWorkingArea()).clone();
        cv::blur(image, image, cv::Size(2, 2), cv::Point(0, 0));
        cv::threshold(image, image, 20, 255, cv::THRESH_BINARY);

        auto imageRect = image.size();
        auto height = imageRect.height;

        auto halfWidth = imageRect.width / 2;
        auto sixthHeightPart = imageRect.height / 6;

        std::vector<Rect::Rect> foundParts;

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(image, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        L(trace, "[FingerprintDetector::Perform] found contours count: {}", std::size(contours));

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

        if (std::size(foundParts) < 8) {
            L(debug, "[FingerprintDetector::Perform] found only {} parts, do nothing", std::size(foundParts));
            return;
        }

        std::vector<std::pair<double, std::shared_ptr<IShape>>> matched;
        for (const auto& part : foundParts) {
            if (part.empty()) {
                L(trace, "[FingerprintDetector::Perform] found empty part: {}", part);
                continue;
            }

            L(trace, "[FingerprintDetector::Perform] part: {}", part);

            auto fppImage = image(part).clone();
            cv::resize(fppImage, fppImage, cv::Size(0, 0), 1.285, 1.285, cv::INTER_LINEAR);

            cv::Mat matchResult;
            cv::matchTemplate(image, fppImage, matchResult, cv::TemplateMatchModes::TM_CCORR_NORMED);

            double minVal = 0, maxVal = 0;
            cv::Point minLoc, maxLoc;
            cv::minMaxLoc(matchResult, &minVal, &maxVal, &minLoc, &maxLoc);

            L(trace, "[FingerprintDetector::Perform] match template: minVal({}) maxVal({}) minLoc({}) maxLoc({})", minVal, maxVal, minLoc, maxLoc);

            if (maxVal > 0.5f) {
                auto matchedValue = std::make_pair(maxVal, std::make_shared<Rectangle>(part));
                auto it = std::upper_bound(matched.begin(), matched.end(), matchedValue, [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
                matched.insert(it, matchedValue);
            }
        }

        auto matchedCount = std::size(matched);
        if (matchedCount >= 4) {
            L(debug, "[FingerprintDetector::Perform] matched");
            context->SetShapes(
                this->type,
                {
                    (matched.end() - 4)->second,
                    (matched.end() - 3)->second,
                    (matched.end() - 2)->second,
                    (matched.end() - 1)->second,
                }
            );
        }
        else {
            L(debug, "[FingerprintDetector::Perform] incorrect match, count: {}", matchedCount);
        }
    }

    const std::array<std::unordered_set<int>, 2> KeypadDetector::table({ {
        {14, 15, 16, 17, 18, 28, 29, 30, 31, 32, 41, 42, 43, 44, 45, 55, 56, 57, 58, 59, 68, 69, 70, 71, 72, 82, 83, 84, 85, 86},
        {15, 16, 17, 18, 19, 29, 30, 31, 32, 33, 42, 43, 44, 45, 46, 56, 57, 58, 59, 60, 70, 71, 72, 73, 74},
    } });

    void KeypadDetector::Perform(std::shared_ptr<IContext> context) {
        L(trace, "[KeypadDetector::Perform] called");

        if (!context) {
            L(debug, "[KeypadDetector::Perform] failed, no context");
            return;
        }

        if (!context->IsDetectorEnabled(this->type)) {
            L(trace, "[KeypadDetector::Perform] skip disabled");
            return;
        }

        if (context->GetCurrentDetector() != this->type) {
            L(trace, "[KeypadDetector::Perform] skip not current");
            return;
        }

        if (context->GetWorkingArea().empty()) {
            L(debug, "[KeypadDetector::Perform] failed, empty working area");
            return;
        }

        if (std::size(context->GetShapes(this->type)) > 0) {
            L(trace, "[KeypadDetector::Perform] skip already found");
            return;
        }

        auto emptyRunCounter = context->KeypadGetEmptyRunCounter();
        auto shapesCacheSize = std::size(context->KeypadGetShapesCache());

        L(trace, "[KeypadDetector::Perform] Empty run counter: {}", emptyRunCounter);
        L(trace, "[KeypadDetector::Perform] Shapes cache size: {}", shapesCacheSize);

        if (emptyRunCounter >= context->KeypadMaxEmptyCirclesInRow && shapesCacheSize > 0) {
            L(debug, "[KeypadDetector::Perform] Found {} empty runs, fill shapes", emptyRunCounter);

            context->SetShapes(this->type, context->KeypadGetShapesCache());
            context->KeypadClearShapesCache();
            context->KeypadClearEmptyRunCounter();
            return;
        }

        auto image = context->GetScreenImage();
        if (std::empty(image)) {
            L(debug, "[KeypadDetector::Perform] got empty screen, do nothing");
            return;
        }

        image = image(context->GetWorkingArea()).clone();

        cv::blur(image, image, cv::Size(3, 3), cv::Point(0, 0));
        cv::threshold(image, image, 100, 255, cv::THRESH_BINARY);

        auto gameHeight = context->GetGameRect().height;
        if (gameHeight <= 0) gameHeight = image.size().height;

        double minDist = std::lround(gameHeight * 0.111f - 19.358f);
        int minRad = std::lround(gameHeight * 0.0332f - 0.808f);
        int maxRad = std::lround(gameHeight * 0.0332f + 9.192f);

        L(trace, "[KeypadDetector::Perform] game height [{}], minimal distance [{}], min radius [{}], max radius [{}]", gameHeight, minDist, minRad, maxRad);

        std::vector<cv::Vec3f> circles;
        cv::HoughCircles(
            image,
            circles,
            cv::HOUGH_GRADIENT,
            1,
            minDist,
            100,
            6,
            minRad,
            maxRad
        );

        std::uint8_t* imagePtr = image.data;

        std::vector<std::shared_ptr<IShape>> foundCircles;
        L(trace, "[KeypadDetector::Perform] found circles count: {}", std::size(foundCircles));

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
        if (foundCount == 0) {
            L(debug, "[KeypadDetector::Perform] empty run");
            context->KeypadRegisterEmptyRun();
        }
        else if (foundCount >= 6) {
            L(debug, "[KeypadDetector::Perform] circles found [{}], save cache", foundCount);
            context->KeypadSetShapesCache(foundCircles);
            context->KeypadClearEmptyRunCounter();
        }
    }
}
