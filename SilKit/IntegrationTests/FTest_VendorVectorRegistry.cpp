// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(FTest_VendorVectorRegistry, registry_returns_concrete_tcp_port)
{
    auto participantConfiguration{SilKit::Config::ParticipantConfigurationFromString("")};
    auto silKitRegistry{SilKit::Vendor::Vector::CreateSilKitRegistry(participantConfiguration)};
    auto uriString{silKitRegistry->StartListening("silkit://127.0.0.1:0")};

    ASSERT_NE(uriString, "silkit://127.0.0.1:0");
    ASSERT_THAT(uriString, testing::StartsWith("silkit://127.0.0.1:"));
}
