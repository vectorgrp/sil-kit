/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <cstdint>

#if !defined(SILKIT_EXTENSION_OS)
#   error  "SILKIT_EXTENSION_OS must be defined for the compiler via CMake for the target platform"
#endif

namespace SilKit { 
//! \brief Fields of the BuildInfo array
enum class BuildInfoField
{
    Cxx = 0,
    Compiler,
    Multithread,
    Debug
};

//! \brief Utilities for build infos encoded in shared libraries.
// We encode as much of the build-time information as possible into shared
// libraries.
// The goal is to ensure C++ runtime compatibility, *before* we actively use
// any C++ feature. The build information descriptor is available as a pure C
// symbol.

//! \brief Return the build system's name
// This is provided by a compile-time definition in CMake and must be set.
constexpr const char* BuildinfoSystem()
{
    return SILKIT_EXTENSION_OS;
}

//! \brief Encodes the C++ language standard version.
// This value is used to make sure that the C++ runtime libraries and STL are 
// compatible.
// Returns an unsigned integer encoding the C++ standard version, e.g. 201402L.
constexpr uint32_t BuildinfoCPlusPlus()
{
#if defined(_MSVC_LANG)
    // MSVC compilers don't set __cplusplus by themselves to the current C++
    // standard.
    // We would have to explicitly set the compile flags "/Zc:__plusplus" and
    // an appropriate "/std:c++MN". However, we leave C++ language
    // standard selection and compile flags to CMake and rely on _MSVC_LANG.
    return _MSVC_LANG;
#else
    //GCC and clang always set __cplusplus to current standard version.
    return __cplusplus; 
#endif
}

//! \brief Determines the compiler or toolset used for building.
// Some platforms require a compatible toolset (linker + compiler) or compatible
// runtime. On MSVC we encode the toolset version used, instead of the compiler
// version. This enables us to use extensions compiled with a different compiler
// but linked with a compatible toolset.
// On MSVC returns the toolset version. On GNU/Linux returns the compiler major
// version.
constexpr uint32_t BuildinfoCompiler()
{
#if defined(_MSC_VER)
#   if defined(SILKIT_MSVC_TOOLSET_VERSION)
    // We rely on CMake to provide proper toolset version.
    return SILKIT_MSVC_TOOLSET_VERSION;
#   elif defined(_MSVC_STL_VERSION)
    // In case newer, future toolset is not recognized by CMake:
    // On VS2015/17/19 _MSVC_STL_VERSION contains the toolset number, e.g. 141, 142 etc.
    return _MSVC_STL_VERSION;
#   endif
#else
#   if defined(_WIN32) && defined(__GNUC__)
    //MinGW
    return 0;
#   elif defined( __GNUC__)
    // the major version of a GNU compiler, the C++ macro _GLIBCXX_RELEASE defaults to this
    return __GNUC__;
#   else
    #error Platform has no Extension support
#   endif
#endif
}

//! \brief Encode whether multithreading is enabled.
// Some platforms provide reentrant standard library code. In Linux/glibc the
// definition of _REENTRANT enables this, and clang and gcc define this when
// linking against -lpthread. On MSVC there are also separate multithreading
// runtime libraries.
// Returns 1 if multithreading is enabled, 0 otherwise.
constexpr uint32_t BuildinfoMultithread()
{
#ifdef _MT
    // MSVC defines _MT.
    return 1;
#elif  defined(_REENTRANT)
    return 1;
#else
    return 0;
#endif
}

//! \brief Indicates whether we have a debug build.
// On platforms with ABI incompatibilities between Release and Debug (MSVC),
// returns 1 for Debug builds and 0 otherwise.
// Note that on GNU/Linux this always defaults to 0.
constexpr uint32_t BuildinfoDebug()
{
#ifdef _DEBUG
    //MSVC defines _DEBUG.
    return 1;
#else
    return 0;
#endif
}


}//end namespace SilKit
