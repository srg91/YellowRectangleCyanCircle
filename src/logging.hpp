#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <csignal>
#include <string>

#ifdef _LOGGING
#define L(level, ...) spdlog::level(__VA_ARGS__)
#else
#define L(level, ...)
#endif

namespace YellowRectangleCyanCircle {
    namespace Logging {
        void Initialize(std::wstring_view cmdLine);

        void signalHandler(int signal);
    }
}
