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

#include <string>
#include <chrono>
#include <map>
#include <mutex>

#include "OrchestrationDatatypes.hpp"
#include "silkit/services/logging/ILogger.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

using namespace std::chrono_literals;
class TimeConfiguration
{
public: //Ctor
    TimeConfiguration(Logging::ILogger* logger);

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
    bool HandleHopOn();

private: //Members
    mutable std::mutex _mx;
    using Lock = std::unique_lock<decltype(_mx)>;
    NextSimTask _currentTask;
    NextSimTask _myNextTask;
    std::map<std::string, NextSimTask> _otherNextTasks;
    bool _blocking;

    bool _hoppedOn = false;
    Logging::ILogger* _logger;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
