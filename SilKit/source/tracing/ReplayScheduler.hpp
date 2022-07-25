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

#pragma once

#include <chrono>
#include <vector>
#include <memory>

#include "ITimeProvider.hpp"
#include "silkit/participant/IParticipant.hpp"
#include "silkit/config/fwd_decl.hpp"

#include "IReplayDataController.hpp"

namespace SilKit {
namespace tracing {

class ReplayScheduler
{
public:
    ReplayScheduler(const Config::Config& config,  const Config::Participant& participantConfig,
        std::chrono::nanoseconds tickPeriod, IParticipant* participant, Services::Orchestration::ITimeProvider* timeProvider);
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
    Services::Logging::ILogger* _log{nullptr};
    IParticipant* _participant{nullptr};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    std::vector<ReplayTask> _replayTasks;
    bool _isDone{false};
    std::vector<std::string> _knownSimulators;
};
} //end namespace tracing
} //end namespace SilKit
