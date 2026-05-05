// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

namespace SilKit {
namespace Services {
namespace Logging {

/*! \brief Topic of a log message
 */
enum class Topic : uint32_t
{
    None = 0,             //!< Log message without topic
    User = 1,             //!< Log message of the SIL Kit user
    LifeCycle = 2,        //!< Log message of the lifecycle
    SystemState = 3,      //!< Log message of the system state
    MessageTracing = 4,   //!< Log message of the message tracing
    ServiceDiscovery = 5, //!< Log message of the service discovery
    Asio = 6,             //!< Log message of the asio
    TimeSync = 7,         //!< Log message of the time sync service
    Participant = 8,      //!< Log message of the participant
    TimeConfig = 9,
    RequestReply = 10,
    SystemMonitor = 11,
    Can = 12,
    Ethernet = 13,
    Flexray = 14,
    Lin = 15,
    Metrics = 16,
    Pubsub = 17,
    Rpc = 18,
    Tracing = 19,
    Dashboard = 20,
    NetSim = 21,
    Extension = 22,
    Invalid = 0xffffffff //!< Invalid log message topic
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
