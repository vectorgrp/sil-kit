// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>

#include "ib/cfg/Config.hpp"

#include "IbExtensions.hpp"

namespace ib { namespace extensions {

//!\brief DllCache keeps a shared reference to already loaded shared libraries.
//
// This allows extensions to cache the already loaded and validated shared
// libraries and reusing them.
//Usage:
// static DllCache cache;
// auto extension = cache.Get("name of extension", config);

class DllCache
{
public:

    auto Get(const std::string& extensionName, const ib::cfg::Config& config)
        -> std::shared_ptr<ib::extensions::IIbExtension>
    {
        try {
            //try to load the extension by its undecorated DLL/so name
            //and cache a reference to it.
            if (!_dll)
            {
                _dll = ib::extensions::LoadExtension(extensionName, config.extensionConfig);
                _extensionName = extensionName;
            }
            if (extensionName != _extensionName)
            {
                throw ExtensionError("Cached Extension " + _extensionName
                    + " differs from requested extension name: " + extensionName);
            }
            return _dll;
        }
        catch (const ib::extensions::ExtensionError& err)
        {
            std::cout << "ERROR loading '" << extensionName << "' extension: " << err.what() << std::endl;
            throw;
        }
    }

private:
    std::string _extensionName;
    std::shared_ptr<ib::extensions::IIbExtension> _dll;
};

}//end namespace extensions
}//end namespace ib
