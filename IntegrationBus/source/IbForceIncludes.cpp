// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/cfg/IParticipantConfiguration.hpp"

/*! \brief Dummy compilation unit to pull in exports from other ib libs
 *
 *  If you are missing an export from a ib lib, just add a dummy
 *  function that uses the missing methods or functions.
 */

    
void __ib_force_include_config_builder()
{
    ib::cfg::ConfigBuilder dummyBuilder("DummyConfig");
    dummyBuilder.SimulationSetup().AddParticipant("Dummy").AddEthernet("ETH0").WithMacAddress("");
}

void __ib_force_include_config_parser()
{
    auto config = ib::cfg::Config::FromJsonFile("");
}

void __ib_force_include_participant_configuration()
{
    auto foo1 = ib::cfg::ParticipantConfigurationFromString("");
    auto foo2 = ib::cfg::ParticipantConfigurationFromFile("");
}
