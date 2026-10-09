// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/DescribeRegistryConnectFailure.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace {


using namespace std::chrono_literals;

using SilKit::Core::ConnectPeerFailure;
using SilKit::Core::ConnectPeerFailures;
using SilKit::Core::DescribeRegistryConnectFailure;

using ::testing::HasSubstr;
using ::testing::Not;


constexpr const char* CONNECT_URI{"silkit://registry.example:8501"};


auto MakeLocalFailure(std::errc errc) -> ConnectPeerFailure
{
    ConnectPeerFailure failure;
    failure.uri = "local:///tmp/registry.silkit";
    failure.isLocal = true;
    failure.stage = ConnectPeerFailure::Stage::Connect;
    failure.errorCode = std::make_error_code(errc);
    failure.message = failure.errorCode.message();
    return failure;
}

auto MakeTcpFailure(const std::string& address, std::errc errc) -> ConnectPeerFailure
{
    ConnectPeerFailure failure;
    failure.uri = "tcp://" + address + ":8501";
    failure.stage = ConnectPeerFailure::Stage::Connect;
    failure.errorCode = std::make_error_code(errc);
    failure.message = failure.errorCode.message();
    return failure;
}

auto MakeResolveFailure() -> ConnectPeerFailure
{
    ConnectPeerFailure failure;
    failure.uri = "tcp://registry.example:8501";
    failure.stage = ConnectPeerFailure::Stage::Resolve;
    failure.message = "resolve: host not found";
    return failure;
}


TEST(Test_DescribeRegistryConnectFailure, connection_refused_names_the_port_and_ignores_local_domain_failures)
{
    const ConnectPeerFailures failures{
        MakeLocalFailure(std::errc::connection_refused),
        MakeTcpFailure("127.0.0.1", std::errc::connection_refused),
        MakeTcpFailure("[::1]", std::errc::connection_refused),
    };

    const auto description{DescribeRegistryConnectFailure(CONNECT_URI, failures, 5000ms)};

    EXPECT_EQ(description.message, "Failed to connect to SIL Kit Registry at 'silkit://registry.example:8501': "
                                   "connection refused. Check that the SIL Kit Registry is running and listening on "
                                   "port 8501.");
    EXPECT_TRUE(description.connectionRefused);
}

TEST(Test_DescribeRegistryConnectFailure, timeout_names_the_timeout)
{
    const ConnectPeerFailures failures{
        MakeLocalFailure(std::errc::connection_refused),
        MakeTcpFailure("192.0.2.1", std::errc::timed_out),
    };

    const auto description{DescribeRegistryConnectFailure(CONNECT_URI, failures, 1234ms)};

    EXPECT_THAT(description.message, HasSubstr(": no answer within 1234ms. Check the address,"));
    EXPECT_FALSE(description.connectionRefused);
}

TEST(Test_DescribeRegistryConnectFailure, unresolvable_host_names_the_host)
{
    const ConnectPeerFailures failures{
        MakeResolveFailure(),
        MakeLocalFailure(std::errc::connection_refused),
    };

    const auto description{DescribeRegistryConnectFailure(CONNECT_URI, failures, 5000ms)};

    EXPECT_THAT(description.message, HasSubstr(": host name 'registry.example' could not be resolved."));
    EXPECT_FALSE(description.connectionRefused);
}

TEST(Test_DescribeRegistryConnectFailure, the_most_specific_failure_is_reported)
{
    // a refused connection on one address shows that the host is up, which is more useful than a timeout on another
    const ConnectPeerFailures failures{
        MakeTcpFailure("192.0.2.1", std::errc::timed_out),
        MakeTcpFailure("[::1]", std::errc::connection_refused),
    };

    const auto description{DescribeRegistryConnectFailure(CONNECT_URI, failures, 5000ms)};

    EXPECT_THAT(description.message, HasSubstr(": connection refused."));
    EXPECT_THAT(description.message, Not(HasSubstr("no answer")));
}

TEST(Test_DescribeRegistryConnectFailure, unreachable_network_uses_the_error_message)
{
    const ConnectPeerFailures failures{
        MakeTcpFailure("192.0.2.1", std::errc::network_unreachable),
    };

    const auto description{DescribeRegistryConnectFailure(CONNECT_URI, failures, 5000ms)};

    EXPECT_THAT(description.message,
                HasSubstr(": " + std::make_error_code(std::errc::network_unreachable).message()
                          + ". Check the address and the network connection"));
}

TEST(Test_DescribeRegistryConnectFailure, local_domain_failure_is_reported_if_nothing_else_was_tried)
{
    auto failure{MakeLocalFailure(std::errc::connection_refused)};
    failure.stage = ConnectPeerFailure::Stage::Other;
    failure.message = "local-domain sockets are disabled via configuration";

    const auto description{DescribeRegistryConnectFailure(CONNECT_URI, {failure}, 5000ms)};

    EXPECT_EQ(description.message, "Failed to connect to SIL Kit Registry at 'silkit://registry.example:8501': "
                                   "local-domain sockets are disabled via configuration.");
}

TEST(Test_DescribeRegistryConnectFailure, without_failures_only_the_uri_is_named)
{
    const auto description{DescribeRegistryConnectFailure(CONNECT_URI, {}, 5000ms)};

    EXPECT_EQ(description.message, "Failed to connect to SIL Kit Registry at 'silkit://registry.example:8501'.");
}


} // namespace
