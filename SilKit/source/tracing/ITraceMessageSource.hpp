// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <vector>
#include <functional>

#include "ITraceMessageSink.hpp"

#include "EndpointAddress.hpp"
#include "TraceMessage.hpp"
#include "Configuration.hpp"

namespace SilKit {

// A ITraceMessageSource can be connected to number of ITraceMessageSinks.
// Usually a Tracer object is used in the implementation of a message source.
class ITraceMessageSource
{
public:
    virtual ~ITraceMessageSource() = default;

    // Adds the given sink to the list of active sinks.
    // Active sinks will receive traced messages.
    virtual void AddSink(ITraceMessageSink* sink, Config::NetworkType networkType) = 0;
};

class Tracer
{
public:
    Tracer() = default;

    // Add the given `sink` to the list trace sinks.
    // A function object is created that wraps the actual sink.Trace call.
    inline void AddSink(const Core::ServiceDescriptor& serviceDescr, ITraceMessageSink& sink);

    //convenience wrapper converting concrete types into type-erased TraceMessage references
    template <typename MsgT>
    void Trace(SilKit::Services::TransmitDirection direction, std::chrono::nanoseconds timestamp, const MsgT& msg);

    // type erased Trace method
    inline void Trace(SilKit::Services::TransmitDirection direction, std::chrono::nanoseconds timestamp,
                      const TraceMessage& msg);

private:
    //Adding a sink will create a function object wrapping the sink and a
    //call to its sink.Trace(msg) method.
    using ConnectedSink =
        std::function<void(SilKit::Services::TransmitDirection, std::chrono::nanoseconds, const TraceMessage&)>;
    std::vector<ConnectedSink> _sinks;
};

////////////////////////////////////////
// Inline Implementations
////////////////////////////////////////

void Tracer::AddSink(const Core::ServiceDescriptor& serviceDescr, ITraceMessageSink& sink)
{
    auto sinkCallback = [serviceDescr, &sink](SilKit::Services::TransmitDirection direction,
                                              std::chrono::nanoseconds timestamp, const TraceMessage& msg) {
        sink.Trace(direction, serviceDescr, timestamp, msg);
    };
    _sinks.emplace_back(std::move(sinkCallback));
}

template <typename MsgT>
void Tracer::Trace(SilKit::Services::TransmitDirection direction, std::chrono::nanoseconds timestamp, const MsgT& msg)
{
    Trace(direction, timestamp, TraceMessage(msg));
}

void Tracer::Trace(SilKit::Services::TransmitDirection direction, std::chrono::nanoseconds timestamp,
                   const TraceMessage& msg)
{
    for (auto& sink : _sinks)
    {
        sink(direction, timestamp, msg);
    }
}

} // namespace SilKit
