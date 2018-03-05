#pragma once

#include "spdlog/spdlog.h"

namespace mock_conduits {

inline auto getLogger(std::string logger_name = "reconduit_test")
{
    static auto logger{ spdlog::stdout_color_mt( logger_name ) };
    return logger;
}

template<typename T> struct PrintType;

}
