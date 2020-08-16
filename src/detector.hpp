#pragma once

#include "interface.hpp"
#include "shapes.hpp"
#include "types.hpp"

#include <array>
#include <unordered_set>

namespace YellowRectangleCyanCircle {
    class AreaDetector : public IAction {
    public:
        void Perform(std::shared_ptr<IContext> context) override final;
    private:
        const DetectorType type = DetectorType::Area;
    };

    class FingerprintDetector : public IAction {
    public:
        void Perform(std::shared_ptr<IContext> context) override final;
    private:
        const DetectorType type = DetectorType::Fingerprint;
    };

    class KeypadDetector : public IAction {
    public:
        void Perform(std::shared_ptr<IContext> context) override final;
    private:
        const DetectorType type = DetectorType::Keypad;

        static const std::array<std::unordered_set<int>, 2> table;
    };
}
