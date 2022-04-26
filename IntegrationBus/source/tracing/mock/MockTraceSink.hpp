// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ITraceMessageSink.hpp"

namespace ib {
namespace test {

using namespace ib::extensions;
using namespace ib::mw;

class MockTraceSink : public ITraceMessageSink
{
public:
    MOCK_METHOD2(Open, void(SinkType type, const std::string& outputPath));
    MOCK_METHOD0(Close, void());

    //! \brief This works around TraceMessage not being copyable for use in Matchers
    void Trace(ib::sim::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const TraceMessage& message) override
    {
        switch (message.Type())
        {
        case TraceMessageType::CanFrameEvent:
            Trace(dir, address, timestamp, message.Get<ib::sim::can::CanFrameEvent>());
            break;
        case TraceMessageType::EthernetFrame:
            Trace(dir, address, timestamp, message.Get<ib::sim::eth::EthernetFrame>());
            break;
        case TraceMessageType::LinFrame:
            Trace(dir, address, timestamp, message.Get<ib::sim::lin::Frame>());
            break;
        case TraceMessageType::FrMessage:
            Trace(dir, address, timestamp, message.Get<ib::sim::fr::FrMessage>());
            break;
        default:
            throw std::runtime_error("Invalid replay data");
        }
    }

    MOCK_METHOD4(Trace, void(ib::sim::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const ib::sim::can::CanFrame& message));

    MOCK_METHOD4(Trace, void(ib::sim::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const ib::sim::eth::EthernetFrame& message));

    MOCK_METHOD4(Trace, void(ib::sim::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const ib::sim::lin::Frame& message));

    MOCK_METHOD4(Trace, void(ib::sim::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const ib::sim::data::DataMessageEvent& message));

    MOCK_METHOD4(Trace, void(ib::sim::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const ib::sim::fr::FrMessage& message));


    auto GetLogger() const -> logging::ILogger* override
    {
        return nullptr;
    }
    auto Name() const -> const std::string& override
    {
        return mockName;
    }

    const std::string mockName{"MockTraceSink"};
};


} // namespace test
} // namespace ib
