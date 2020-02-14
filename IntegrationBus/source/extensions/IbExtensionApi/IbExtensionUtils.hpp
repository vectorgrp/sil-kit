// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>

namespace ib { namespace extensions {

//! \brief Return the build system's name
const char* BuildinfoSystem();

//! \brief Utilities for build infos encoded in shared libraries.
constexpr uint32_t BuildinfoCPlusPlus()
{
    return __cplusplus;
}
constexpr uint32_t BuildinfoCompiler()
{
#ifdef __GNUC__
    return __GNUC__;
#elif _MSC_VER
    return _MSC_VER;
#endif
}

constexpr uint32_t BuildinfoMultithread()
{
#ifdef _MT
    return 1;
#else
    return 0;
#endif
}
constexpr uint32_t BuildinfoDebug()
{
#ifdef _DEBUG
    return 1;
#else
    return 0;
#endif
}

}//end namespace extensions
}//end namespace ib
