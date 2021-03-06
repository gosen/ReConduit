#ifndef __RECONDUIT_LOGGER__HPP__
#define __RECONDUIT_LOGGER__HPP__

#include "spdlog/spdlog.h"

#include <string>
#include <iostream>

namespace reconduits {

template<typename T, unsigned int sz = sizeof(T)> struct PrintType;
template<typename T, unsigned int sz = sizeof(T)> struct PrintT
{
    PrintT() {
        std::cout << typeid(T).name() << " size("<< sz <<")\n";
    }
};

inline auto getLogger(std::string logger_name = "conduits")
{
    static auto logger{ spdlog::stdout_color_mt( logger_name ) };
    return logger;
}

}

#endif //__RECONDUIT_LOGGER__HPP__
