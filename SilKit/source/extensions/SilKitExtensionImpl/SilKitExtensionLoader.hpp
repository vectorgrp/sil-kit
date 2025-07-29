// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <sstream>

#include "silkit/services/logging/ILogger.hpp"

#include "SilKitExtensions.hpp"
#include "DllCache.hpp"

namespace SilKit {

//! \brief FactorySingleton creates an instance of the given interface.
// The underlying extension library is cached using a static DllCache instance,
// which keeps the shared library loaded during the lifetime of the calling process.

template <typename FactoryT>
auto SilKitExtensionLoader(Services::Logging::ILogger* logger, const std::string& extensionName,
                           const Config::Extensions& config) -> FactoryT&
{
    static DllCache cache;
    //the dll instance must be kept alive, especially when exceptions are thrown in the factory
    auto& extension = cache.Get(logger, extensionName, config);
    try
    {
        auto& factory = dynamic_cast<FactoryT&>(extension);
        return factory;
    }
    catch (const std::bad_cast& err)
    {
        std::stringstream msg;
        msg << "Error loading SIL Kit extension '" << extensionName << "': " << err.what();
        logger->Error(msg.str());
        throw ExtensionError(msg.str());
    }
}

} //end namespace SilKit
