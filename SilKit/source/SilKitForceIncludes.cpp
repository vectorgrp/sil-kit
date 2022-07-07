// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "silkit/config/IParticipantConfiguration.hpp"

/*! \brief Dummy compilation unit to pull in exports from other silkit libs
 *
 *  If you are missing an export from a silkit lib, just add a dummy
 *  function that uses the missing methods or functions.
 */

void __silkit_force_include_participant_configuration()
{
    auto foo1 = SilKit::Config::ParticipantConfigurationFromString("");
    auto foo2 = SilKit::Config::ParticipantConfigurationFromFile("");
}
