// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#if defined(_MSC_VER)

    #include <process.h>
    inline auto GetTestPid()
    {
        return _getpid();
    }

#else // hopefully a posix OS

    #include <unistd.h>
    inline auto GetTestPid()
    {
        return getpid();
    }
    
#endif
