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
    auto Get(Services::Logging::ILogger* logger, const std::string& extensionName, const Config::Extensions& config)
        -> SilKit::ISilKitExtension&
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
            msg << "Error loading SILKIT extension '" << extensionName << "': " << err.what();
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
