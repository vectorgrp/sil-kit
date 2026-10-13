// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "config/ConfigurationTestUtils.hpp"

#include "util/Uri.hpp"

#include "core/vasio/VAsioRegistry.hpp"
#include "core/vasio/io/MakeAsioIoContext.hpp"

#include "services/logging/MockLogger.hpp"

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

TEST(Test_VAsioRegistry, start_listening_on_invalid_address_throws)
{
    auto configuration{SilKit::Config::MakeEmptyParticipantConfigurationImpl()};

    SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
    EXPECT_THROW(vAsioRegistry.StartListening("silkit://333.0.0.0:8501"), SilKit::ConfigurationError);
}

auto HasIpV6Loopback() -> bool
{
    ::testing::NiceMock<SilKit::Services::Logging::MockLogger> logger;

    auto ioContext{VSilKit::MakeAsioIoContext({})};
    ioContext->SetLogger(logger);

    try
    {
        ioContext->MakeTcpAcceptor("::1", 0);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

TEST(Test_VAsioRegistry, start_listening_on_ipv6_literal_uses_tcp)
{
    if (!HasIpV6Loopback())
    {
        GTEST_SKIP() << "IPv6 loopback is not available";
    }

    auto configuration{SilKit::Config::MakeEmptyParticipantConfigurationImpl()};

    SilKit::Core::VAsioRegistry vAsioRegistry{configuration};
    SilKit::Core::Uri registryUri{vAsioRegistry.StartListening("silkit://[::1]:0")};
    ASSERT_EQ(registryUri.Host(), "[::1]");
    // the port stays 0 if the registry fell back to domain sockets only
    ASSERT_NE(registryUri.Port(), 0);
}


} // namespace
