// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace VSilKit {

struct ExecutionEnvironment
{
    std::string operatingSystem;
    std::string hostname;
    std::string pageSize;
    std::string processorCount;
    std::string processorArchitecture;
    std::string physicalMemoryMiB;
    std::string executable;
    std::string username;
};

auto GetExecutionEnvironment() -> ExecutionEnvironment;

} // namespace VSilKit