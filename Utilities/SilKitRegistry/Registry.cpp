// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "silkit/version.hpp"
#include "silkit/cfg/IParticipantConfiguration.hpp"
#include "silkit/core/logging/string_utils.hpp"

#include "SignalHandler.hpp"
#include "VAsioRegistry.hpp"
#include "CommandlineParser.hpp"
#include "ParticipantConfiguration.hpp"

using namespace SilKit::Core;

using asio::ip::tcp;
using namespace SilKit::Util;
using CliParser = SilKit::Util::CommandlineParser;

namespace {
auto lowerCase(std::string s)
{
    std::transform(s.begin(),
        s.end(),
        s.begin(),
        [](unsigned char c){ return (unsigned char)std::tolower(c);});
    return s;
}
auto isValidLogLevel(const std::string& levelStr)
{
    auto logLevel = lowerCase(levelStr);
    return logLevel == "trace"
        || logLevel == "debug"
        || logLevel == "warn"
        || logLevel == "info"
        || logLevel == "error"
        || logLevel == "critical"
        || logLevel == "off";
}

auto ConfigureLogging(std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration,
    const std::string& logLevel)
{
    auto config = std::static_pointer_cast<SilKit::Config::ParticipantConfiguration>(configuration);

    auto it = std::find_if(config->logging.sinks.begin(),
        config->logging.sinks.end(),
        [](auto const& el) { return el.type == SilKit::Config::Sink::Type::Stdout;}
    );

    auto level = SilKit::Core::Logging::from_string(logLevel);

    if (it != config->logging.sinks.end())
    {
        it->level = level;
    }
    else
    {
        SilKit::Config::Sink newSink{};
        newSink.type = SilKit::Config::Sink::Type::Stdout;
        newSink.level = level;
        config->logging.sinks.emplace_back(std::move(newSink));
    }
}

} //namespace


std::promise<int> signalPromise;
int main(int argc, char** argv)
{
    CliParser commandlineParser;
    commandlineParser.Add<CliParser::Flag>("version", "v", "[--version]",
        "-v, --version: Get version info.");
    commandlineParser.Add<CliParser::Flag>("help", "h", "[--help]",
        "-h, --help: Get this help.");
    commandlineParser.Add<CliParser::Flag>("use-signal-handler", "s", "[--use-signal-handler]",
        "-s, --use-signal-handler: Exit this process when a signal is received. If not set, the process runs infinitely.");
    commandlineParser.Add<CliParser::Option>(
        "listen-uri", "u", "silkit://localhost:8500", "[--listen-uri <uri>]",
        "-u, --listen-uri <vib-uri>: The silkit:// URI the registry should listen on. Defaults to 'silkit://localhost:8500'.");
    commandlineParser.Add<CliParser::Option>(
        "configuration", "c", "", "[--configuration <configuration>]",
        "-c, --configuration <configuration>: Path and filename of the Participant configuration YAML or JSON file. Note that the "
        "format was changed in v3.6.11.");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("log", "l", "info", "[--log <level>]",
            "-l, --log <level>: Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. Defaults to 'info'.");

    std::cout << "Vector SilKit -- Registry of the VAsio Middleware" << std::endl
        << std::endl;

    try
    {
        commandlineParser.ParseArguments(argc, argv);
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

    if (commandlineParser.Get<CliParser::Flag>("version").Value())
    {
        std::string hash{ SilKit::Version::GitHash() };
        auto shortHash = hash.substr(0, 7);
        std::cout
            << "Version Info:" << std::endl
            << " - Vector SilKit: " << SilKit::Version::String() << ", #" << shortHash << std::endl;

        return 0;
    }

    auto&& configurationFilename{ commandlineParser.Get<CliParser::Option>("configuration").Value() };
    auto useSignalHandler{ commandlineParser.Get<CliParser::Flag>("use-signal-handler").Value() };
    auto listenUri{ commandlineParser.Get<CliParser::Option>("listen-uri").Value() };
    auto logLevel{ commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("log").Value() };

    if (!isValidLogLevel(logLevel))
    {
        std::cerr << "Error: Argument '<level>' must be one of 'trace', 'debug',"
                     " 'warn', 'info', 'error', 'critical', 'off'"
                  << std::endl;
        return -1;
    }

    try
    {
        auto configuration = (!configurationFilename.empty()) ?
            SilKit::Config::ParticipantConfigurationFromFile(configurationFilename) :
            SilKit::Config::ParticipantConfigurationFromString("");

        ConfigureLogging(configuration, logLevel);

        std::cout << "SilKitRegistry listening on " << listenUri << std::endl;
        VAsioRegistry registry{ configuration };
        registry.ProvideDomain(listenUri);
        
        if (useSignalHandler)
        {
            using namespace SilKit::registry;


            auto signalValue = signalPromise.get_future();
            RegisterSignalHandler(
                [](auto sigNum)
                {
                    signalPromise.set_value(sigNum);
                }
            );
            std::cout << "Registered signal handler" << std::endl;

            signalValue.wait();

            std::cout << "Signal " << signalValue.get() << " received!" << std::endl;
            std::cout << "Exiting..." << std::endl;
        }
        else
        {
            std::cout << "Press enter to shutdown registry..." << std::endl;
            std::cin.ignore();
        }
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Error: Failed to load configuration '" << configurationFilename << "', " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();

        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();

        return -3;
    }

    return 0;
}
