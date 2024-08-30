/* Copyright (c) 2023 Vector Informatik GmbH

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
