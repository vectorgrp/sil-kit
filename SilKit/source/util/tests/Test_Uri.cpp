// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <initializer_list>
#include <string>
#include <tuple>

#include <cstdint>

#include "silkit/participant/exception.hpp"

#include "Uri.hpp"


namespace {


using SilKit::Core::Uri;
using SilKit::Core::UriType;


void CheckSilKitUri(const std::string& uriString, const std::string& host, uint16_t port)
{
    Uri parsedUri{uriString};
    EXPECT_EQ(parsedUri.Type(), UriType::SilKit) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Host(), host) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Port(), port) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Path(), "") << parsedUri.EncodedString();

    auto madeUri{Uri::MakeSilKit(host, port, "")};
    EXPECT_EQ(madeUri.Type(), UriType::SilKit) << madeUri.EncodedString();
    EXPECT_EQ(madeUri.Host(), host) << madeUri.EncodedString();
    EXPECT_EQ(madeUri.Port(), port) << madeUri.EncodedString();
    EXPECT_EQ(madeUri.Path(), "") << madeUri.EncodedString();
}

void CheckSilKitUriWithSimName(const std::string& uriString, const std::string& host, uint16_t port,
                               const std::string& simulationName)
{
    Uri parsedUri{uriString};
    EXPECT_EQ(parsedUri.Type(), UriType::SilKit) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Host(), host) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Port(), port) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Path(), simulationName) << parsedUri.EncodedString();

    auto madeUri{Uri::MakeSilKit(host, port, simulationName)};
    EXPECT_EQ(madeUri.Type(), UriType::SilKit) << madeUri.EncodedString();
    EXPECT_EQ(madeUri.Host(), host) << madeUri.EncodedString();
    EXPECT_EQ(madeUri.Port(), port) << madeUri.EncodedString();
    EXPECT_EQ(madeUri.Path(), simulationName) << madeUri.EncodedString();
}

void CheckTcpUri(const std::string& uriString, const std::string& host, uint16_t port)
{
    Uri parsedUri{uriString};
    EXPECT_EQ(parsedUri.Type(), UriType::Tcp) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Host(), host) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Port(), port) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Path(), "") << parsedUri.EncodedString();

    auto madeUri{Uri::MakeTcp(host, port)};
    EXPECT_EQ(madeUri.Type(), UriType::Tcp) << madeUri.EncodedString();
    EXPECT_EQ(madeUri.Host(), host) << madeUri.EncodedString();
    EXPECT_EQ(madeUri.Port(), port) << madeUri.EncodedString();
    EXPECT_EQ(madeUri.Path(), "") << madeUri.EncodedString();
}

void CheckLocalUri(const std::string& uriString, const std::string& path)
{
    Uri parsedUri{uriString};
    EXPECT_EQ(parsedUri.Type(), UriType::Local) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Host(), "") << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Port(), 0) << parsedUri.EncodedString();
    EXPECT_EQ(parsedUri.Path(), path) << parsedUri.EncodedString();

    // there is no Uri::MakeLocal
}


TEST(Test_Uri, parse_silkit_scheme)
{
    CheckSilKitUri("silkit://localhost:8500", "localhost", 8500);
    CheckSilKitUri("silkit://example.com:8500", "example.com", 8500);
    CheckSilKitUri("silkit://0.0.0.0:8500", "0.0.0.0", 8500);
    CheckSilKitUri("silkit://[::]:8500", "[::]", 8500);
    CheckSilKitUri("silkit://[2001:0db8:85a3:08d3:1319:8a2e:0370:7344]:8500",
                   "[2001:0db8:85a3:08d3:1319:8a2e:0370:7344]", 8500);
}


TEST(Test_Uri, parse_silkit_scheme_with_simulation_name)
{
    const std::initializer_list<std::tuple<std::string, uint16_t>> baseUris = {
        {"localhost", 8500},
        {"example.com", 8500},
        {"0.0.0.0", 8500},
        {"127.0.0.1", 8500},
        {"[::]", 8500},
        {"[::1]", 8500},
        {"[2001:0db8:85a3:08d3:1319:8a2e:0370:7344]", 8500},
    };

    for (const auto& baseUri : baseUris)
    {
        std::string host;
        uint16_t port;
        std::tie(host, port) = baseUri;

        std::string uri{"silkit://" + host + ":" + std::to_string(port)};

        CheckSilKitUriWithSimName(uri, host, port, "");
        CheckSilKitUriWithSimName(uri + "/", host, port, "");
        CheckSilKitUriWithSimName(uri + "/a", host, port, "a");
        CheckSilKitUriWithSimName(uri + "/a/b", host, port, "a/b");
        CheckSilKitUriWithSimName(uri + "/a/b/c", host, port, "a/b/c");
        CheckSilKitUriWithSimName(uri + "/a/b/c/", host, port, "a/b/c/");
        CheckSilKitUriWithSimName(uri + "/a.1_2-3~4", host, port, "a.1_2-3~4");

        EXPECT_THROW(Uri::Parse(uri + "//"), SilKit::SilKitError);
        EXPECT_THROW(Uri::Parse(uri + "/a//"), SilKit::SilKitError);
        EXPECT_THROW(Uri::Parse(uri + "/a//b"), SilKit::SilKitError);
        EXPECT_THROW(Uri::Parse(uri + "/a!b"), SilKit::SilKitError);
        EXPECT_THROW(Uri::Parse(uri + "/a?b"), SilKit::SilKitError);
        EXPECT_THROW(Uri::Parse(uri + "/a&b"), SilKit::SilKitError);
        EXPECT_THROW(Uri::Parse(uri + "/a%b"), SilKit::SilKitError);
        EXPECT_THROW(Uri::Parse(uri + "/a$b"), SilKit::SilKitError);
        EXPECT_THROW(Uri::Parse(uri + "/a#b"), SilKit::SilKitError);
        EXPECT_THROW(Uri::Parse(uri + "/a@b"), SilKit::SilKitError);
    }
}


TEST(Test_Uri, parse_tcp_scheme)
{
    CheckTcpUri("tcp://localhost:8500", "localhost", 8500);
    CheckTcpUri("tcp://example.com:8500", "example.com", 8500);
    CheckTcpUri("tcp://0.0.0.0:8500", "0.0.0.0", 8500);
    CheckTcpUri("tcp://[::]:8500", "[::]", 8500);
    CheckTcpUri("tcp://[2001:0db8:85a3:08d3:1319:8a2e:0370:7344]:8500", "[2001:0db8:85a3:08d3:1319:8a2e:0370:7344]",
                8500);
}


TEST(Test_Uri, parse_local_scheme_posix_path)
{
    CheckLocalUri("local:///one/two/three", "/one/two/three");
    CheckLocalUri("local:///", "/");
}


TEST(Test_Uri, parse_local_scheme_windows_path_slash)
{
    CheckLocalUri("local:///", "/");
}


TEST(Test_Uri, parse_local_scheme_windows_path_backslash) {}


TEST(Test_Uri, parse_uris)
{
    auto uri = Uri::Parse("silkit://hostname:1234/path");
    ASSERT_EQ(uri.Scheme(), "silkit");
    ASSERT_EQ(uri.Host(), "hostname");
    ASSERT_EQ(uri.Port(), 1234);
    ASSERT_EQ(uri.Path(), "path"); //currently no query string / parameters / fragments handled
    ASSERT_EQ(uri.EncodedString(), "silkit://hostname:1234/path");

    // SILKIT-1466: query strings are forbidden (until we explicitly support them)
    EXPECT_THROW(Uri::Parse("silkit://hostname:1234/path?params"), SilKit::SilKitError);

    //parsing garbage should throw
    EXPECT_THROW(Uri::Parse("foo/bar"), SilKit::SilKitError);
    EXPECT_THROW(Uri::Parse("fo:oo://bar:qux:quz://"), SilKit::SilKitError);
    EXPECT_THROW(Uri::Parse("silkit://:1234"), SilKit::SilKitError);

    // Parse known internal (VAsio) URIs
    uri = Uri::Parse("local:///tmp/domainsockets.silkit");
    ASSERT_EQ(uri.Type(), Uri::UriType::Local);
    ASSERT_EQ(uri.Host(), "");
    ASSERT_EQ(uri.Scheme(), "local");
    ASSERT_EQ(uri.Path(), "/tmp/domainsockets.silkit");
    ASSERT_EQ(uri.EncodedString(), "local:///tmp/domainsockets.silkit");

    uri = Uri::Parse("tcp://123.123.123.123:3456/");
    ASSERT_EQ(uri.Type(), Uri::UriType::Tcp);
    ASSERT_EQ(uri.Scheme(), "tcp");
    ASSERT_EQ(uri.Host(), "123.123.123.123");
    ASSERT_EQ(uri.Port(), 3456);
    ASSERT_EQ(uri.Path(), "");
    ASSERT_EQ(uri.EncodedString(), "tcp://123.123.123.123:3456/");

    uri = Uri::Parse("tcp://[fe00::0]:3456/");
    ASSERT_EQ(uri.Type(), Uri::UriType::Tcp);
    ASSERT_EQ(uri.Scheme(), "tcp");
    ASSERT_EQ(uri.Host(), "[fe00::0]");
    ASSERT_EQ(uri.Port(), 3456);
    ASSERT_EQ(uri.Path(), "");

    //Windows local paths (contains C:\)
    uri = Uri::Parse(R"aw(local://C:\\temp\hello\ world")aw");
    ASSERT_EQ(uri.Scheme(), "local");
    ASSERT_EQ(uri.Host(), "");
    ASSERT_EQ(uri.Port(), 0);
    ASSERT_EQ(uri.Path(), R"aw(C:\\temp\hello\ world")aw");

    // No Port given results in default port
    uri = Uri::Parse("silkit://localhost");
    ASSERT_EQ(uri.Scheme(), "silkit");
    ASSERT_EQ(uri.Host(), "localhost");
    ASSERT_EQ(uri.Port(), 8500);
    ASSERT_EQ(uri.Path(), "");
    // Port 0 resolved to random port at runtime, must be 0 when using tcp:// or silkit://
    uri = Uri::Parse("silkit://localhost:0");
    ASSERT_EQ(uri.Scheme(), "silkit");
    ASSERT_EQ(uri.Host(), "localhost");
    ASSERT_EQ(uri.Port(), 0);
    ASSERT_EQ(uri.Path(), "");
}


} // namespace
