// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>

#include "ParticipantConfiguration.hpp"

#include "IbExtensions.hpp"

namespace ib { namespace extensions {

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

    auto Get(mw::logging::ILogger* logger,
        const std::string& extensionName,
        const cfg::Extensions& config)
        -> ib::extensions::IIbExtension&
    {
        try {
            //try to load the extension by its undecorated DLL/so name
            //and cache a reference to it.
            if (!_dll)
            {
                _dll = ib::extensions::LoadExtension(logger, extensionName, config);
                _extensionName = extensionName;
            }
            if (extensionName != _extensionName)
            {
                throw ExtensionError("Cached Extension " + _extensionName
                    + " differs from requested extension name: " + extensionName);
            }
            return *_dll;
        }
        catch (const ib::extensions::ExtensionError& err)
        {
            std::stringstream msg;
            msg << "ERROR loading VIB extension '" << extensionName << "': " << err.what();
            if(logger)
            {
                logger->Error(msg.str());
            }
            throw ib::extensions::ExtensionError{msg.str()};
        }
    }

private:
    std::string _extensionName;
    std::shared_ptr<ib::extensions::IIbExtension> _dll;
};

}//end namespace extensions
}//end namespace ib
