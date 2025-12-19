// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <chrono>
#include <map>
#include <mutex>

#include "core/internal/OrchestrationDatatypes.hpp"
#include "services/logging/LoggerMessage.hpp"

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

    void SetStepDuration(std::chrono::nanoseconds duration);
    auto GetMinimalAlignedDuration() const -> std::chrono::nanoseconds;

    void SetDynamicStepSizeEnabled(bool enabled);

private: //Members
    mutable std::mutex _mx;
    using Lock = std::unique_lock<decltype(_mx)>;
    NextSimTask _currentTask;
    NextSimTask _myNextTask;
    std::map<std::string, NextSimTask> _otherNextTasks;
    bool _blocking;

    bool _hoppedOn = false;
    Logging::ILoggerInternal* _logger;

    // When enabled, each simulation step is shortened ("aligned") to the minimal duration among all
    // synchronized participants (see GetMinimalAlignedDuration / AdvanceTimeStep). Enabled by default
    // via participant configuration; can be turned off with Experimental.TimeSynchronization.DynamicSimulationStep.
    bool _dynamicStepSizeEnabled{false};
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
