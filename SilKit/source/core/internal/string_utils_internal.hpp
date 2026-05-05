// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <iostream>
#include <sstream>

#include "EndpointAddress.hpp"


#include <ostream>
#include <string>
#include <algorithm>
#include <cctype>

#include "LoggingTopics.hpp"
#include "StringHelpers.hpp"

namespace SilKit {
namespace Core {
inline std::string to_string(const SilKit::Core::EndpointAddress& address);
inline std::ostream& operator<<(std::ostream& out, const SilKit::Core::EndpointAddress& address);
}
}

namespace SilKit {
namespace Services {
namespace Logging {

inline std::string to_string(const Topic& topic);
inline Topic from_topic_string(const std::string& topicStr);
inline std::ostream& operator<<(std::ostream& out, const Topic& topic);

} // namespace Logging
} // namespace Services
}

// ================================================================================
//  Inline Implementations
// ================================================================================

namespace SilKit {
namespace Core {
std::string to_string(const SilKit::Core::EndpointAddress& address)
{
    std::stringstream outStream;
    outStream << address;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const SilKit::Core::EndpointAddress& address)
{
    out << "Addr{" << address.participant << ", " << address.endpoint << "}";
    return out;
}

} // namespace Core
} // namespace SilKit

namespace SilKit {
namespace Services {
namespace Logging {

std::string to_string(const Topic& topic)
{
    std::stringstream outStream;
    outStream << topic;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& outStream, const Topic& topic)
{
    switch (topic)
    {
    case Topic::User:
        outStream << "User";
        break;
    case Topic::TimeSync:
        outStream << "TimeSync";
        break;
    case Topic::LifeCycle:
        outStream << "LifeCycle";
        break;
    case Topic::SystemState:
        outStream << "SystemState";
        break;
    case Topic::MessageTracing:
        outStream << "MessageTracing";
        break;
    case Topic::ServiceDiscovery:
        outStream << "ServiceDiscovery";
        break;
    case Topic::Participant:
        outStream << "Participant";
        break;
    case Topic::Asio:
        outStream << "Asio";
        break;
    case Topic::TimeConfig:
        outStream << "TimeConfig";
        break;
    case Topic::RequestReply:
        outStream << "RequestReply";
        break;
    case Topic::SystemMonitor:
        outStream << "SystemMonitor";
        break;
    case Topic::Can:
        outStream << "Can";
        break;
    case Topic::Ethernet:
        outStream << "Ethernet";
        break;
    case Topic::Flexray:
        outStream << "Flexray";
        break;
    case Topic::Lin:
        outStream << "Lin";
        break;
    case Topic::Metrics:
        outStream << "Metrics";
        break;
    case Topic::Pubsub:
        outStream << "Pubsub";
        break;
    case Topic::Rpc:
        outStream << "Rpc";
        break;
    case Topic::Tracing:
        outStream << "Tracing";
        break;
    case Topic::Dashboard:
        outStream << "Dashboard";
        break;
    case Topic::NetSim:
        outStream << "NetSim";
        break;
    case Topic::Extension:
        outStream << "Extension";
        break;
    case Topic::None:
        outStream << "None";
        break;
    default:
        outStream << "Invalid";
    }
    return outStream;
}

inline Topic from_topic_string(const std::string& topicStr)
{
    auto topic = SilKit::Util::LowerCase(topicStr);
    if (topic == "none")
        return Topic::None;
    if (topic == "user")
        return Topic::User;
    if (topic == "timesync")
        return Topic::TimeSync;
    if (topic == "lifecycle")
        return Topic::LifeCycle;
    if (topic == "systemstate")
        return Topic::SystemState;
    if (topic == "messagetracing")
        return Topic::MessageTracing;
    if (topic == "servicediscovery")
        return Topic::ServiceDiscovery;
    if (topic == "participant")
        return Topic::Participant;
    if (topic == "timeconfig")
        return Topic::TimeConfig;
    if (topic == "asio")
        return Topic::Asio;
    if (topic == "requestreply")
        return Topic::RequestReply;
    if (topic == "systemmonitor")
        return Topic::SystemMonitor;
    if (topic == "can")
        return Topic::Can;
    if (topic == "ethernet")
        return Topic::Ethernet;
    if (topic == "flexray")
        return Topic::Flexray;
    if (topic == "lin")
        return Topic::Lin;
    if (topic == "metrics")
        return Topic::Metrics;
    if (topic == "pubsub")
        return Topic::Pubsub;
    if (topic == "rpc")
        return Topic::Rpc;
    if (topic == "tracing")
        return Topic::Tracing;
    if (topic == "dashboard")
        return Topic::Dashboard;
    if (topic == "netsim")
        return Topic::NetSim;
    if (topic == "extension")
        return Topic::Extension;
    return Topic::Invalid;
}

} // namespace Logging
} // namespace Services
} // namespace SilKit
