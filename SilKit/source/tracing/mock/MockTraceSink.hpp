// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "ITraceMessageSink.hpp"

namespace SilKit {
namespace Tests {


using namespace SilKit::Core;

class MockTraceSink : public ITraceMessageSink
{
public:
    MOCK_METHOD(void, Open, (SinkType type, const std::string& outputPath), (override));
    MOCK_METHOD(void, Close, (), (override));
    MOCK_METHOD(void, Trace,
                (SilKit::Services::TransmitDirection dir, const ServiceDescriptor& address,
                 std::chrono::nanoseconds timestamp, const SilKit::Services::Can::CanFrameEvent& message));
    MOCK_METHOD(void, Trace,
                (SilKit::Services::TransmitDirection dir, const ServiceDescriptor& address,
                 std::chrono::nanoseconds timestamp, const SilKit::Services::Ethernet::EthernetFrame& message));
    MOCK_METHOD(void, Trace,
                (SilKit::Services::TransmitDirection dir, const ServiceDescriptor& address,
                 std::chrono::nanoseconds timestamp, const SilKit::Services::Lin::LinFrame& message));
    MOCK_METHOD(void, Trace,
                (SilKit::Services::TransmitDirection dir, const ServiceDescriptor& address,
                 std::chrono::nanoseconds timestamp, const SilKit::Services::PubSub::DataMessageEvent& message));
    MOCK_METHOD(void, Trace,
                (SilKit::Services::TransmitDirection dir, const ServiceDescriptor& address,
                 std::chrono::nanoseconds timestamp, const SilKit::Services::Flexray::FlexrayFrameEvent& message));

    //! \brief This works around TraceMessage not being copyable for use in Matchers
    void Trace(SilKit::Services::TransmitDirection dir, const ServiceDescriptor& address,
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
