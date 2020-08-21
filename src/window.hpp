#pragma once

#include "interface.hpp"
#include "logging.hpp"

namespace YellowRectangleCyanCircle {
    struct WindowUpdater : public IAction {
        void Perform(std::shared_ptr<IContext> context) override final;
    };
}
