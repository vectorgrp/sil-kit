// Copyright (c) Vector Informatik GmbH. All rights reserved.


#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/mw/EndpointAddress.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

#include "MockComAdapter.hpp"

namespace ib {
namespace sim {
namespace lin {

class ILinController;

namespace test {

class LinMockComAdapter : public mw::test::DummyComAdapter
{
public:
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const SendFrameRequest&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const SendFrameHeaderRequest&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const Transmission&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const FrameResponseUpdate&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const ControllerConfig&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const ControllerStatusUpdate&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const WakeupPulse&));
};

inline auto MakeControllerConfig(ControllerMode mode) -> ControllerConfig
{
    ControllerConfig config;
    config.controllerMode = mode;
    return config;
}

inline auto MakeFrame(LinIdT linId, ChecksumModel checksumModel = ChecksumModel::Undefined, uint8_t dataLength = 0, std::array<uint8_t, 8> data = std::array<uint8_t, 8>{}) -> Frame
{
    Frame frame;
    frame.id = linId;
    frame.checksumModel = checksumModel;
    frame.dataLength = dataLength;
    frame.data = data;
    return frame;
}
inline auto AFrameWithId(LinIdT linId) -> testing::Matcher<const Frame&>
{
    using namespace testing;
    return Field(&Frame::id, linId);
}
inline auto ATransmissionWith(Frame frame) -> testing::Matcher<const Transmission&>
{
    using namespace testing;
    return Field(&Transmission::frame, frame);
}
inline auto ATransmissionWith(FrameStatus status) -> testing::Matcher<const Transmission&>
{
    using namespace testing;
    return Field(&Transmission::status, status);
}
inline auto ATransmissionWith(FrameStatus status, std::chrono::nanoseconds timestamp) -> testing::Matcher<const Transmission&>
{
    using namespace testing;
    return AllOf(
        Field(&Transmission::status, status),
        Field(&Transmission::timestamp, timestamp)
    );
}
inline auto ATransmissionWith(Frame frame, FrameStatus status) -> testing::Matcher<const Transmission&>
{
    using namespace testing;
    return AllOf(
        Field(&Transmission::frame, frame),
        Field(&Transmission::status, status)
    );
}
inline auto ATransmissionWith(Frame frame, FrameStatus status, std::chrono::nanoseconds timestamp) -> testing::Matcher<const Transmission&>
{
    using namespace testing;
    return AllOf(
        Field(&Transmission::frame, frame),
        Field(&Transmission::status, status),
        Field(&Transmission::timestamp, timestamp)
    );
}

inline auto AControllerStatusUpdateWith(ControllerStatus status) -> testing::Matcher<const ControllerStatusUpdate&>
{
    using namespace testing;
    return Field(&ControllerStatusUpdate::status, status);
}

struct Callbacks
{
    MOCK_METHOD3(FrameStatusHandler, void(ILinController*, const Frame&, FrameStatus));
    MOCK_METHOD1(GoToSleepHandler, void(ILinController*));
    MOCK_METHOD1(WakeupHandler, void(ILinController*));
    MOCK_METHOD3(FrameResponseUpdateHandler, void(ILinController*, ib::mw::EndpointAddress, const FrameResponse&));
};


} // namespace test
} // namespace lin
} // namespace sim
} // namespace ib

