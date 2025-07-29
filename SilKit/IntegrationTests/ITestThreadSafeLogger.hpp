// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <memory>
#include <future>
#include <iostream>
#include <sstream>
#include <string>

namespace SilKit {
namespace Tests {

using namespace SilKit::Config;

using namespace std::chrono_literals;

inline std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

struct ThreadSafeLogger
{
    // No copy
    ThreadSafeLogger(const ThreadSafeLogger&) = delete;
    ThreadSafeLogger& operator=(const ThreadSafeLogger&) = delete;
    // Only movable
    ThreadSafeLogger() = default;
    ThreadSafeLogger(ThreadSafeLogger&&) = default;
    ThreadSafeLogger& operator=(ThreadSafeLogger&&) = default;

    template <typename T>
    auto operator<<(T&& arg) -> ThreadSafeLogger&
    {
        buf << std::forward<T>(arg);
        return *this;
    }

    ~ThreadSafeLogger()
    {
        buf << std::endl;
        std::cout << buf.str();
        std::cout << std::flush;
    }
    std::stringstream buf;
};

//! thread safe stream logger for testing
inline auto Log() -> ThreadSafeLogger
{
    return ThreadSafeLogger();
}

} //namespace Tests
} //namespace SilKit
