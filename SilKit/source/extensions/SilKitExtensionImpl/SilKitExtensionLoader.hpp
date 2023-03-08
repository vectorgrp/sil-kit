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

#include <memory>
#include <sstream>

#include "silkit/services/logging/ILogger.hpp"

#include "SilKitExtensions.hpp"
#include "DllCache.hpp"

namespace SilKit { 

//! \brief FactorySingleton creates an instance of the given interface.
// The underlying extension library is cached using a static DllCache instance,
// which keeps the shared library loaded during the lifetime of the calling process.

template<typename FactoryT> 
auto SilKitExtensionLoader(Services::Logging::ILogger* logger,
    const std::string& extensionName,
    const Config::Extensions& config)
    -> FactoryT&
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
        msg << "Error loading SILKIT extension '" << extensionName
            << "': " << err.what();
        logger->Error(msg.str());
        throw ExtensionError(msg.str());
    }

}

}//end namespace SilKit
