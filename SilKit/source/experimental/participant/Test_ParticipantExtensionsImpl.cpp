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
