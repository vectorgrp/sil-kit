// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>
#include "IbExtensions.hpp"
#include "ib/cfg/Config.hpp"

namespace ib { namespace extensions {

//!\brief DllCache is keeps the shared library loaded until
//        all instances of its extension objects are destructed.
//Usage:
// static DllCache cache;
// auto extension = cache.Get("name of extension");

struct DllCache
{
    auto Get(const std::string& extensionName, const ib::cfg::Config& config)
        -> std::shared_ptr<ib::extensions::IIbExtension>
    {
        try {
            //try to load the extension by its undecorated DLL/so name
            //and cache a reference to it.
            auto dllInst = _dll.lock();
            if (!dllInst)
            {
                dllInst = ib::extensions::LoadExtension(extensionName, config.extensionConfig);
                _dll = dllInst;
                _extensionName = extensionName;
            }
            return dllInst;
        }
        catch (const ib::extensions::ExtensionError& err)
        {
            std::cout << "ERROR loading '" << extensionName << "' extension: " << err.what() << std::endl;
            throw;
        }
    }
    std::string _extensionName;
    std::weak_ptr<ib::extensions::IIbExtension> _dll;
};

}//end namespace extensions
}//end namespace ib
