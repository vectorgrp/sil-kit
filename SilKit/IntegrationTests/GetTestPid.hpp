// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <sstream>

#if defined(_WIN32)
#include <process.h>
#define getpid _getpid
#else // assume POSIX
#include <unistd.h>
#endif


inline auto MakeTestDashboardUri()
{
    std::stringstream ss;
    int port = 20000;
    int pid = getpid();
    port += pid % 1000; // clamp to [8500, 9500)
    // add a random offset to prevent two tests listening on the same port
    ss << "http://localhost:" << port;
    return ss.str();
}
