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

#include <functional>
#include <numeric>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CommandlineParser.hpp"

namespace SilKit {
namespace Util {
namespace tests {

using ::testing::Return;
using ::testing::A;
using ::testing::An;
using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Throw;

auto BuildArguments(const std::vector<const char*>& arguments) -> std::vector<char*>
{
    std::vector<char*> argv;
    for (const auto& arg : arguments)
    {
        argv.push_back(const_cast<char*>(arg));
    }
    argv.push_back(nullptr);

    return argv;
}

TEST(Test_CommandlineParser, test_mixed_arguments)
{
    auto arguments = { "main", "-u=silkit://localhost:8501", "XYZ", "--name=Xxxx", "--version" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "[--version]",
        "-v, --version: Get version info.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "[--help]",
        "-h, --help: Get this help.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "NetworkSimulator", "[--name=<participantName>]",
        "-n, --name <participantName>: The participant name used to take part in the simulation. Defaults to 'NetworkSimulator'");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("registryUri", "u", "silkit://localhost:8500", "[--registryUri=<registryUri>]",
        "-u, --registryUri <registryUri>: The registry URI to connect to. Defaults to 'silkit://localhost:8500'");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "<configuration>",
        "<configuration>: Path and filename of the network simulator configuration YAML or JSON file. Note that the format was changed in v3.6.11");

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").Value(), "silkit://localhost:8500");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "NetworkSimulator");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "");

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").Value(), "silkit://localhost:8501");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "Xxxx");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "XYZ");
}

TEST(Test_CommandlineParser, test_spaced_option_values)
{
    auto arguments = { "main", "-u", "silkit://localhost:8501", "XYZ", "--name", "Xxxx", "--version" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("registryUri", "u", "", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "", "");

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").Value(), "silkit://localhost:8501");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "Xxxx");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "XYZ");
}

TEST(Test_CommandlineParser, test_bad_argument)
{
    auto arguments = { "main", "-registryUri=silkit://localhost:8501", "XYZ", "--name=Xxxx", "--version" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("registryUri", "u", "", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "", "");

    EXPECT_THROW(commandlineParser.ParseArguments(argc, argv), SilKitError);
}

TEST(Test_CommandlineParser, test_no_arguments)
{
    auto arguments = { "main" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "NetworkSimulator", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("registryUri", "u", "silkit://localhost:8500", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "", "");

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").Value(), "silkit://localhost:8500");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "NetworkSimulator");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "");
}

TEST(Test_CommandlineParser, test_missing_option_value)
{
    auto arguments = { "main", "--registryUri" };
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("registryUri", "u", "", "", "");

    EXPECT_THROW(commandlineParser.ParseArguments(argc, argv), SilKitError);
}


} // namespace tests
} // namespace Util
} // namespace SilKit
