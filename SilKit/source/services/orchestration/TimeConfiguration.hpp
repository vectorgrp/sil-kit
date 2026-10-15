// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <chrono>
#include <map>
#include <mutex>
#include <set>

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

    //! How far this participant may run ahead of the last time point announced by any other
    //! participant (Experimental.TimeSynchronization.MaxRunAhead). Zero is the classic lockstep.
    void SetMaxRunAhead(std::chrono::nanoseconds maxRunAhead);
    //! The configured run-ahead, or zero while dynamic step sizes are active (not combined yet).
    auto EffectiveMaxRunAhead() const -> std::chrono::nanoseconds;

    //! Exact mode (Experimental.TimeSynchronization.Latency and NetworkLatencies): messages received on a link
    //! are due its latency after they were sent. A step [T, T+d) may start once every peer's progress plus the
    //! latency of its messages reaches T+d, so all messages due in the step have arrived.
    void SetLatencies(std::chrono::nanoseconds latency, std::map<std::string, std::chrono::nanoseconds> perNetwork);
    //! Latency of the messages received on a link, or zero outside exact mode.
    auto LatencyOf(const std::string& linkName) const -> std::chrono::nanoseconds;
    bool IsExactModeEnabled() const;
    //! The smallest configured latency, for diagnostics.
    auto MinimalLatency() const -> std::chrono::nanoseconds;
    //! Records a network a peer has a (sending) service on. Returns true if it was new.
    bool AddPeerNetwork(const std::string& participantName, const std::string& network);
    //! Raises what is known about a peer's progress from the send time of one of its messages. Returns true if
    //! the bound rose.
    bool RaiseSendTimeBound(const std::string& participantName, std::chrono::nanoseconds sendTime);

    //! Whether our next step must be announced to the other participants, given the time point we last announced.
    //! Without a window (run-ahead or latency) every step is announced. Otherwise announcements are thinned out:
    //! they are sent when our time has moved on by half the smallest window, and whenever we are about to block.
    //! The latter keeps the distributed algorithm deadlock-free.
    bool ShouldAnnounceNextSimStep(std::chrono::nanoseconds lastAnnouncedTimePoint) const;

private: //Methods
    auto EffectiveMaxRunAheadUnlocked() const -> std::chrono::nanoseconds;
    bool OtherParticipantHasLowerTimepointUnlocked() const;
    auto LatencyOfUnlocked(const std::string& linkName) const -> std::chrono::nanoseconds;
    bool IsExactModeEnabledUnlocked() const;
    //! The latency of the messages this participant receives from a peer: the smallest latency of the networks the
    //! peer has services on, or the global latency if none are known.
    auto PeerLatencyUnlocked(const std::string& participantName) const -> std::chrono::nanoseconds;
    auto PeerProgressUnlocked(const std::string& participantName, const NextSimTask& announced) const
        -> std::chrono::nanoseconds;

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

    std::chrono::nanoseconds _maxRunAhead{0};

    std::chrono::nanoseconds _latency{0};
    std::map<std::string, std::chrono::nanoseconds> _networkLatencies;
    std::map<std::string, std::set<std::string>> _peerNetworks;
    // PeerLatencyUnlocked is evaluated for every peer several times per step; cleared when its inputs change
    mutable std::map<std::string, std::chrono::nanoseconds> _peerLatencyCache;
    // Lower bounds on the send times of peers' future messages, learned from the messages themselves
    std::map<std::string, std::chrono::nanoseconds> _sendTimeBounds;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
