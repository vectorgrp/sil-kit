// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

// Static builds
#ifdef SILKIT_BUILD_STATIC
#   define SilKitAPI
#elif defined(EXPORT_SilKitAPI)
// define SilKitAPI as EXPORT
#    if defined(_WIN32)
#        define SilKitAPI __declspec(dllexport)
#    elif defined(__GNUC__)
#        define SilKitAPI __attribute__((visibility("default")))
#    else
#        define SilKitAPI
#        pragma warning Unknown dynamic link export semantics.
#    endif
#else // defined(EXPORT_SilKitAPI)
// declare SilKitAPI as IMPORT
#    if defined(_WIN32)
#        define SilKitAPI __declspec(dllimport)
#    elif defined(__GNUC__)
#        define SilKitAPI
#    else
#        define SilKitAPI
#        pragma warning Unknown dynamic link import semantics.
#    endif
#endif // SILKIT_BUILD_STATIC
