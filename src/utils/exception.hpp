#pragma once
#include <stdexcept>
#include <cerrno>
#include <cstring>

namespace utils {
    class errno_error  : public std::runtime_error {
    public:
        errno_error() : runtime_error(std::string{"Error " + std::to_string(errno) + ": "} + strerror(errno)){

        }
    };
}