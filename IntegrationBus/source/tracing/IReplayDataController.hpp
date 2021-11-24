// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include "ib/cfg/Config.hpp"
#include "ib/extensions/IReplay.hpp"
#include "ib/mw/EndpointAddress.hpp"
#include "IIbServiceEndpoint.hpp"

#include <limits>

namespace ib {
namespace tracing {

//!< Helper to check whether Direction `dir` is active in the config 
inline bool IsReplayEnabledFor(const cfg::Replay& cfg, cfg::Replay::Direction dir)
{
    return cfg.direction == dir
        || cfg.direction == cfg::Replay::Direction::Both;
}

//!< For replaying in the receive path we use an unlikely EndpointAddress
inline auto ReplayEndpointAddress() -> ib::mw::EndpointAddress
{
    return {
        std::numeric_limits<decltype(ib::mw::EndpointAddress::participant)>::max(),
        std::numeric_limits<decltype(ib::mw::EndpointAddress::endpoint)>::max()
    };
}

struct ReplayServiceId : public mw::IIbServiceEndpoint
{
    void SetServiceId(const mw::ServiceId& ) override
    {
    }
    auto GetServiceId() const -> const mw::ServiceId& override
    {
        static mw::ServiceId id;
        id.legacyEpa = ReplayEndpointAddress();
        return id;
    }
};

class IReplayDataController
{
public:
    virtual ~IReplayDataController() = default;

    //! \brief Replay the given message.
    // The controller is responsible for converting the replay message into a
    // concrete type, e.g. sim::eth::EthFrame.
    virtual void ReplayMessage(const extensions::IReplayMessage* message) = 0;
};
} //end namespace tracing
} //end namespace ib
