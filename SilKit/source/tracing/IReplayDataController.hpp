// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IServiceEndpoint.hpp"

#include "Configuration.hpp"
#include "IReplay.hpp"
#include "EndpointAddress.hpp"

#include <limits>

namespace SilKit {
namespace Tracing {

//! Helper to check whether Direction `dir` is active in the config
inline bool IsReplayEnabledFor(const Config::Replay& cfg, Config::Replay::Direction dir)
{
    if (cfg.direction == Config::Replay::Direction::Undefined)
    {
        return false;
    }
    return cfg.direction == dir || cfg.direction == Config::Replay::Direction::Both;
}

//! For replaying in the receive path we use an unlikely EndpointAddress
inline auto ReplayEndpointAddress() -> SilKit::Core::EndpointAddress
{
    return {std::numeric_limits<decltype(SilKit::Core::EndpointAddress::participant)>::max(),
            std::numeric_limits<decltype(SilKit::Core::EndpointAddress::endpoint)>::max()};
}

struct ReplayServiceDescriptor : public Core::IServiceEndpoint
{
    void SetServiceDescriptor(const Core::ServiceDescriptor&) override {}
    auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override
    {
        static Core::ServiceDescriptor id;
        id.SetParticipantNameAndComputeId("__!!Replay");
        id.SetServiceName("ReplayController");
        id.SetServiceId(ReplayEndpointAddress().endpoint);
        return id;
    }
};

class IReplayDataController
{
public:
    virtual ~IReplayDataController() = default;

    //! \brief Replay the given message.
    // The controller is responsible for converting the replay message into a
    // concrete type, e.g. Services::Ethernet::EthernetFrame.
    virtual void ReplayMessage(const IReplayMessage* message) = 0;
};

} // namespace Tracing
} // namespace SilKit
