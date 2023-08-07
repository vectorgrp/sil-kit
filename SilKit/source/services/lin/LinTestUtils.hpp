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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/services/lin/LinDatatypes.hpp"

#include "EndpointAddress.hpp"
#include "MockParticipant.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

class ILinController;

namespace Tests {

class LinMockParticipant : public Core::Tests::DummyParticipant
{
public:
    MOCK_METHOD(void, SendMsg, (const Core::IServiceEndpoint*, const LinSendFrameRequest&));
    MOCK_METHOD(void, SendMsg, (const Core::IServiceEndpoint*, const LinSendFrameHeaderRequest&));
    MOCK_METHOD(void, SendMsg, (const Core::IServiceEndpoint*, const LinTransmission&));
    MOCK_METHOD(void, SendMsg, (const Core::IServiceEndpoint*, const LinFrameResponseUpdate&));
    MOCK_METHOD(void, SendMsg, (const Core::IServiceEndpoint*, const WireLinControllerConfig&));
    MOCK_METHOD(void, SendMsg, (const Core::IServiceEndpoint*, const LinControllerStatusUpdate&));
    MOCK_METHOD(void, SendMsg, (const Core::IServiceEndpoint*, const LinWakeupPulse&));
};

inline auto MakeControllerConfig(LinControllerMode mode) -> LinControllerConfig
{
    LinControllerConfig config{};
    config.controllerMode = mode;
    return config;
}

inline auto MakeFrame(LinId linId, LinChecksumModel checksumModel = LinChecksumModel::Unknown,
                      uint8_t dataLength = 0, std::array<uint8_t, 8> data = std::array<uint8_t, 8>{}) -> LinFrame
{
    LinFrame frame;
    frame.id = linId;
    frame.checksumModel = checksumModel;
    frame.dataLength = dataLength;
    frame.data = data;
    return frame;
}

inline auto AFrameWithId(LinId linId) -> testing::Matcher<const LinFrame&>
{
    using namespace testing;
    return Field(&LinFrame::id, linId);
}

inline auto AHeaderRequestWithId(LinId linId) -> testing::Matcher<const LinSendFrameHeaderRequest&>
{
    using namespace testing;
    return Field(&LinSendFrameHeaderRequest::id, linId);
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

inline auto ATransmissionWith(LinFrameStatus status, std::chrono::nanoseconds timestamp)
    -> testing::Matcher<const LinTransmission&>
{
    using namespace testing;
    return AllOf(Field(&LinTransmission::status, status), Field(&LinTransmission::timestamp, timestamp));
}

inline auto ATransmissionWith(LinFrame frame, LinFrameStatus status) -> testing::Matcher<const LinTransmission&>
{
    using namespace testing;
    return AllOf(Field(&LinTransmission::frame, frame), Field(&LinTransmission::status, status));
}

inline auto ATransmissionWith(LinFrame frame, LinFrameStatus status, std::chrono::nanoseconds timestamp)
    -> testing::Matcher<const LinTransmission&>
{
    using namespace testing;
    return AllOf(Field(&LinTransmission::frame, frame), Field(&LinTransmission::status, status),
                 Field(&LinTransmission::timestamp, timestamp));
}

inline auto AControllerStatusUpdateWith(LinControllerStatus status)
    -> testing::Matcher<const LinControllerStatusUpdate&>
{
    using namespace testing;
    return Field(&LinControllerStatusUpdate::status, status);
}

struct Callbacks
{
    MOCK_METHOD3(FrameStatusHandler, void(ILinController*, const LinFrame&, LinFrameStatus));
    MOCK_METHOD1(GoToSleepHandler, void(ILinController*));
    MOCK_METHOD1(WakeupHandler, void(ILinController*));
    MOCK_METHOD1(LinSlaveConfigurationHandler, void(ILinController*));
};

inline auto ToWire(LinControllerConfig config, WireLinControllerConfig::SimulationMode simulationMode =
                                                   WireLinControllerConfig::SimulationMode::Default)
    -> WireLinControllerConfig
{
    WireLinControllerConfig result{};
    result.baudRate = config.baudRate;
    result.controllerMode = config.controllerMode;
    result.frameResponses = config.frameResponses;
    result.simulationMode = simulationMode;
    return result;
}

} // namespace Tests
} // namespace Lin
} // namespace Services
} // namespace SilKit
