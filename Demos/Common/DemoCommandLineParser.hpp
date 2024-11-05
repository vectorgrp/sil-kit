#pragma once

#include "silkit/SilKit.hpp"
#include "DemoDatatypes.hpp"
#include "CommandlineParser.hpp"

using namespace std::chrono_literals;
using CliParser = SilKit::Util::CommandlineParser;

namespace SilKitDemo {

auto ToLowerCase(std::string s) -> std::string
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (unsigned char)std::tolower(c); });
    return s;
}

auto IsValidLogLevel(const std::string& levelStr) -> bool
{
    auto logLevel = ToLowerCase(levelStr);
    return logLevel == "trace" || logLevel == "debug" || logLevel == "warn" || logLevel == "info" || logLevel == "error"
           || logLevel == "critical" || logLevel == "off";
}

int ParseCommandLineArguments2(int argc, char** argv, std::string defaultParticipantName, RunArgs& runArgs)
{
    SilKit::Util::CommandlineParser commandlineParser;
    commandlineParser.Add<CliParser::Flag>("help", "h", "[--help]", "-h, --help: Get this help.");
    commandlineParser.Add<CliParser::Flag>("async", "a", "[--async]", "-a, --async: Run without time synchronization.");
    commandlineParser.Add<CliParser::Option>(
        "connect-uri", "u", "silkit://localhost:8500", "[--connect-uri <silkitUri>]",
        "-u, --connect-uri <silkitUri>: The registry URI to connect to. Defaults to 'silkit://localhost:8500'.");
    commandlineParser.Add<CliParser::Option>("name", "n", defaultParticipantName, "[--name <participantName>]",
                                             "-n, --name <participantName>: The participant name used to take "
                                             "part in the simulation. Defaults to "
                                                 + defaultParticipantName + ".");
    commandlineParser.Add<CliParser::Option>(
        "configuration", "c", "", "[--configuration <filePath>]",
        "-c, --configuration <filePath>: Path to the Participant configuration YAML or JSON file. "
        "Cannot be used together with the '--log' option.");
    commandlineParser.Add<CliParser::Option>(
        "log", "l", "info", "[--log <level>]",
        "-l, --log <level>: Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. "
        "Defaults to 'info' if the '--configuration' option is not specified. Cannot be used together with the "
        "'--configuration' option.");
    
    try
    {
        commandlineParser.ParseArguments(argc, argv);
    }
    catch (const SilKit::SilKitError& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);

        return -1;
    }

    if (commandlineParser.Get<CliParser::Flag>("help").Value())
    {
        commandlineParser.PrintUsageInfo(std::cout, argv[0]);
        return 0;
    }

    const auto participantName{commandlineParser.Get<CliParser::Option>("name").Value()};

    const bool hasLogOption{commandlineParser.Get<CliParser::Option>("log").HasValue()};
    const bool hasCfgOption{commandlineParser.Get<CliParser::Option>("configuration").HasValue()};
    if (hasLogOption && hasCfgOption)
    {
        std::cerr << "Error: Options '--log' and '--configuration' cannot be used simultaneously" << std::endl;
        commandlineParser.PrintUsageInfo(std::cerr, argv[0]);
        return -1;
    }

    const auto configurationFilename{commandlineParser.Get<CliParser::Option>("configuration").Value()};
    const auto connectUri{commandlineParser.Get<CliParser::Option>("connect-uri").Value()};
    const auto runSync{!commandlineParser.Get<CliParser::Flag>("async").Value()};

    const auto logLevel{commandlineParser.Get<CliParser::Option>("log").Value()};
    if (!IsValidLogLevel(logLevel))
    {
        std::cerr << "Error: Argument of the '--log' option must be one of 'trace', 'debug', 'warn', 'info', 'error', "
                     "'critical', or 'off'"
                  << std::endl;
        return -1;
    }

    std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration;
    try
    {
        if (hasCfgOption)
        {
            configuration = SilKit::Config::ParticipantConfigurationFromFile(configurationFilename);
        }
        else
        {
            std::string configLogLevel{logLevel};

            // due to the validation, we know that the first character is ASCII
            configLogLevel[0] = static_cast<char>(std::toupper(configLogLevel[0]));

            std::ostringstream ss;
            ss << R"({"Logging":{"Sinks":[{"Type":"Stdout","Level":")" << configLogLevel << R"("}]}})";

            configuration = SilKit::Config::ParticipantConfigurationFromString(ss.str());
        }
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Error: Failed to load configuration '" << configurationFilename << "', " << error.what()
                  << std::endl;

        return -2;
    }

    runArgs.participantName = std::move(participantName);
    runArgs.participantConfiguration = std::move(configuration);
    runArgs.runSync = runSync;
    runArgs.registryUri = connectUri;

    return 1;
}

/*
RunArgs ParseCommandLineArguments(int argc, char** argv)
{
    if (argc < 1 || argc > 3)
    {
        std::stringstream error_msg;
        error_msg << "Wrong number of command line arguments! Start demo with: " << argv[0]
                  << " [ParticipantConfiguration.yaml|json] [--async]" << std::endl;
        throw SilKit::SilKitError(error_msg.str());
    }

    std::string participantConfigurationFilename = "";
    bool runSync = true;
    std::vector<std::string> args;
    std::copy((argv + 1), (argv + argc), std::back_inserter(args));
    for (auto arg : args)
    {
        if (arg == "--async")
        {
            runSync = false;
        }
        else
        {
            participantConfigurationFilename = arg;
        }
    }

    std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration;
    if (participantConfigurationFilename == "")
    {
        std::string configStr_Info_JSON = R"({"Logging":{"Sinks":[{"Type":"Stdout","Level":"Info"}]}})";
        participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(configStr_Info_JSON);
    }
    else
    {
        participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);
    }

    return RunArgs{ std::move(participantConfiguration), runSync};
}
*/

} // namespace SilKitDemo