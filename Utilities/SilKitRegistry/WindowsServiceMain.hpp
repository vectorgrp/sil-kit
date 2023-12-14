// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/vendor/ISilKitRegistry.hpp"
#include "silkit/participant/exception.hpp"

#include <functional>
#include <memory>

#include "Registry.hpp"

#if defined(_WIN32) && defined(_MSC_VER)
#    define HAS_SILKIT_REGISTRY_WINDOWS_SERVICE 1
#endif

namespace SilKitRegistry {

using StartFunction = std::function<SilKitRegistry::RegistryInstance()>;

#ifdef HAS_SILKIT_REGISTRY_WINDOWS_SERVICE

void RunWindowsService(StartFunction start);

inline constexpr bool HasWindowsServiceSupport()
{
    return true;
}

#else

inline void RunWindowsService(StartFunction)
{
    throw SilKit::LogicError("Running the registry as a Windows Service is not supported by this executable.");
}

inline constexpr bool HasWindowsServiceSupport()
{
    return false;
}

#endif

} // namespace SilKitRegistry
