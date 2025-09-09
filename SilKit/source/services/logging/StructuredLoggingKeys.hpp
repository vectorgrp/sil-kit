// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <string>
#include <string_view>

namespace SilKit {
namespace Services {
namespace Logging {
namespace Keys {

const std::string virtualTimeNS{"VirtualTimeNS"};
const std::string msg{"Msg"};
const std::string from{"From"};
constexpr std::string_view to{"To"};

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
