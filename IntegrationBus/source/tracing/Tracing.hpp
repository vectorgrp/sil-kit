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

#include "ib/extensions/ITraceMessageSink.hpp"

namespace ib {
namespace tracing {



using extensions::Direction;
using extensions::ITraceMessageSink;
using extensions::SinkType;
using extensions::TraceMessage;
using extensions::TraceMessageType;


// utilities 
std::ostream& operator<<(std::ostream& out, const TraceMessage&);
std::ostream& operator<<(std::ostream& out, const TraceMessageType&);
std::string to_string(const TraceMessageType&);
std::string to_string(const TraceMessage&);

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
        (Direction rxOrTx, std::chrono::nanoseconds timestamp, const MsgT& msg)
        {
            sink.Trace(rxOrTx, id, timestamp, msg);
        };
        _sinks.emplace_back(std::move(eventHandler));
    }

    bool IsActive() const
    {
        return  _sinks.size() > 0;
    }

    void Trace(Direction rxOrTx, std::chrono::nanoseconds timestamp, const MsgT& msg)
    {
        for (auto& sink: _sinks)
        {
            sink(rxOrTx, timestamp, msg);
        }
    }
private:
    std::vector<ActiveSinks> _sinks;
};

// Configure the comadapters tracing sinks based on the configuration, and add appropriate tracepoints to
// the controllers.
using RegistrationCallbackT = std::function<void(std::unique_ptr<ITraceMessageSink>)>;

void CreateTraceMessageSinks(
    mw::logging::ILogger* logger,
    const cfg::Config& config,
    const cfg::Participant& participantConfig,
    RegistrationCallbackT callback);

} //end namespace tracing
} //end namespace ib
