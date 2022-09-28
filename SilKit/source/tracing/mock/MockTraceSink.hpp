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
            throw SilKitError("Invalid replay data");
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


    auto GetLogger() const -> Services::Logging::ILogger* override
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
