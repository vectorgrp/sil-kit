// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ConfigurationTestUtils.hpp"

#include "Uri.hpp"

#include "VAsioRegistry.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace {


TEST(Test_VAsioRegistry, check_start_listening)
{
    using UriType = SilKit::Core::Uri::UriType;

    auto configuration{SilKit::Config::MakeEmptyParticipantConfigurationImpl()};

    {
        SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
        SilKit::Core::Uri registryUri{vAsioRegistry.StartListening("silkit://localhost:0")};
        ASSERT_EQ(registryUri.Type(), UriType::SilKit);
        ASSERT_EQ(registryUri.Host(), "localhost");
        ASSERT_NE(registryUri.Port(), 0);
    }

    {
        SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
        SilKit::Core::Uri registryUri{vAsioRegistry.StartListening("silkit://0.0.0.0:0")};
        ASSERT_EQ(registryUri.Type(), UriType::SilKit);
        ASSERT_EQ(registryUri.Host(), "0.0.0.0");
        ASSERT_NE(registryUri.Port(), 0);
    }

    {
        SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
        SilKit::Core::Uri registryUri{vAsioRegistry.StartListening("silkit://127.0.0.1:0")};
        ASSERT_EQ(registryUri.Type(), UriType::SilKit);
        ASSERT_EQ(registryUri.Host(), "127.0.0.1");
        ASSERT_NE(registryUri.Port(), 0);
    }
}


} // namespace
