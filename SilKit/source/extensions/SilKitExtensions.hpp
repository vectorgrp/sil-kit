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

#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "ParticipantConfiguration.hpp"
#include "silkit/participant/exception.hpp"
#include "silkit/services/logging/ILogger.hpp"

#include "ISilKitExtension.hpp"


/*! SIL Kit Extensions
 * 
 * The extension mechanism consists of two parts:
 *   1.) loading dynamic, shared library (ie, *.so or *.dll), and
 *   2.) the extension points derived from ISilKitExtension.
 *
 * To create an extension for use inside of SILKIT create an interface as a
 * subclass of ISilKitExtension, and implement the interface in a
 * shared library.
 *
 * For implementing an extension in shared module form, refer to the
 * SilKitExtensionApi/ folder.
 */
namespace SilKit { 

//! \brief Lookup paths to consider when loading dynamic, shared modules
using ExtensionPathHints = std::vector<std::string>;

//! \brief Loads the extension by name, inferring a suitable file path by
//         decorating the name with platform specific prefix, suffix and
//         extensions.
// The first match is loaded.
auto LoadExtension(Services::Logging::ILogger* logger,
    const std::string& undecorated_name) -> std::shared_ptr<ISilKitExtension>;

//! \brief Loads the extension by name and uses the additional search path hints from
//         the extension configuration.
//! NB: a path hint can contain the prefix "ENV:" to refer to an environment
//! variable name.
auto LoadExtension(Services::Logging::ILogger* logger,
    const std::string& undecorated_name, const Config::Extensions& config)
    -> std::shared_ptr<ISilKitExtension>;


}//end namespace SilKit
