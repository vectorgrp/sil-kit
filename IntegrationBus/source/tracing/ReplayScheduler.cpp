// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ReplayScheduler.hpp"

#include <string>

#include "ib/cfg/Config.hpp"
#include "ib/mw/IComAdapter.hpp"

#include "IReplayDataController.hpp"
#include "Tracing.hpp"

namespace ib {
namespace tracing {

using namespace extensions;

namespace 
{ 
template<typename ConfigT>
auto FindReplayChannel(const ConfigT& controllerConfig, IReplayFile* replayFile) ->  std::shared_ptr<IReplayChannel>
{
    return {};
}

} //end anonymous namespace

ReplayScheduler::ReplayScheduler(const cfg::Config& config,
    const cfg::Participant& participantConfig,
    std::chrono::nanoseconds tickPeriod,
    mw::IComAdapter* comAdapter,
    mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
    , _tickPeriod{tickPeriod}
{
    ConfigureControllers(config, participantConfig);
}

void ReplayScheduler::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}

void ReplayScheduler::ConfigureControllers(const cfg::Config& config, const cfg::Participant& participantConfig)
{
    auto* log = _comAdapter->GetLogger();
    // create trace sources (aka IReplayFile)
    auto replayFiles = tracing::CreateReplayFiles(log, config, participantConfig);
    if (replayFiles.empty())
    {
        log->Error("ReplayScheduler: cannot create replay files.");
        throw std::runtime_error("ReplayScheduler: cannot create replay files.");
    }
    // create controllers listed in config
    auto makeTasks = [this, log, &replayFiles](auto& controllers, auto createMethod) {
        for (const auto& controllerConfig : controllers)
        {
            try {
                ReplayTask task{};

                auto createController = std::bind(createMethod, _comAdapter, std::placeholders::_1);
                auto* controller = createController(controllerConfig.name);

                if (controller == nullptr)
                    throw std::runtime_error("Create controller returned nullptr");

                auto& replayController = dynamic_cast<ReplayController&>(*controller);
                task.controller = &replayController;

                auto replayFile = replayFiles.at(controllerConfig.replay.useTraceSource);
                if (!task.replayChannel)
                    throw std::runtime_error("No replay file found");

                task.replayChannel = FindReplayChannel(controllerConfig, replayFile.get());
                if (!task.replayChannel)
                    throw std::runtime_error("Could not find a replay channel");

                task.initialTime = task.replayChannel->StartTime();

                _replayTasks.emplace_back(std::move(task));
            }
            catch (const std::runtime_error& ex)
            {
                log->Warn("Could not configure controller " + controllerConfig.name
                    + ": " + ex.what());
            }
        }
    };
    makeTasks(participantConfig.ethernetControllers, &mw::IComAdapter::CreateEthController);
    /*
    makeTasks(participantConfig.canControllers);
    makeTasks(participantConfig.flexrayControllers);
    makeTasks(participantConfig.linControllers);
    makeTasks(participantConfig.pwmPorts);
    makeTasks(participantConfig.patternPorts);
    makeTasks(participantConfig.digitalIoPorts);
    makeTasks(participantConfig.analogIoPorts);
    makeTasks(participantConfig.genericPublishers);
    makeTasks(participantConfig.genericSubscribers);
    */
}

void ReplayScheduler::ReplayMessages(std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    for (auto& task : _replayTasks)
    {
        //TODO 
        // - play all messages that have a relative timestamp that is in our [now, now+duration] time frame
        // - if we get a replaymessage with higher relative timestamp, put it in currentMessage and continue to next task
    }
}

} //end namespace tracing
} //end namespace ib

