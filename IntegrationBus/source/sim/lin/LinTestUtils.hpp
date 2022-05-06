// Copyright (c) Vector Informatik GmbH. All rights reserved.


#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/sim/lin/LinDatatypes.hpp"

#include "EndpointAddress.hpp"
#include "MockParticipant.hpp"

namespace ib {
namespace sim {
namespace lin {

class ILinController;

namespace test {

class LinMockParticipant : public mw::test::DummyParticipant
{
public:
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const LinSendFrameRequest&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const LinSendFrameHeaderRequest&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const LinTransmission&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const LinFrameResponseUpdate&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const LinControllerConfig&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const LinControllerStatusUpdate&));
    MOCK_METHOD2(SendIbMessage, void(const mw::IIbServiceEndpoint*, const LinWakeupPulse&));
};

inline auto MakeControllerConfig(LinControllerMode mode) -> LinControllerConfig
{
    LinControllerConfig config;
    config.controllerMode = mode;
    return config;
}

inline auto MakeFrame(LinIdT linId, LinChecksumModel checksumModel = LinChecksumModel::Undefined, uint8_t dataLength = 0, std::array<uint8_t, 8> data = std::array<uint8_t, 8>{}) -> LinFrame
{
    LinFrame frame;
    frame.id = linId;
    frame.checksumModel = checksumModel;
    frame.dataLength = dataLength;
    frame.data = data;
    return frame;
}
inline auto AFrameWithId(LinIdT linId) -> testing::Matcher<const LinFrame&>
{
    using namespace testing;
    return Field(&LinFrame::id, linId);
}
inline auto ATransmissionWith(LinFrame frame) -> testing::Matcher<const LinTransmission&>
{
    using namespace testing;
    return Field(&LinTransmission::frame, frame);
}
inline auto ATransmissionWith(LinFrameStatus status) -> testing::Matcher<const LinTransmission&>
{
    using namespace testing;
    return Field(&LinTransmission::status, status);
}
inline auto ATransmissionWith(LinFrameStatus status, std::chrono::nanoseconds timestamp) -> testing::Matcher<const LinTransmission&>
{
    using namespace testing;
    return AllOf(
        Field(&LinTransmission::status, status),
        Field(&LinTransmission::timestamp, timestamp)
    );
}
inline auto ATransmissionWith(LinFrame frame, LinFrameStatus status) -> testing::Matcher<const LinTransmission&>
{
    using namespace testing;
    return AllOf(
        Field(&LinTransmission::frame, frame),
        Field(&LinTransmission::status, status)
    );
}
inline auto ATransmissionWith(LinFrame frame, LinFrameStatus status, std::chrono::nanoseconds timestamp) -> testing::Matcher<const LinTransmission&>
{
    using namespace testing;
    return AllOf(
        Field(&LinTransmission::frame, frame),
        Field(&LinTransmission::status, status),
        Field(&LinTransmission::timestamp, timestamp)
    );
}

inline auto AControllerStatusUpdateWith(LinControllerStatus status) -> testing::Matcher<const LinControllerStatusUpdate&>
{
    using namespace testing;
    return Field(&LinControllerStatusUpdate::status, status);
}

struct Callbacks
{
    MOCK_METHOD3(FrameStatusHandler, void(ILinController*, const LinFrame&, LinFrameStatus));
    MOCK_METHOD1(GoToSleepHandler, void(ILinController*));
    MOCK_METHOD1(WakeupHandler, void(ILinController*));
    MOCK_METHOD3(FrameResponseUpdateHandler, void(ILinController*, const std::string&, const LinFrameResponse&));
};


} // namespace test
} // namespace lin
} // namespace sim
} // namespace ib

