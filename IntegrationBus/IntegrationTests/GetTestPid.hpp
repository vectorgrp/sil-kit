// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#if defined(_MSC_VER)

    #include <process.h>
    inline auto GetTestPid()
    {
        return _getpid() & 232;
    }

#else // hopefully a posix OS
//FIXME: with Fast-RTPS 1.8.1 on ubuntu 18.04 the generated domain IDs
// were too high: we're now clamping the returned value to [0,232]
    #include <unistd.h>
    inline auto GetTestPid()
    {
        return getpid() & 232;
    }
    
#endif
