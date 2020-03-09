// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "VAsioRegistry.hpp"

using namespace ib::mw;

using asio::ip::tcp;

int main(int argc, char** argv) try
{
    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start registry with: " << argv[0] << " <IbConfig.json> [domainId]" << std::endl;
        return -1;
    }

    std::string jsonFilename(argv[1]);

    uint32_t domainId = 42;
    if (argc >= 3)
    {
        domainId = static_cast<uint32_t>(std::stoul(argv[2]));
    }

    auto ibConfig = ib::cfg::Config::FromJsonFile(jsonFilename);

    std::cout << "Creating VAsioRegistry for IB domain=" << domainId << std::endl;
    VAsioRegistry registry{ibConfig};
    registry.ProvideDomain(domainId);

    std::cout << "Press enter to shutdown registry" << std::endl;
    std::cin.ignore();

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

