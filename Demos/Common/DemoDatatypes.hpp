#pragma once

#include "silkit/SilKit.hpp"

namespace SilKitDemo 
{

// Parsed from command line
struct RunArgs
{
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration;
    bool runSync;
};

// Context created in the demo runner and given to the application
struct Context
{
    SilKit::IParticipant* participant;
    SilKit::Services::Orchestration::ILifecycleService* lifecycleService;
    SilKit::Services::Logging::ILogger* logger;
};

}