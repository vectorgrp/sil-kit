// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <chrono>
#include <map>
#include <mutex>

#include "OrchestrationDatatypes.hpp"
#include "LoggerMessage.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

using namespace std::chrono_literals;
class TimeConfiguration
{
public: //Ctor
    TimeConfiguration(Logging::ILoggerInternal* logger);

public: //Methods
    void SetBlockingMode(bool blocking);
    void AddSynchronizedParticipant(const std::string& otherParticipantName);
    bool RemoveSynchronizedParticipant(const std::string& otherParticipantName);
    auto GetSynchronizedParticipantNames() -> std::vector<std::string>;
    void OnReceiveNextSimStep(const std::string& participantName, NextSimTask nextStep);
    void SynchronizedParticipantRemoved(const std::string& otherParticipantName);
    void SetStepDuration(std::chrono::nanoseconds duration);
    void AdvanceTimeStep();
    auto CurrentSimStep() const -> NextSimTask;
    auto NextSimStep() const -> NextSimTask;
    bool OtherParticipantHasLowerTimepoint() const;
    void Initialize();
    bool IsBlocking() const;

    bool ShouldResendNextSimStep();

    // Returns true (only once) in the step the actual hop-on happened
    bool IsHopOn();
    bool HoppedOn();

private: //Members
    mutable std::mutex _mx;
    using Lock = std::unique_lock<decltype(_mx)>;
    NextSimTask _currentTask;
    NextSimTask _myNextTask;
    std::map<std::string, NextSimTask> _otherNextTasks;
    bool _blocking;

    bool _hoppedOn = false;
    Logging::ILoggerInternal* _logger;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
