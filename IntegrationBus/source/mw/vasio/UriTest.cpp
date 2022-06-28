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

	//parsing garbage should throw
	EXPECT_THROW(Uri::Parse("foo/bar"),
		ib::ConfigurationError);
	EXPECT_THROW(Uri::Parse("fo:oo://bar:qux:quz://"),
		ib::ConfigurationError);

	// Parse known internal (VAsio) URIs
	uri = Uri::Parse("local:///tmp/domainsockets.vib");
	ASSERT_EQ(uri.Type(), Uri::UriType::Local);
	ASSERT_EQ(uri.Host(), "");
	ASSERT_EQ(uri.Scheme(), "local");
	ASSERT_EQ(uri.Path(), "tmp/domainsockets.vib");

	uri = Uri::Parse("tcp://123.123.123.123:3456/");
	ASSERT_EQ(uri.Type(), Uri::UriType::Tcp);
	ASSERT_EQ(uri.Scheme(), "tcp");
	ASSERT_EQ(uri.Host(), "123.123.123.123");
	ASSERT_EQ(uri.Port(), 3456);
	ASSERT_EQ(uri.Path(), "");
}
