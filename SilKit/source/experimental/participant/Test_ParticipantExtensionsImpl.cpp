// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/participant/exception.hpp"

#include "NullConnectionParticipant.hpp"
#include "ConfigurationTestUtils.hpp"
#include "ParticipantExtensionsImpl.hpp"

namespace {

using namespace SilKit::Core;

class Test_ParticipantExtensionsImpl : public testing::Test
{
protected:
    Test_ParticipantExtensionsImpl() {}
};

TEST_F(Test_ParticipantExtensionsImpl, create_system_controller_not_null)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    auto systemController = SilKit::Experimental::Participant::CreateSystemControllerImpl(participant.get());
    EXPECT_NE(systemController, nullptr);
}

TEST_F(Test_ParticipantExtensionsImpl, error_on_create_system_controller_twice)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    auto systemController = SilKit::Experimental::Participant::CreateSystemControllerImpl(participant.get());
    SILKIT_UNUSED_ARG(systemController);
    EXPECT_THROW(SilKit::Experimental::Participant::CreateSystemControllerImpl(participant.get()), SilKit::SilKitError);
}

TEST_F(Test_ParticipantExtensionsImpl, create_network_simulator_not_null)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    auto networkSimulator = SilKit::Experimental::Participant::CreateNetworkSimulatorImpl(participant.get());
    EXPECT_NE(networkSimulator, nullptr);
}

TEST_F(Test_ParticipantExtensionsImpl, error_on_create_network_simulator_twice)
{
    auto participant =
        CreateNullConnectionParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "TestParticipant");

    auto networkSimulator = SilKit::Experimental::Participant::CreateNetworkSimulatorImpl(participant.get());
    SILKIT_UNUSED_ARG(networkSimulator);
    EXPECT_THROW(SilKit::Experimental::Participant::CreateNetworkSimulatorImpl(participant.get()), SilKit::SilKitError);
}

} // anonymous namespace
