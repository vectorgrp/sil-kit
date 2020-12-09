// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>
#include <memory>

#include "ib/mw/sync/ITimeProvider.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/cfg/fwd_decl.hpp"

#include "IReplayDataController.hpp"

namespace ib {
namespace tracing {

class ReplayScheduler
{
public:
    ReplayScheduler(const cfg::Config& config,  const cfg::Participant& participantConfig,
        std::chrono::nanoseconds tickPeriod, mw::IComAdapter* comAdapter, mw::sync::ITimeProvider* timeProvider);

public:
    // Methods
    void StartReplay();
    void StopReplay();
private:
    // Methods
  
    void ConfigureControllers(const cfg::Config& config, const cfg::Participant& participantConfig);
    void ReplayMessages(std::chrono::nanoseconds now, std::chrono::nanoseconds duration);

private:
    // Members
    struct ReplayTask
    {
        std::string name;
        IReplayDataController* controller{nullptr};
        std::shared_ptr<extensions::IReplayChannelReader> replayReader;
        std::chrono::nanoseconds initialTime{0};
    };
    std::chrono::nanoseconds _tickPeriod{0};
    std::chrono::nanoseconds _startTime{std::chrono::nanoseconds::min()};
    bool _isStarted{false};
    mw::logging::ILogger* _log{nullptr};
    mw::IComAdapter* _comAdapter{nullptr};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    std::vector<ReplayTask> _replayTasks;
};
} //end namespace tracing
} //end namespace ib
