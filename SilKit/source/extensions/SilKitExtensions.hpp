// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
 * To create an extension for use inside of SIL Kit create an interface as a
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
auto LoadExtension(Services::Logging::ILogger* logger, const std::string& undecorated_name,
                   const Config::Extensions& config) -> std::shared_ptr<ISilKitExtension>;


} //end namespace SilKit
