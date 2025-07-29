// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <memory>

#include "ParticipantConfiguration.hpp"

#include "SilKitExtensions.hpp"

namespace SilKit {


//!\brief DllCache keeps a shared reference to already loaded shared libraries.
//
// This allows extensions to cache the already loaded and validated shared
// libraries and reusing them.
//Usage:
// static DllCache cache;
// auto extension = cache.Get("name of extension");

class DllCache
{
public:
    auto Get(Services::Logging::ILogger* logger, const std::string& extensionName,
             const Config::Extensions& config) -> SilKit::ISilKitExtension&
    {
        try
        {
            //try to load the extension by its undecorated DLL/so name
            //and cache a reference to it.
            if (!_dll)
            {
                _dll = SilKit::LoadExtension(logger, extensionName, config);
                _extensionName = extensionName;
            }
            if (extensionName != _extensionName)
            {
                throw ExtensionError("Cached Extension " + _extensionName
                                     + " differs from requested extension name: " + extensionName);
            }
            return *_dll;
        }
        catch (const SilKit::ExtensionError& err)
        {
            std::stringstream msg;
            msg << "Error loading SIL Kit extension '" << extensionName << "': " << err.what();
            if (logger)
            {
                logger->Error(msg.str());
            }
            throw SilKit::ExtensionError{msg.str()};
        }
    }

private:
    std::string _extensionName;
    std::shared_ptr<SilKit::ISilKitExtension> _dll;
};


} //end namespace SilKit
