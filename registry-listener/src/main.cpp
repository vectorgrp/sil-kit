// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <exception>
#include <future>
#include <iostream>
#include <memory>

#include "RegistryEventListener.hpp"

#include "CreateSilKitRegistryWithDashboard.hpp"
#include "config/ParticipantConfiguration.hpp"
#include "config/ParticipantConfigurationFromXImpl.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"
#include "util/CommandlineParser.hpp"
#include "util/SignalHandler.hpp"
#include "util/Uri.hpp"

namespace {

class ShutdownSignal
{
public:
    ShutdownSignal()
    {
        SilKit::Util::RegisterSignalHandler([this](int) { _signal.set_value(); });
    }
    ~ShutdownSignal()
    {
        SilKit::Util::ShutdownSignalHandler();
    }
    ShutdownSignal(const ShutdownSignal&) = delete;
    auto operator=(const ShutdownSignal&) -> ShutdownSignal& = delete;

    void Wait()
    {
        _signal.get_future().get();
    }

private:
    std::promise<void> _signal;
};

} // namespace

int main(int argc, char** argv)
{
    try
    {
        using Parser = SilKit::Util::CommandlineParser;
        Parser parser;
        parser.Add<Parser::Flag>("help", "h", "[--help]", "-h, --help: Show this help");
        parser.Add<Parser::Option>("listen-uri", "u", "", "[--listen-uri <uri>]",
                                   "-u, --listen-uri <uri>: Override the configured listen URI (default: silkit://localhost:8500)");
        parser.Add<Parser::Option>("configuration", "c", "", "[--configuration <file>]",
                                   "-c, --configuration <file>: SIL Kit participant configuration (YAML/JSON)");
        parser.ParseArguments(argc, argv);
        if (parser.Get<Parser::Flag>("help").Value())
        {
            parser.PrintUsageInfo(std::cout, argv[0]);
            return 0;
        }

        const auto configFile = parser.Get<Parser::Option>("configuration").Value();
        const auto config = configFile.empty()
                                ? std::make_shared<SilKit::Config::ParticipantConfiguration>()
                                : std::dynamic_pointer_cast<SilKit::Config::ParticipantConfiguration>(
                                      SilKit::Config::ParticipantConfigurationFromFileImpl(configFile));
        if (!config)
        {
            throw SilKit::ConfigurationError{"Expected an internal participant configuration"};
        }

        const auto listenOption = parser.Get<Parser::Option>("listen-uri").Value();
        auto listenUri = config->middleware.registryUri;
        if (!listenOption.empty())
        {
            listenUri = listenOption;
        }
        if (listenUri.empty())
        {
            listenUri = "silkit://localhost:8500";
        }
        if (SilKit::Core::Uri::Parse(listenUri).Type() != SilKit::Core::UriType::SilKit)
        {
            throw SilKit::ConfigurationError{"Listen URI must use the silkit:// scheme"};
        }
        config->middleware.registryUri = listenUri;
        if (!config->experimental.metrics.collectFromRemote.has_value())
        {
            config->experimental.metrics.collectFromRemote = true;
        }

        ShutdownSignal shutdown;
        // The registry borrows the listener; reverse destruction order keeps it alive through teardown.
        RegistryEventListener listener;
        auto registry = VSilKit::CreateSilKitRegistryWithDashboard(config, &listener);
        registry->StartListening(listenUri);
        shutdown.Wait();
    }
    catch (const std::exception& error)
    {
        std::cerr << "registry-listener: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}
