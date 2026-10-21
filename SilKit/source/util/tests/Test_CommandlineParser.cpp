// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <functional>
#include <numeric>
#include <sstream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "util/CommandlineParser.hpp"

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

static auto BuildArguments(const std::vector<const char*>& arguments) -> std::vector<char*>
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
    auto arguments = {"main", "-u=silkit://localhost:8501", "XYZ", "--name=Xxxx", "--version"};
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "[--version]",
                                                                 "-v, --version: Get version info.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "[--help]", "-h, --help: Get this help.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>(
        "name", "n", "NetworkSimulator", "[--name=<participantName>]",
        "-n, --name <participantName>: The participant name used to take part in the simulation. Defaults to "
        "'NetworkSimulator'");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>(
        "registryUri", "u", "silkit://localhost:8500", "[--registryUri=<registryUri>]",
        "-u, --registryUri <registryUri>: The registry URI to connect to. Defaults to 'silkit://localhost:8500'");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>(
        "configuration", "<configuration>",
        "<configuration>: Path and filename of the network simulator configuration YAML or JSON file. Note that the "
        "format was changed in v3.6.11");

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").Value(),
              "silkit://localhost:8500");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "NetworkSimulator");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "");

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").Value(),
              "silkit://localhost:8501");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "Xxxx");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "XYZ");
}

TEST(Test_CommandlineParser, test_spaced_option_values)
{
    auto arguments = {"main", "-u", "silkit://localhost:8501", "XYZ", "--name", "Xxxx", "--version"};
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
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").Value(),
              "silkit://localhost:8501");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "Xxxx");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "XYZ");
}

TEST(Test_CommandlineParser, test_bad_argument)
{
    auto arguments = {"main", "-registryUri=silkit://localhost:8501", "XYZ", "--name=Xxxx", "--version"};
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
    auto arguments = {"main"};
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("help", "h", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "NetworkSimulator", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("registryUri", "u", "silkit://localhost:8500", "",
                                                                   "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "", "");

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("help").Value(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("registryUri").Value(),
              "silkit://localhost:8500");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "NetworkSimulator");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").HasValue(), false);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(), "");
}

TEST(Test_CommandlineParser, test_missing_option_value)
{
    auto arguments = {"main", "--registryUri"};
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("registryUri", "u", "", "", "");

    EXPECT_THROW(commandlineParser.ParseArguments(argc, argv), SilKitError);
}

TEST(Test_CommandlineParser, test_print_usage_info_skips_hidden_arguments)
{
    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "[--version]",
                                                                 "-v, --version: Get version info.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("debug", "d", "[--debug]", "-d, --debug: Hidden.",
                                                                 SilKit::Util::CommandlineParser::Hidden);
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "Default", "[--name <name>]",
                                                                   "-n, --name <name>: The name.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("secret", "s", "", "[--secret <secret>]",
                                                                   "-s, --secret <secret>: Hidden.",
                                                                   SilKit::Util::CommandlineParser::Hidden);
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "<configuration>",
                                                                       "<configuration>: The configuration.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::PositionalList>("files", "<file1> [file2 ...]",
                                                                           "<file1>, <file2>, ...: The files.");

    std::ostringstream out;
    commandlineParser.PrintUsageInfo(out, "main");

    EXPECT_EQ(out.str(), "Usage: main [--version] [--name <name>] <configuration> <file1> [file2 ...]\n"
                         "Arguments:\n"
                         "-v, --version: Get version info.\n"
                         "-n, --name <name>: The name.\n"
                         "<configuration>: The configuration.\n"
                         "<file1>, <file2>, ...: The files.\n");
}

TEST(Test_CommandlineParser, test_hidden_arguments_are_parsed)
{
    auto arguments = {"main", "-d", "--secret", "xyz"};
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("debug", "d", "", "",
                                                                 SilKit::Util::CommandlineParser::Hidden);
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("secret", "s", "", "", "",
                                                                   SilKit::Util::CommandlineParser::Hidden);

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("debug").Value(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("secret").Value(), "xyz");
}

TEST(Test_CommandlineParser, test_positional_list_collects_remaining_positionals)
{
    auto arguments = {"main", "config.yaml", "a.txt", "--name=Xxxx", "b.txt"};
    auto args = BuildArguments(arguments);
    auto argc = static_cast<int>(args.size()) - 1;
    auto argv = args.data();

    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Positional>("configuration", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::PositionalList>("files", "", "");

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::PositionalList>("files").HasValues(), false);

    commandlineParser.ParseArguments(argc, argv);

    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Positional>("configuration").Value(),
              "config.yaml");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name").Value(), "Xxxx");
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::PositionalList>("files").HasValues(), true);
    EXPECT_EQ(commandlineParser.Get<SilKit::Util::CommandlineParser::PositionalList>("files").Values(),
              (std::vector<std::string>{"a.txt", "b.txt"}));
}

TEST(Test_CommandlineParser, test_argument_accessors)
{
    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "", "");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "Default", "", "");

    auto& flag = commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("version");
    EXPECT_EQ(flag.ShortName(), "v");
    EXPECT_EQ(flag.DefaultValue(), false);

    auto& option = commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("name");
    EXPECT_EQ(option.ShortName(), "n");
    EXPECT_EQ(option.DefaultValue(), "Default");
}

TEST(Test_CommandlineParser, test_get_unknown_or_mismatched_argument)
{
    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "", "", "");

    EXPECT_THROW(commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("unknown"), SilKitError);
    EXPECT_THROW(commandlineParser.Get<SilKit::Util::CommandlineParser::Flag>("name"), SilKitError);
}

TEST(Test_CommandlineParser, test_parse_errors)
{
    auto parse = [](std::vector<const char*> arguments) {
        auto args = BuildArguments(arguments);
        auto argc = static_cast<int>(args.size()) - 1;

        SilKit::Util::CommandlineParser commandlineParser;
        commandlineParser.Add<SilKit::Util::CommandlineParser::Flag>("version", "v", "", "");
        commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("name", "n", "", "", "");
        commandlineParser.ParseArguments(argc, args.data());
    };

    // No positional argument declared
    EXPECT_THROW(parse({"main", "XYZ"}), SilKitError);
    // Unknown flag, long and short form
    EXPECT_THROW(parse({"main", "--unknown"}), SilKitError);
    EXPECT_THROW(parse({"main", "-x"}), SilKitError);
    // Unknown option with value
    EXPECT_THROW(parse({"main", "--unknown=value"}), SilKitError);
    // A flag does not take a value
    EXPECT_THROW(parse({"main", "--version=true"}), SilKitError);
    // Short option without value
    EXPECT_THROW(parse({"main", "-n"}), SilKitError);
}


} // namespace tests
} // namespace Util
} // namespace SilKit
