// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/cfg/IParticipantConfiguration.hpp"
// TODO: remove next include when middleware config is in participant config
#include "ib/cfg/vasio/IMiddlewareConfiguration.hpp"
#include "SignalHandler.hpp"
#include "VAsioRegistry.hpp"

using namespace ib::mw;

using asio::ip::tcp;

std::promise<int> signalPromise;

int main(int argc, char** argv) try
{
    auto useSignalHandler = false;

    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start registry with: " << argv[0] << " <IbConfig.json> [--use-signal-handler] [domainId]" << std::endl;
        return -1;
    }

    std::string jsonFilename(argv[1]);

    uint32_t domainId = 42;

    //check for optional use-signal-handler/domainId arguments
    for (auto i = 2; i < argc && i < 4; i++)
    {
        if (argv[i] == std::string{"--use-signal-handler"})
        {
            useSignalHandler = true;
        }
        else
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[i]));
        }
    }

    // TODO: use this instead when middleware config is in participant config
    //auto ibConfig = ib::cfg::ReadParticipantConfigurationFromJsonFile(jsonFilename);
    auto ibConfig = ib::cfg::vasio::v1::CreateDummyIMiddlewareConfiguration();

    std::cout << "Creating VAsioRegistry for IB domain=" << domainId << std::endl;
    VAsioRegistry registry{ ibConfig };
    registry.ProvideDomain(domainId);

    if (useSignalHandler)
    {
        using namespace ib::registry;


        auto signalValue = signalPromise.get_future();
        RegisterSignalHandler(
            [](auto sigNum)
            {
                signalPromise.set_value(sigNum);
            }
        );
        std::cout << "Registered signal handler" << std::endl;

        signalValue.wait();

        std::cout << "IbRegistry signal " << signalValue.get() << " received!" << std::endl;
        std::cout << "Exiting." << std::endl;
    }
    else
    {
        std::cout << "Press enter to shutdown registry" << std::endl;
        std::cin.ignore();
    }

    return 0;
}
catch (const ib::cfg::Misconfiguration& error)
{
    std::cerr << "Invalid configuration: " << error.what() << std::endl;
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

