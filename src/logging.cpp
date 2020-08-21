#include "logging.hpp"

namespace YellowRectangleCyanCircle {
    namespace Logging {
        void Initialize(std::wstring_view cmdLine) {
            if (cmdLine.find(L"-v") == std::string::npos) return;

            spdlog::level::level_enum level;

            switch (std::count(cmdLine.begin(), cmdLine.end(), L'v')) {
            case 1:
                level = spdlog::level::info;
                break;
            case 2:
                level = spdlog::level::debug;
                break;
            default:
                level = spdlog::level::trace;
            }

            auto logger = spdlog::basic_logger_mt("root", "YellowRectangleCyanCircle.log", true);
            logger->set_level(level);
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");

            spdlog::set_default_logger(logger);

            for (auto s : { SIGABRT, SIGFPE, SIGILL, SIGSEGV, SIGINT }) {
                std::signal(s, signalHandler);
            }
        }

        void signalHandler(int signal) {
            spdlog::default_logger()->flush();
            ::abort();
        }
    }
}
