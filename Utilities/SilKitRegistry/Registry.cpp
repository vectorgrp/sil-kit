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

#include <fstream>
#include <random>

#include "silkit/SilKitVersion.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/services/logging/string_utils.hpp"

#include "SignalHandler.hpp"
#include "WindowsServiceMain.hpp"

#include "VAsioRegistry.hpp"
#include "CommandlineParser.hpp"
#include "ParticipantConfiguration.hpp"
#include "Filesystem.hpp"
#include "YamlParser.hpp"
#include "ValidateAndSanitizeConfig.hpp"

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

void ConfigureLogging(std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration,
                      const std::string& logLevel)
{
    auto config = std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(configuration);
    SILKIT_ASSERT(config != nullptr);

    SilKit::Config::Sink newSink{};
    newSink.type = SilKit::Config::Sink::Type::Stdout;
    newSink.level = SilKit::Services::Logging::from_string(logLevel);
    config->logging.sinks.emplace_back(std::move(newSink));

}

void ConfigureLoggingForWindowsService(std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration,
                                       const std::string& logLevel)
{
    namespace fs = SilKit::Filesystem;

    auto config = std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(configuration);
    SILKIT_ASSERT(config != nullptr);

    SilKit::Config::Sink newSink{};
    newSink.type = SilKit::Config::Sink::Type::File;
    newSink.level = SilKit::Services::Logging::from_string(logLevel);
    newSink.logName = []() -> std::string {
        std::ostringstream path;
        path << fs::temp_directory_path().string() << fs::path::preferred_separator
             << "sil-kit-registry_windows-service";
        return path.str();
    }();
    config->logging.sinks.emplace_back(std::move(newSink));
}

void SanitizeConfiguration(std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration,
                           const std::string& listenUri)
{
    auto originalConfiguration = std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(configuration);
    SILKIT_ASSERT(originalConfiguration != nullptr);

    // generate a dummy participant name to satisfy the constraints of the sanitization function
    const std::string participantName =
        originalConfiguration->participantName.empty() ? "SIL Kit Registry" : originalConfiguration->participantName;

    // sanitize the configuration to select the correct listenUri
    auto result = ValidateAndSanitizeConfig(configuration, participantName, listenUri);

    // reset the participant name in the resulting configuration
    result.participantConfiguration.participantName = originalConfiguration->participantName;

    *originalConfiguration = result.participantConfiguration;
}

auto ExtractRegistryUriFromConfiguration(std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration)
    -> std::string
{
    auto cfg = std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(configuration);
    SILKIT_ASSERT(cfg != nullptr);

    return cfg->middleware.registryUri;
}

auto GenerateRandomCharacters(const std::string& generatedConfigurationPath, size_t count) -> std::string
{
    std::mt19937_64 gen{
        std::hash<std::string>{}(generatedConfigurationPath)
        ^ std::hash<std::chrono::steady_clock::rep>{}(std::chrono::steady_clock::now().time_since_epoch().count())};
    std::uniform_int_distribution<int> dis{0, 127};

    std::string result;
    while (result.size() != count)
    {
        const char ch = static_cast<char>(dis(gen));
        if (std::isalnum(ch, std::locale::classic()))
        {
            result.push_back(ch);
        }
    }
    return result;
}

auto StartRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration, std::string listenUri,
                   CommandlineParser::Option generatedConfigurationPathOpt) -> std::unique_ptr<VAsioRegistry>
{
    auto registry = std::make_unique<VAsioRegistry>(configuration);
    const auto chosenListenUri = registry->StartListening(listenUri);

    std::cout << "SIL Kit Registry listening on " << chosenListenUri << std::endl;

    if (generatedConfigurationPathOpt.HasValue())
    {
        const auto generatedConfigurationPath = generatedConfigurationPathOpt.Value();

        SilKit::Config::ParticipantConfiguration generatedConfiguration;
        generatedConfiguration.middleware.registryUri = chosenListenUri;

        namespace fs = SilKit::Filesystem;

        auto tmpPath = fs::path(generatedConfigurationPath + "."
                                + GenerateRandomCharacters(generatedConfigurationPath, 8) + ".tmp");

        const auto serializedConfiguration = [&generatedConfiguration,
                                              path = generatedConfigurationPath]() -> std::string {
            const auto yamlNode = SilKit::Config::to_yaml(generatedConfiguration);

            const auto jsonSuffix = std::string{".json"};
            const auto pathHasJsonSuffix =
                path.size() >= jsonSuffix.size()
                && (path.compare(path.size() - jsonSuffix.size(), std::string::npos, jsonSuffix) == 0);

            if (pathHasJsonSuffix)
            {
                return SilKit::Config::yaml_to_json(yamlNode);
            }
            else
            {
                return YAML::Dump(yamlNode);
            }
        }();

        auto file = std::ofstream{tmpPath.string(), std::ios::out | std::ios::trunc};
        file << serializedConfiguration << std::endl;
        file.close();

        const auto newPath = fs::path(generatedConfigurationPathOpt.Value());

        fs::rename(tmpPath, newPath);
    }

    return registry;
}

} // namespace

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
        "-u, --listen-uri <silkit-uri>: The silkit:// URI the registry should listen on. Defaults to 'silkit://localhost:8500'.");
    commandlineParser.Add<CliParser::Option>(
        "generate-configuration", "g", "", "[--generate-configuration <configuration>]",
        "-g, --generate-configuration <configuration>: Generate a configuration file which includes the URI the "
        "registry listens on. ");
    commandlineParser.Add<SilKit::Util::CommandlineParser::Option>("log", "l", "info", "[--log <level>]",
            "-l, --log <level>: Log to stdout with level 'trace', 'debug', 'warn', 'info', 'error', 'critical' or 'off'. Defaults to 'info'.");

    if (SilKitRegistry::HasWindowsServiceSupport())
    {
        commandlineParser.Add<CliParser::Flag>("windows-service", "W", "[--windows-service]",
                                               "-W, --windows-service: Run as a Windows service.", CliParser::Hidden);
    }

    std::cout << "Vector SIL Kit -- Registry, SIL Kit version: " << SilKit::Version::String() << std::endl
        << std::endl;

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

    if (commandlineParser.Get<CliParser::Flag>("version").Value())
    {
        std::string hash{ SilKit::Version::GitHash() };
        auto shortHash = hash.substr(0, 7);
        std::cout
            << "Version Info:" << std::endl
            << " - Vector SilKit: " << SilKit::Version::String() << ", #" << shortHash << std::endl;

        return 0;
    }

    auto useSignalHandler{ commandlineParser.Get<CliParser::Flag>("use-signal-handler").Value() };
    auto listenUri{ commandlineParser.Get<CliParser::Option>("listen-uri").Value() };
    auto logLevel{ commandlineParser.Get<SilKit::Util::CommandlineParser::Option>("log").Value() };

    bool windowsService{SilKitRegistry::HasWindowsServiceSupport()
                        && commandlineParser.Get<CliParser::Flag>("windows-service").Value()};

    if (!isValidLogLevel(logLevel))
    {
        std::cerr << "Error: Argument '<level>' must be one of 'trace', 'debug',"
                     " 'warn', 'info', 'error', 'critical', 'off'"
                  << std::endl;
        return -1;
    }

    if (useSignalHandler && windowsService)
    {
        std::cerr << "Error: Conflicting flags --windows-service / -W and --use-signal-handler / -s." << std::endl;
        return -1;
    }

    try
    {
        auto configuration = SilKit::Config::ParticipantConfigurationFromString("");

        if (windowsService)
        {
            ConfigureLoggingForWindowsService(configuration, logLevel);
        }
        else
        {
            ConfigureLogging(configuration, logLevel);
        }
        SanitizeConfiguration(configuration, listenUri);

        listenUri = ExtractRegistryUriFromConfiguration(configuration);

        const auto generatedConfigurationPathOpt = commandlineParser.Get<CliParser::Option>("generate-configuration");

        if (windowsService)
        {
            SilKitRegistry::RunWindowsService([=] {
                return StartRegistry(configuration, listenUri, generatedConfigurationPathOpt);
            });
        }
        else
        {
            const auto registry = StartRegistry(configuration, listenUri, generatedConfigurationPathOpt);

            if (useSignalHandler)
            {
                using namespace SilKit::registry;

                auto signalValue = signalPromise.get_future();
                RegisterSignalHandler([](auto sigNum) {
                    signalPromise.set_value(sigNum);
                });
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
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Error in configuration: " << error.what() << std::endl;
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
