// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>

namespace ib { namespace extensions {
//! \brief Fields of the BuildInfo array
enum class BuildInfoField
{
    Cxx = 0,
    Compiler,
    Multithread,
    Debug
};

//! \brief Return the build system's name
const char* BuildinfoSystem();

//! \brief Utilities for build infos encoded in shared libraries.
constexpr uint32_t BuildinfoCPlusPlus()
{
    return __cplusplus; // requires compile flag /Zc:__cplusplus on MSVC
}
constexpr uint32_t BuildinfoCompiler()
{
#ifdef __GNUC__
    // the major version of a GNU compiler, the C++ macro _GLIBCXX_RELEASE defaults to this
    return __GNUC__;
#elif defined(_MSC_VER)
#   if defined(IB_MSVC_TOOLSET_VERSION)
    // rely on CMake to provide proper toolset version
    return IB_MSVC_TOOLSET_VERSION;
#   elif defined(_MSVC_STL_VERSION)
    // in case newer toolset is not recognized by CMake:
    // On VS2015/17/19_MSVC_STL_VERSION contains the toolset number, e.g. 141, 142 etc.
    return _MSVC_STL_VERSION;
#   endif
#endif
}

constexpr uint32_t BuildinfoMultithread()
{
#ifdef _MT
    return 1;
#elif  defined(_REENTRANT)
    return 1; //gcc -dumpspecs confirms that pthread implies _REENTRANT on GNU/Linux
#else
    return 0;
#endif
}
constexpr uint32_t BuildinfoDebug()
{
#ifdef _DEBUG
    return 1;
#else
    return 0; //gcc does not change its behavior based on '-g'.
#endif
}

}//end namespace extensions
}//end namespace ib
