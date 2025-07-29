// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"
#include "silkit/SilKitMacros.hpp"

#include "extensions/SilKitExtensionImpl/CreateMdf4Tracing.hpp"

/*! \brief Dummy compilation unit to pull in exports from other silkit libs
 *
 *  If you are missing an export from a silkit lib, just add a dummy
 *  function that uses the missing methods or functions.
 */

void __silkit_force_include_extensions_mdf4()
{
    SilKit::CreateMdf4Tracing({}, nullptr, "", "");
    SilKit::CreateMdf4Replay({}, nullptr, "");
}

void __silkit_force_include_participant_configuration()
{
    auto foo1 = SilKit::Config::ParticipantConfigurationFromString("");
    auto foo2 = SilKit::Config::ParticipantConfigurationFromFile("");
}

void __silkit_force_include_experimental()
{
    // Participant extensions
    auto systemController = SilKit::Experimental::Participant::CreateSystemController(nullptr);
    SILKIT_UNUSED_ARG(systemController);

    // LinController extensions
    auto handlerId = SilKit::Experimental::Services::Lin::AddLinSlaveConfigurationHandler(nullptr, nullptr);
    SILKIT_UNUSED_ARG(handlerId);
    SilKit::Experimental::Services::Lin::RemoveLinSlaveConfigurationHandler(nullptr, SilKit::Util::HandlerId{});
    auto slaveConfig = SilKit::Experimental::Services::Lin::GetSlaveConfiguration(nullptr);
    SILKIT_UNUSED_ARG(slaveConfig);
}
