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

const std::string virtualTimeNS{"VirtualTimeNS"};
const std::string msg{"Msg"};
const std::string from{"From"};

const std::string waitingTime{"WaitingTime"};
const std::string executionTime{"ExecutionTime"};

const std::string participantName{"ParticipantName"};
const std::string registryUri{"RegistryUri"};
const std::string silKitVersion{"SilKitVersion"};

const std::string newParticipantState{"NewParticipantState"};
const std::string oldParticipantState{"OldParticipantState"};
const std::string enterTime{"EnterTime"};
const std::string enterReason{"EnterReason"};

const std::string serviceType{"ServiceType"};
const std::string serviceName{"ServiceName"};
const std::string networkType{"NetworkType"};
const std::string networkName{"NetworkName"};
const std::string controllerTypeName{"ControllerTypeName"};


const std::string controllerName{"ControllerName"};
const std::string controllerType{"ControllerType"};
const std::string pubSubTopic{"PubSubTopic"};
const std::string controllerFuncName{"ControllerFuncName"};
const std::string mediaType{"MediaType"};
const std::string network{"Network"};
const std::string label{"Label"};


} // namespace Keys
} // namespace Logging
} // namespace Services
} // namespace SilKit
