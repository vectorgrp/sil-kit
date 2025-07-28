// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <stdexcept>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/participant/IParticipant.hpp"
#include "silkit/services/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/SilKit.hpp"

namespace {

TEST(ITest_SameParticipants, test_participants_with_unique_name)
{
    // Needed otherwise temp participants would run out of scope in
    // EXPECT_THROW/EXPECT_NO_THROW and disconnect from the registry
    std::unique_ptr<SilKit::IParticipant> participant1;
    std::unique_ptr<SilKit::IParticipant> participant2;

    // start registry
    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    auto registryUri = registry->StartListening("silkit://localhost:0");

    auto pCfg = SilKit::Config::ParticipantConfigurationFromString("");
    EXPECT_NO_THROW(participant1 = SilKit::CreateParticipant(pCfg, "Participant1", registryUri));
    EXPECT_NO_THROW(participant2 = SilKit::CreateParticipant(pCfg, "Participant2", registryUri));
}

TEST(ITest_SameParticipants, test_participants_with_same_name)
{
    // Needed otherwise temp participants would run out of scope in
    // EXPECT_THROW/EXPECT_NO_THROW and disconnect from the registry
    std::unique_ptr<SilKit::IParticipant> participant1;
    std::unique_ptr<SilKit::IParticipant> participant2;

    // start registry
    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    auto registryUri = registry->StartListening("silkit://localhost:0");

    auto pCfg = SilKit::Config::ParticipantConfigurationFromString("");
    EXPECT_NO_THROW(participant1 = SilKit::CreateParticipant(pCfg, "Participant", registryUri));
    EXPECT_THROW(participant2 = SilKit::CreateParticipant(pCfg, "Participant", registryUri), SilKit::SilKitError);
}

} /* anonymous namespace */
