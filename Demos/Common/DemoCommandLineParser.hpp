#pragma once

#include "silkit/SilKit.hpp"
#include "DemoDatatypes.hpp"

using namespace std::chrono_literals;

namespace SilKitDemo {

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

} // namespace SilKitDemo