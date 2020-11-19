// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>
#include <memory>

#include "ib/mw/sync/ITimeProvider.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/cfg/fwd_decl.hpp"

#include "ReplayController.hpp"

namespace ib {
namespace tracing {

struct ReplayTask
{
    ReplayController* controller{nullptr}; //!< the controller with enabled replay, owned by comAdapter.
    std::chrono::nanoseconds lastTimestamp{0}; //!< the time stamp of the last replay message.
    std::shared_ptr<extensions::IReplayChannel> replayChannel; //!< source of replay messages
};

class ReplayScheduler
{
public:
    ReplayScheduler(const cfg::Config& config,  const cfg::Participant& participantConfig,
        std::chrono::nanoseconds tickPeriod, mw::IComAdapter* comAdapter, mw::sync::ITimeProvider* timeProvider);
    ~ReplayScheduler() = default;

private:
    // Methods
   
    void ConfigureControllers(const cfg::Config& config, const cfg::Participant& participantConfig);

    //! \brief Replay messages the given duration (now, duration).
    void ReplayMessages(std::chrono::nanoseconds now, std::chrono::nanoseconds duration);
private:
    // Members
    struct ReplayTask
    {
        ReplayController* controller{nullptr};
        std::shared_ptr<extensions::IReplayChannel> replayChannel;
        std::chrono::nanoseconds initialTime{0};
    };
    std::chrono::nanoseconds _tickPeriod{0};
    mw::IComAdapter* _comAdapter{nullptr};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    std::vector<ReplayTask> _replayTasks;
};
} //end namespace tracing
} //end namespace ib
