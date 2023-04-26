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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"

#include "MockCapiTest.hpp"


namespace {

using testing::DoAll;
using testing::SetArgPointee;
using testing::Return;

using namespace SilKit::Services::Can;

MATCHER_P(CanFrameMatcher, controlFrame, "")
{
    *result_listener << "matches can frames of the c-api to the cpp api";

    const CanFrame frame1 = controlFrame;
    const SilKit_CanFrame* frame2 = arg;
    if (frame1.canId != frame2->id || frame1.dlc != frame2->dlc || frame1.dataField.size() != frame2->data.size)
    {
        return false;
    }
    if (frame1.dataField.size() != frame2->data.size)
    {
        return false;
    }
    for (size_t i = 0; i < frame1.dataField.size(); i++)
    {
        if (frame1.dataField[i] != frame2->data.data[i])
        {
            return false;
        }
    }
    if (frame1.sdt != frame2->sdt || frame1.vcid != frame2->vcid || frame1.af != frame2->af)
    {
        return false;
    }
    if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Ide)) != 0)
        != ((frame2->flags & SilKit_CanFrameFlag_ide) != 0))
    {
        return false;
    }
    if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)) != 0)
        != ((frame2->flags & SilKit_CanFrameFlag_fdf) != 0))
    {
        return false;
    }
    if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Brs)) != 0)
        != ((frame2->flags & SilKit_CanFrameFlag_brs) != 0))
    {
        return false;
    }
    if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Esi)) != 0)
        != ((frame2->flags & SilKit_CanFrameFlag_esi) != 0))
    {
        return false;
    }
    if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Rtr)) != 0)
        != ((frame2->flags & SilKit_CanFrameFlag_rtr) != 0))
    {
        return false;
    }
    if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Xlf)) != 0)
        != ((frame2->flags & SilKit_CanFrameFlag_xlf) != 0))
    {
        return false;
    }
    if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Sec)) != 0)
        != ((frame2->flags & SilKit_CanFrameFlag_sec) != 0))
    {
        return false;
    }
    return true;
}

class HourglassCanTest : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_CanController* mockCanController{reinterpret_cast<SilKit_CanController*>(uintptr_t(0x12345678))};

    HourglassCanTest()
    {
        using testing::_;
        ON_CALL(capi, SilKit_CanController_Create(_, _, _, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockCanController), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

TEST_F(HourglassCanTest, SilKit_CanController_Create)
{
    SilKit_Participant* participant{(SilKit_Participant*)123456};
    std::string name = "CanController1";
    std::string network = "CanNetwork1";

    EXPECT_CALL(capi, SilKit_CanController_Create(testing::_, participant, testing::StrEq(name.c_str()),
                                                  testing::StrEq(network.c_str())))
        .Times(1);
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(participant, name,
                                                                                                  network);
}

TEST_F(HourglassCanTest, SilKit_CanController_Start)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    EXPECT_CALL(capi, SilKit_CanController_Start(mockCanController)).Times(1);
    canController.Start();
}

TEST_F(HourglassCanTest, SilKit_CanController_Stop)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    EXPECT_CALL(capi, SilKit_CanController_Stop(mockCanController)).Times(1);
    canController.Stop();
}

TEST_F(HourglassCanTest, SilKit_CanController_Reset)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    EXPECT_CALL(capi, SilKit_CanController_Reset(mockCanController)).Times(1);
    canController.Reset();
}

TEST_F(HourglassCanTest, SilKit_CanController_Sleep)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    EXPECT_CALL(capi, SilKit_CanController_Sleep(mockCanController)).Times(1);
    canController.Sleep();
}

TEST_F(HourglassCanTest, SilKit_CanController_SendFrame)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    std::vector<uint8_t> payload{5};
    SilKit::Services::Can::CanFrame frame{456, SilKit_CanFrameFlag_ide, 1, 2, 3, 4, payload};
    void* userContext = &frame;
    EXPECT_CALL(capi, SilKit_CanController_SendFrame(mockCanController, CanFrameMatcher(frame), userContext)).Times(1);
    canController.SendFrame(frame, userContext);
}

TEST_F(HourglassCanTest, SilKit_CanController_SetBaudRate)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    EXPECT_CALL(capi, SilKit_CanController_SetBaudRate(mockCanController, 12345, 23456, 34567)).Times(1);
    canController.SetBaudRate(12345, 23456, 34567);
}

TEST_F(HourglassCanTest, SilKit_CanController_AddFrameTransmitHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    EXPECT_CALL(capi, SilKit_CanController_AddFrameTransmitHandler(mockCanController, testing::_, testing::_,
                                                                   SilKit_CanTransmitStatus_Transmitted, testing::_))
        .Times(1);
    canController.AddFrameTransmitHandler(nullptr, static_cast<SilKit::Services::Can::CanTransmitStatusMask>(
                                                       SilKit::Services::Can::CanTransmitStatus::Transmitted));
}

TEST_F(HourglassCanTest, SilKit_CanController_RemoveFrameTransmitHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    SilKit::Util::HandlerId handlerId{(SilKit::Util::HandlerId)1234};
    EXPECT_CALL(capi, SilKit_CanController_RemoveFrameTransmitHandler(mockCanController, 1234)).Times(1);
    canController.RemoveFrameTransmitHandler(handlerId);
}

TEST_F(HourglassCanTest, SilKit_CanController_AddFrameHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    EXPECT_CALL(capi, SilKit_CanController_AddFrameHandler(mockCanController, testing::_, testing::_,
                                                           SilKit_Direction_SendReceive, testing::_))
        .Times(1);
    canController.AddFrameHandler(
        nullptr, static_cast<SilKit::Services::DirectionMask>(SilKit::Services::TransmitDirection::TXRX));
}

TEST_F(HourglassCanTest, SilKit_CanController_RemoveFrameHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    SilKit::Util::HandlerId handlerId{(SilKit::Util::HandlerId)1234};
    EXPECT_CALL(capi, SilKit_CanController_RemoveFrameHandler(mockCanController, 1234)).Times(1);
    canController.RemoveFrameHandler(handlerId);
}

TEST_F(HourglassCanTest, SilKit_CanController_AddStateChangeHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    EXPECT_CALL(capi, SilKit_CanController_AddStateChangeHandler(mockCanController, testing::_, testing::_, testing::_))
        .Times(1);
    canController.AddStateChangeHandler(nullptr);
}

TEST_F(HourglassCanTest, SilKit_CanController_RemoveStateChangeHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    SilKit::Util::HandlerId handlerId{(SilKit::Util::HandlerId)1234};
    EXPECT_CALL(capi, SilKit_CanController_RemoveStateChangeHandler(mockCanController, 1234)).Times(1);
    canController.RemoveStateChangeHandler(handlerId);
}

TEST_F(HourglassCanTest, SilKit_CanController_AddErrorStateChangeHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    EXPECT_CALL(capi,
                SilKit_CanController_AddErrorStateChangeHandler(mockCanController, testing::_, testing::_, testing::_))
        .Times(1);
    canController.AddErrorStateChangeHandler(nullptr);
}

TEST_F(HourglassCanTest, SilKit_CanController_RemoveErrorStateChangeHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Services::Can::CanController canController(
        nullptr, "CanController1", "CanNetwork1");

    SilKit::Util::HandlerId handlerId{(SilKit::Util::HandlerId)1234};
    EXPECT_CALL(capi, SilKit_CanController_RemoveErrorStateChangeHandler(mockCanController, 1234)).Times(1);
    canController.RemoveErrorStateChangeHandler(handlerId);
}
} //namespace
