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
