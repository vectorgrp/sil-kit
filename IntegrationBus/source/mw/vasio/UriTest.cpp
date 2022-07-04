//Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Uri.hpp"

#include "ib/exception.hpp"

#include "gtest/gtest.h"

using namespace ib::mw;
TEST(UriTest, parse_uris)
{
	auto uri = Uri::Parse("vib://hostname:1234/path?params");
	ASSERT_EQ(uri.Scheme(), "vib");
	ASSERT_EQ(uri.Host(), "hostname");
	ASSERT_EQ(uri.Port(), 1234);
	ASSERT_EQ(uri.Path(), "path?params"); //currently no query string / parameters / fragments handled
	ASSERT_EQ(uri.EncodedString(), "vib://hostname:1234/path?params" );

	//parsing garbage should throw
	EXPECT_THROW(Uri::Parse("foo/bar"),
		ib::ConfigurationError);
	EXPECT_THROW(Uri::Parse("fo:oo://bar:qux:quz://"),
		ib::ConfigurationError);

	EXPECT_THROW(Uri::Parse("vib://:1234"),
		ib::ConfigurationError);

	// Parse known internal (VAsio) URIs
	uri = Uri::Parse("local:///tmp/domainsockets.vib");
	ASSERT_EQ(uri.Type(), Uri::UriType::Local);
	ASSERT_EQ(uri.Host(), "");
	ASSERT_EQ(uri.Scheme(), "local");
	ASSERT_EQ(uri.Path(), "/tmp/domainsockets.vib");
	ASSERT_EQ(uri.EncodedString(), "local:///tmp/domainsockets.vib");

	uri = Uri::Parse("tcp://123.123.123.123:3456/");
	ASSERT_EQ(uri.Type(), Uri::UriType::Tcp);
	ASSERT_EQ(uri.Scheme(), "tcp");
	ASSERT_EQ(uri.Host(), "123.123.123.123");
	ASSERT_EQ(uri.Port(), 3456);
	ASSERT_EQ(uri.Path(), "");
	ASSERT_EQ(uri.EncodedString(),  "tcp://123.123.123.123:3456/");

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
	uri = Uri::Parse("vib://localhost");
	ASSERT_EQ(uri.Scheme(), "vib");
	ASSERT_EQ(uri.Host(), "localhost");
	ASSERT_EQ(uri.Port(), 8500);
	ASSERT_EQ(uri.Path(), "");
	// Port 0 resolved to random port at runtime, must be 0 when using tcp:// or vib://
	uri = Uri::Parse("vib://localhost:0");
	ASSERT_EQ(uri.Scheme(), "vib");
	ASSERT_EQ(uri.Host(), "localhost");
	ASSERT_EQ(uri.Port(), 0);
	ASSERT_EQ(uri.Path(), "");
}
