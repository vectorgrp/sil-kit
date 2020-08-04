// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>
#include <functional>

#include "ib/mw/EndpointAddress.hpp"
#include "ib/extensions/TraceMessage.hpp"
#include "ib/extensions/ITraceMessageSink.hpp"

namespace ib {
namespace extensions {

// A ITraceMessageSource can be connected to number of ITraceMessageSinks.
// Usually a Tracer object is used in the implementation of a message source.
class ITraceMessageSource
{
public:
    virtual ~ITraceMessageSource() = default;

    // Adds the given sink to the list of active sinks.
    // Active sinks will receive traced messages.
    virtual void AddSink(ITraceMessageSink* sink) = 0;
};

class Tracer
{
public:
    Tracer() = default;

    // Add the given `sink` to the list trace sinks.
    // A function object is created that wraps the actual sink.Trace call.
    inline void AddSink(mw::EndpointAddress id, ITraceMessageSink& sink);

    //convenience wrapper converting concrete types into type-erased TraceMessage references
    template<typename MsgT>
    void Trace(Direction direction,
            std::chrono::nanoseconds timestamp,
            const MsgT& msg);

    // type erased Trace method
    inline void Trace(Direction direction,
            std::chrono::nanoseconds timestamp,
            const TraceMessage& msg);

private:
    //Adding a sink will create a function object wrapping the sink and a
    //call to its sink.Trace(msg) method.
    using ConnectedSink = std::function<void(Direction, std::chrono::nanoseconds, const TraceMessage&)>;
    std::vector<ConnectedSink> _sinks;
};

////////////////////////////////////////
// Inline Implementations
////////////////////////////////////////

void Tracer::AddSink(mw::EndpointAddress id, ITraceMessageSink& sink)
{
    auto sinkCallback = [id, &sink]
        (Direction direction, std::chrono::nanoseconds timestamp, const TraceMessage& msg)
        {
            sink.Trace(direction, id, timestamp, msg);
        };
    _sinks.emplace_back(std::move(sinkCallback));
}

template<typename MsgT>
void Tracer::Trace(Direction direction,
        std::chrono::nanoseconds timestamp,
        const MsgT& msg)
{
    Trace(direction, timestamp, TraceMessage(msg));
}

void Tracer::Trace(Direction direction,
        std::chrono::nanoseconds timestamp,
        const TraceMessage& msg)
{
    for (auto& sink: _sinks)
    {
        sink(direction, timestamp, msg);
    }
}

} //end namespace extensions
} //end namespace ib
