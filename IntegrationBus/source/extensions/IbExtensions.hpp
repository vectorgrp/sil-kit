// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <stdexcept>
#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "ib/cfg/Config.hpp"
#include "ib/extensions/ExtensionError.hpp"

#include "IIbExtension.hpp"


/*! IB Extensions
 * 
 * The extension mechanism consists of two parts:
 *   1.) loading dynamic, shared library (ie, *.so or *.dll), and
 *   2.) the extension points derived from IIbExtension.
 *
 * To create an extension for use inside of VIB create an interface as a
 * subclass of IIbExtension, and implement the interface in a
 * shared library.
 *
 * For implementing an extension in shared module form, refer to the
 * IbExtensionApi/ folder.
 */
namespace ib { namespace extensions {

//! \brief Lookup paths to consider when loading dynamic, shared modules
using ExtensionPathHints = std::vector<std::string>;

//! \brief Loads the extension by name, inferring a suitable file path by
//         decorating the name with platform specific prefix, suffix and
//         extensions.
// The first match is loaded.
auto LoadExtension(const std::string& undecorated_name)
    -> std::shared_ptr<IIbExtension>;

//! \brief Loads the extension by name and uses the additional search path hints from
//         the extension configuration.
//! NB: a path hint can contain the prefix "ENV:" to refer to an environment
//! variable name.
auto LoadExtension(
    const std::string& undecorated_name,
    const cfg::ExtensionConfig& config)
    -> std::shared_ptr<IIbExtension>;

}//end namespace extensions
}//end namespace ib
