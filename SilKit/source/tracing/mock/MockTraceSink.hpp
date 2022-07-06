// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ITraceMessageSink.hpp"

namespace SilKit {
namespace Tests {


using namespace SilKit::Core;

class MockTraceSink : public ITraceMessageSink
{
public:
    MOCK_METHOD2(Open, void(SinkType type, const std::string& outputPath));
    MOCK_METHOD0(Close, void());

    //! \brief This works around TraceMessage not being copyable for use in Matchers
    void Trace(SilKit::Services::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const TraceMessage& message) override
    {
        switch (message.Type())
        {
        case TraceMessageType::CanFrameEvent:
            Trace(dir, address, timestamp, message.Get<SilKit::Services::Can::CanFrameEvent>());
            break;
        case TraceMessageType::EthernetFrame:
            Trace(dir, address, timestamp, message.Get<SilKit::Services::Ethernet::EthernetFrame>());
            break;
        case TraceMessageType::LinFrame:
            Trace(dir, address, timestamp, message.Get<SilKit::Services::Lin::LinFrame>());
            break;
        case TraceMessageType::FlexrayFrameEvent:
            Trace(dir, address, timestamp, message.Get<SilKit::Services::Flexray::FlexrayFrameEvent>());
            break;
        default:
            throw std::runtime_error("Invalid replay data");
        }
    }

    MOCK_METHOD4(Trace, void(SilKit::Services::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const SilKit::Services::Can::CanFrame& message));

    MOCK_METHOD4(Trace, void(SilKit::Services::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const SilKit::Services::Ethernet::EthernetFrame& message));

    MOCK_METHOD4(Trace, void(SilKit::Services::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const SilKit::Services::Lin::LinFrame& message));

    MOCK_METHOD4(Trace, void(SilKit::Services::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const SilKit::Services::PubSub::DataMessageEvent& message));

    MOCK_METHOD4(Trace, void(SilKit::Services::TransmitDirection dir, const EndpointAddress& address,
        std::chrono::nanoseconds timestamp, const SilKit::Services::Flexray::FlexrayFrameEvent& message));


    auto GetLogger() const -> Logging::ILogger* override
    {
        return nullptr;
    }
    auto Name() const -> const std::string& override
    {
        return mockName;
    }

    const std::string mockName{"MockTraceSink"};
};


} // namespace Tests
} // namespace SilKit
