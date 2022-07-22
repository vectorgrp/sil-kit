// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <string>
#include <chrono>
#include <map>
#include <mutex>


#include "SyncDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

using namespace std::chrono_literals;
class TimeConfiguration
{
public: //Ctor
    TimeConfiguration() ;
public: //Methods
    void SetBlockingMode(bool blocking);
    void SynchronizedParticipantAdded(const std::string& otherParticipantName);
    void OnReceiveNextSimStep(const std::string& participantName, NextSimTask nextStep);
    void SynchronizedParticipantRemoved(const std::string& otherParticipantName);
    void SetStepDuration(std::chrono::nanoseconds duration);
    void AdvanceTimeStep();
    auto CurrentSimStep() const -> NextSimTask;
    auto NextSimStep() const -> NextSimTask;
    bool OtherParticipantHasHigherTimepoint() const;
    void Initialize();
    bool IsBlocking() const;

private: //Members
    mutable std::mutex _mx;
    using Lock = std::unique_lock<decltype(_mx)>;
    NextSimTask _currentTask;
    NextSimTask _myNextTask;
    std::map<std::string, NextSimTask> _otherNextTasks;
    bool _blocking;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
