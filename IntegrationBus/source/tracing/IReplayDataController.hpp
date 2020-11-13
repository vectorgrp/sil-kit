// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include "ib/cfg/Config.hpp"
#include "ib/extensions/IReplay.hpp"

namespace ib {
namespace tracing {
class IReplayDataController
{
public:
    virtual ~IReplayDataController() = default;


    //! \brief Configure the controller with the given replay configuration.
    virtual void Configure(const cfg::Replay& replayConfig) = 0;

    //! \brief Replay the given message.
    // The controller is responsible for converting the replay message into a
    // concrete type, e.g. sim::eth::EthFrame.
    virtual void ReplayMessage(const extensions::IReplayMessage& message) = 0;
};
} //end namespace tracing
} //end namespace ib
