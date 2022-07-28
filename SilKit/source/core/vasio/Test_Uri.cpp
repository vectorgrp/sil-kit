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

#include "Uri.hpp"

#include "silkit/participant/exception.hpp"

#include "gtest/gtest.h"

using namespace SilKit::Core;
TEST(UriTest, parse_uris)
{
	auto uri = Uri::Parse("silkit://hostname:1234/path?params");
	ASSERT_EQ(uri.Scheme(), "silkit");
	ASSERT_EQ(uri.Host(), "hostname");
	ASSERT_EQ(uri.Port(), 1234);
	ASSERT_EQ(uri.Path(), "path?params"); //currently no query string / parameters / fragments handled
	ASSERT_EQ(uri.EncodedString(), "silkit://hostname:1234/path?params" );

	//parsing garbage should throw
	EXPECT_THROW(Uri::Parse("foo/bar"),
		SilKit::ConfigurationError);
	EXPECT_THROW(Uri::Parse("fo:oo://bar:qux:quz://"),
		SilKit::ConfigurationError);

	EXPECT_THROW(Uri::Parse("silkit://:1234"),
		SilKit::ConfigurationError);

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
