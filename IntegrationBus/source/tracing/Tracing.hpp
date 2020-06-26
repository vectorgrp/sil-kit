// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <functional>
#include <vector>
#include <memory>
#include <ostream>

#include "ib/sim/fwd_decl.hpp"
#include "ib/mw/EndpointAddress.hpp"

#include "ib/cfg/fwd_decl.hpp"
#include "ib/mw/logging/fwd_decl.hpp"

#include "ib/extensions/ExtensionHandle.hpp"
#include "ib/extensions/ITraceMessageSink.hpp"

namespace ib {
namespace tracing {



using extensions::Direction;
using extensions::ITraceMessageSink;
using extensions::SinkType;
using extensions::TraceMessage;
using extensions::TraceMessageType;



// This interface allows attaching trace sinks to a controller, as configured by the user
// and constructed in the ComAdapter.
class IControllerToTraceSink
{
public:
    virtual ~IControllerToTraceSink() = default;
    virtual void AddSink(ITraceMessageSink*) = 0;
};

//! \brief Tracer is used in the service controllers to collect a
//         number of trace sinks, allowing to distribute trace messages
//         in a 1-Controller to N-Sinks fashion.
//  The traced message `msg` has to remain valid during the Trace() call.
template<typename MsgT>
class Tracer
{
public:
    using ActiveSinks = std::function<void(Direction, std::chrono::nanoseconds, const MsgT&)>;

    //! \brief Enable the current tracer for the message sink
    void AddSink(const mw::EndpointAddress& id, ITraceMessageSink& sink)
    {
        auto eventHandler = [id, &sink]
        (Direction direction, std::chrono::nanoseconds timestamp, const MsgT& msg)
        {
            sink.Trace(direction, id, timestamp, msg);
        };
        _sinks.emplace_back(std::move(eventHandler));
    }

    void Trace(Direction direction, std::chrono::nanoseconds timestamp, const MsgT& msg)
    {
        for (auto& sink: _sinks)
        {
            sink(direction, timestamp, msg);
        }
    }
private:
    std::vector<ActiveSinks> _sinks;
};

// Configure the trace sinks based on the configuration and return a vector of
// the sinks

auto CreateTraceMessageSinks(
    mw::logging::ILogger* logger,
    const cfg::Config& config,
    const cfg::Participant& participantConfig
    ) -> std::vector<extensions::ExtensionHandle<ITraceMessageSink>>;

} //end namespace tracing
} //end namespace ib
