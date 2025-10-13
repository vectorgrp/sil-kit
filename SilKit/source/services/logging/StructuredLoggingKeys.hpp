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

constexpr std::string_view virtualTimeNS{"VirtualTimeNS"};
constexpr std::string_view msg{"Msg"};
constexpr std::string_view from{"From"};
constexpr std::string_view to{"To"};
constexpr std::string_view raw{"Raw"};

constexpr std::string_view waitingTime{"WaitingTime"};
constexpr std::string_view executionTime{"ExecutionTime"};

constexpr std::string_view participantName{"ParticipantName"};
constexpr std::string_view registryUri{"RegistryUri"};
constexpr std::string_view silKitVersion{"SilKitVersion"};

constexpr std::string_view newParticipantState{"NewParticipantState"};
constexpr std::string_view oldParticipantState{"OldParticipantState"};
constexpr std::string_view enterTime{"EnterTime"};
constexpr std::string_view enterReason{"EnterReason"};

constexpr std::string_view serviceType{"ServiceType"};
constexpr std::string_view serviceName{"ServiceName"};
constexpr std::string_view networkType{"NetworkType"};
constexpr std::string_view networkName{"NetworkName"};
constexpr std::string_view controllerTypeName{"ControllerTypeName"};


constexpr std::string_view controllerName{"ControllerName"};
constexpr std::string_view controllerType{"ControllerType"};
constexpr std::string_view pubSubTopic{"PubSubTopic"};
constexpr std::string_view controllerFuncName{"ControllerFuncName"};
constexpr std::string_view mediaType{"MediaType"};
constexpr std::string_view network{"Network"};
constexpr std::string_view label{"Label"};


} // namespace Keys
} // namespace Logging
} // namespace Services
} // namespace SilKit
