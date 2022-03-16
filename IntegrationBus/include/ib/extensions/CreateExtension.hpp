// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <functional>

#include "ib/IbMacros.hpp"
#include "ib/extensions/IIbRegistry.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/cfg/IParticipantConfiguration.hpp"


namespace ib { namespace extensions {

/*! \brief Loads the IbRegistry extension from a shared library and
*        returns an instance of IIbRegistry to the caller. Search paths
*        can be specified in the extension element in the configuration,
*        additionally to the default ones (see documentation).
*
* Throws std::runtime_error on error.
*/

IntegrationBusAPI auto CreateIbRegistry(std::shared_ptr<ib::cfg::IParticipantConfiguration> config)
    -> std::unique_ptr<IIbRegistry>;

IntegrationBusAPI auto CreateIbRegistry(mw::logging::ILogger* logger,
    std::shared_ptr<ib::cfg::IParticipantConfiguration> config)
    -> std::unique_ptr<IIbRegistry>;

}//end namespace extensions
}//end namespace ib
