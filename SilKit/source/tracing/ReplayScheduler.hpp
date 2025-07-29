// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <vector>
#include <memory>

#include "IParticipantInternal.hpp"
#include "ParticipantConfiguration.hpp"
#include "ITimeProvider.hpp"
#include "IReplayDataController.hpp"
#include "ISimulator.hpp"

namespace SilKit {
namespace Tracing {

class ReplayScheduler
{
public:
    ReplayScheduler(const Config::ParticipantConfiguration& participantConfig, Core::IParticipantInternal* participant);

    ~ReplayScheduler();

    void ConfigureTimeProvider(Services::Orchestration::ITimeProvider* timeProvider);

    void ConfigureController(const std::string& controllerName, IReplayDataController* controller,
                             const Config::Replay& replayConfig, const std::string& networkName,
                             Config::NetworkType networkType);

private:
    // Methods

    void CreateReplayFiles(const Config::ParticipantConfiguration& participantConfiguration);

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

    std::chrono::nanoseconds _startTime{std::chrono::nanoseconds::min()};
    Services::Logging::ILogger* _log{nullptr};
    Core::IParticipantInternal* _participant{nullptr};
    Services::Orchestration::ITimeProvider* _timeProvider{nullptr};
    std::vector<ReplayTask> _replayTasks;
    bool _isDone{false};
    std::vector<std::string> _knownSimulators;

    std::map<std::string, std::shared_ptr<IReplayFile>> _replayFiles;
};

} // namespace Tracing
} //end namespace SilKit
