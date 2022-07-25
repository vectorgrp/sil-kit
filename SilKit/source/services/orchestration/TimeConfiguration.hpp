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
