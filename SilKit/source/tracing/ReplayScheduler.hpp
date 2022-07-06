// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>
#include <memory>

#include "ITimeProvider.hpp"
#include "silkit/core/IParticipant.hpp"
#include "silkit/cfg/fwd_decl.hpp"

#include "IReplayDataController.hpp"

namespace SilKit {
namespace tracing {

class ReplayScheduler
{
public:
    ReplayScheduler(const Config::Config& config,  const Config::Participant& participantConfig,
        std::chrono::nanoseconds tickPeriod, Core::IParticipant* participant, Core::Orchestration::ITimeProvider* timeProvider);
    ~ReplayScheduler();
    void ConfigureNetworkSimulators(const Config::Config& config, const Config::Participant& participantConfig,
        tracing::IReplayDataController& netsim);
private:
    // Methods
  
    void ConfigureControllers(const Config::Config& config, const Config::Participant& participantConfig);
    void ReplayMessages(std::chrono::nanoseconds now, std::chrono::nanoseconds duration);

private:
    // Members
    struct ReplayTask
    {
        std::shared_ptr<IReplayFile> replayFile;
        std::string name;
        IReplayDataController* controller{nullptr};
        std::shared_ptr<IReplayChannelReader> replayReader;
        std::chrono::nanoseconds initialTime{0};
        bool doneReplaying{false};
    };
    std::chrono::nanoseconds _tickPeriod{0};
    std::chrono::nanoseconds _startTime{std::chrono::nanoseconds::min()};
    Core::Logging::ILogger* _log{nullptr};
    Core::IParticipant* _participant{nullptr};
    Core::Orchestration::ITimeProvider* _timeProvider{nullptr};
    std::vector<ReplayTask> _replayTasks;
    bool _isDone{false};
    std::vector<std::string> _knownSimulators;
};
} //end namespace tracing
} //end namespace SilKit
