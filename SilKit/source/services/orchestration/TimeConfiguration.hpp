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

    void SetDynamicStepSizeEnabled(bool enabled);
    bool IsDynamicStepSizeEnabled() const;

private: //Methods
    // Computes the minimal step duration that keeps this participant aligned with the earliest next
    // timepoint among all other synchronized participants. The caller must already hold _mx (this is
    // only invoked from AdvanceTimeStep); it deliberately does not lock so it stays reentrant there.
    auto GetMinimalAlignedDuration() const -> std::chrono::nanoseconds;

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
    // synchronized participants (see GetMinimalAlignedDuration / AdvanceTimeStep). Disabled by default;
    // can be turned on via Experimental.TimeSynchronization.DynamicSimulationStep.
    bool _dynamicStepSizeEnabled{false};
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
