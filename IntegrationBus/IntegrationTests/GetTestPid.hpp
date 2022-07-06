// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <sstream>

#if defined(_WIN32)
#   include <process.h>
#   define getpid _getpid
#else // assume POSIX
#   include <unistd.h>
#endif

inline auto MakeTestRegistryUri()
{
    std::stringstream ss;
    int port = 8500;
    int pid = getpid();
    port += pid % 1000; // clamp to [8500, 9500)
    // add a random offset to prevent two tests listening on the same port
    ss << "silkit://localhost:" << port;
    return ss.str();
}
