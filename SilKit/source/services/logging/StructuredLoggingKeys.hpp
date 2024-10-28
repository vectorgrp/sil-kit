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

#include <atomic>


#include <string>



namespace SilKit {
namespace Services {
namespace Logging {
namespace Keys {

const std::string VIRTUAL_TIME_NS{"VirtualTimeNS"};
const std::string MSG{"msg"};
const std::string FROM{"From"};

const std::string WAITING_TIME{"WaitingTime"};
const std::string EXECUTION_TIME{"ExecutionTime"};

const std::string PARTICIPANT_NAME{"ParticipantName"};
const std::string NEW_PARTICIPANT_STATE{"NewParticipantState"};
const std::string OLD_PARTICIPANT_STATE{"OldParticipantState"};
const std::string ENTER_TIME{"EnterTime"};
const std::string ENTER_REASON{"EnterReason"};




} // namespace Keys
} // namespace Logging
} // namespace Services
} // namespace SilKit
