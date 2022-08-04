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
#include "silkit/services/can/all.hpp"
#include "MockParticipant.hpp"

namespace {
    using namespace SilKit::Services::Can; 
    using SilKit::Core::Tests::DummyParticipant;

    MATCHER_P(CanFrameMatcher, controlFrame, "") {
        *result_listener << "matches can frames of the c-api to the cpp api";
        const SilKit_CanFrame& frame2 = controlFrame;
        const CanFrame& frame1 = arg;
        if (frame1.canId != frame2.id || frame1.dlc != frame2.dlc || frame1.dataField.size() != frame2.data.size)
        {
            return false;
        }
        for (size_t i = 0; i < frame1.dataField.size(); i++)
        {
            if (frame1.dataField[i] != frame2.data.data[i])
            {
                return false;
            }
        }
        if (frame1.sdt != frame2.sdt || frame1.vcid != frame2.vcid || frame1.af != frame2.af)
        {
            return false;
        }
        if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Ide)) != 0) != ((frame2.flags & SilKit_CanFrameFlag_ide) != 0))
        {
            return false;
        }
        if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)) != 0) != ((frame2.flags & SilKit_CanFrameFlag_fdf) != 0))
        {
            return false;
        }
        if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Brs)) != 0) != ((frame2.flags & SilKit_CanFrameFlag_brs) != 0))
        {
            return false;
        }
        if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Esi)) != 0) != ((frame2.flags & SilKit_CanFrameFlag_esi) != 0))
        {
            return false;
        }
        if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Rtr)) != 0) != ((frame2.flags & SilKit_CanFrameFlag_rtr) != 0))
        {
            return false;
        }
        if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Xlf)) != 0) != ((frame2.flags & SilKit_CanFrameFlag_xlf) != 0))
        {
            return false;
        }
        if (((frame1.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Sec)) != 0) != ((frame2.flags & SilKit_CanFrameFlag_sec) != 0))
        {
            return false;
        }
        return true;
    }

    class MockCanController : public SilKit::Services::Can::ICanController
    {
    public:
        MOCK_METHOD(void, SetBaudRate, (uint32_t rate, uint32_t fdRate, uint32_t xlRate), (override));
        MOCK_METHOD(void, Reset, (), (override));
        MOCK_METHOD(void, Start, (), (override));
        MOCK_METHOD(void, Stop, (), (override));
        MOCK_METHOD(void, Sleep, (), (override));
        MOCK_METHOD(void, SendFrame, (const CanFrame&, void*), (override));
        MOCK_METHOD(SilKit::Services::HandlerId, AddFrameHandler, (FrameHandler, SilKit::Services::DirectionMask), (override));
        MOCK_METHOD(void, RemoveFrameHandler, (SilKit::Services::HandlerId), (override));
        MOCK_METHOD(SilKit::Services::HandlerId, AddStateChangeHandler, (StateChangeHandler), (override));
        MOCK_METHOD(void, RemoveStateChangeHandler, (SilKit::Services::HandlerId), (override));
        MOCK_METHOD(SilKit::Services::HandlerId, AddErrorStateChangeHandler, (ErrorStateChangeHandler), (override));
        MOCK_METHOD(void, RemoveErrorStateChangeHandler, (SilKit::Services::HandlerId), (override));
        MOCK_METHOD(SilKit::Services::HandlerId, AddFrameTransmitHandler, (FrameTransmitHandler, CanTransmitStatusMask), (override));
        MOCK_METHOD(void, RemoveFrameTransmitHandler, (SilKit::Services::HandlerId), (override));
    };

    void SilKitCALL FrameTransmitHandler(void* /*context*/, SilKit_CanController* /*controller*/, SilKit_CanFrameTransmitEvent* /*ack*/)
    {
    }

    void SilKitCALL FrameHandler(void* /*context*/, SilKit_CanController* /*controller*/, SilKit_CanFrameEvent* /*metaData*/)
    {
    }

    void SilKitCALL StateChangeHandler(void* /*context*/, SilKit_CanController* /*controller*/, SilKit_CanStateChangeEvent* /*state*/)
    {
    }

    void SilKitCALL ErrorStateChangeHandler(void* /*context*/, SilKit_CanController* /*controller*/, SilKit_CanErrorStateChangeEvent* /*state*/)
    {
    }

    class MockParticipant : public DummyParticipant
    {
    public:
        MOCK_METHOD2(CreateCanController, SilKit::Services::Can::ICanController*(const std::string& /*canonicalName*/,
                                               const std::string& /*networkName*/));
    };

    class CapiCanTest : public testing::Test
    {
    public:
        MockCanController mockController;
        CapiCanTest()
        {
        }
    };

    TEST_F(CapiCanTest, can_controller_function_mapping)
    {
        using SilKit::Util::HandlerId;

        SilKit_ReturnCode returnCode;
        MockParticipant mockParticipant;
        SilKit_HandlerId handlerId;

        EXPECT_CALL(mockParticipant, CreateCanController("ControllerName", "NetworkName")).Times(testing::Exactly(1));
        SilKit_CanController* testParam;
        returnCode = SilKit_CanController_Create(&testParam, (SilKit_Participant*)&mockParticipant, "ControllerName",
                                              "NetworkName");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SetBaudRate(123, 456, 789)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_SetBaudRate((SilKit_CanController*)&mockController, 123, 456, 789);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Start()).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_Start((SilKit_CanController*)&mockController);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Stop()).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_Stop((SilKit_CanController*)&mockController);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Sleep()).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_Sleep((SilKit_CanController*)&mockController);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendFrame(testing::_, testing::_)).Times(testing::Exactly(1));
        SilKit_CanFrame cf{};
        SilKit_Struct_Init(SilKit_CanFrame, cf);
        cf.id = 1;
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, &cf, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddFrameHandler(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_AddFrameHandler((SilKit_CanController*)&mockController, NULL, &FrameHandler,
                                                       SilKit_Direction_SendReceive, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveFrameHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_RemoveFrameHandler((SilKit_CanController*)&mockController, 0);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddStateChangeHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_AddStateChangeHandler((SilKit_CanController*)&mockController, NULL,
                                                             &StateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveStateChangeHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_RemoveStateChangeHandler((SilKit_CanController*)&mockController, 0);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddErrorStateChangeHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_AddErrorStateChangeHandler((SilKit_CanController*)&mockController, NULL,
                                                                  &ErrorStateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveErrorStateChangeHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_RemoveErrorStateChangeHandler((SilKit_CanController*)&mockController, 0);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode =
            SilKit_CanController_AddFrameTransmitHandler((SilKit_CanController*)&mockController, NULL, &FrameTransmitHandler,
                                                      SilKit_CanTransmitStatus_Transmitted, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveFrameTransmitHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_RemoveFrameTransmitHandler((SilKit_CanController*)&mockController, 0);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    }

    TEST_F(CapiCanTest, can_controller_send_frame_no_flags)
    {

        SilKit_ReturnCode returnCode;

        SilKit_CanFrame cf1{};
        SilKit_Struct_Init(SilKit_CanFrame, cf1);
        cf1.id = 1;
        cf1.data = { 0,0 };
        cf1.dlc = 1;
        cf1.flags = 0;
        EXPECT_CALL(mockController, SendFrame(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    }

    TEST_F(CapiCanTest, can_controller_send_frame_flags1)
    {

        SilKit_ReturnCode returnCode;

        SilKit_CanFrame cf1{};
        SilKit_Struct_Init(SilKit_CanFrame, cf1);
        cf1.id = 1;
        cf1.data = { 0,0 };
        cf1.dlc = 1;
        cf1.flags = SilKit_CanFrameFlag_ide | SilKit_CanFrameFlag_rtr | SilKit_CanFrameFlag_esi;
        EXPECT_CALL(mockController, SendFrame(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    }

    TEST_F(CapiCanTest, can_controller_send_frame_flags2)
    {

        SilKit_ReturnCode returnCode;

        SilKit_CanFrame cf1{};
        SilKit_Struct_Init(SilKit_CanFrame, cf1);
        cf1.id = 1;
        cf1.data = { 0,0 };
        cf1.dlc = 1;
        cf1.flags = SilKit_CanFrameFlag_fdf | SilKit_CanFrameFlag_brs;
        EXPECT_CALL(mockController, SendFrame(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    }


    TEST_F(CapiCanTest, can_controller_nullpointer_params)
    {
        SilKit_ReturnCode returnCode;
        SilKit_HandlerId handlerId;

        returnCode = SilKit_CanController_SetBaudRate(nullptr, 123, 456, 789);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode = SilKit_CanController_Start(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode = SilKit_CanController_Stop(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode = SilKit_CanController_Sleep(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        SilKit_CanFrame cf{};
        SilKit_Struct_Init(SilKit_CanFrame, cf);
        cf.id = 1;
        returnCode = SilKit_CanController_SendFrame(nullptr, &cf, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, nullptr, NULL);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode =
            SilKit_CanController_AddFrameHandler(nullptr, NULL, &FrameHandler, SilKit_Direction_SendReceive, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddFrameHandler((SilKit_CanController*)&mockController, NULL, nullptr,
                                                       SilKit_Direction_SendReceive, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddFrameHandler((SilKit_CanController*)&mockController, NULL, &FrameHandler,
                                                       SilKit_Direction_SendReceive, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode = SilKit_CanController_AddStateChangeHandler(nullptr, NULL, &StateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode =
            SilKit_CanController_AddStateChangeHandler((SilKit_CanController*)&mockController, NULL, nullptr, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddStateChangeHandler((SilKit_CanController*)&mockController, NULL,
                                                             &StateChangeHandler, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode = SilKit_CanController_AddErrorStateChangeHandler(nullptr, NULL, &ErrorStateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddErrorStateChangeHandler((SilKit_CanController*)&mockController, NULL, nullptr,
                                                                  &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddErrorStateChangeHandler((SilKit_CanController*)&mockController, NULL,
                                                                  &ErrorStateChangeHandler, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode = SilKit_CanController_AddFrameTransmitHandler(nullptr, NULL, &FrameTransmitHandler,
                                                               SilKit_CanTransmitStatus_Canceled, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddFrameTransmitHandler((SilKit_CanController*)&mockController, NULL, nullptr,
                                                               SilKit_CanTransmitStatus_Canceled, &handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_AddFrameTransmitHandler(
            (SilKit_CanController*)&mockController, NULL, &FrameTransmitHandler, SilKit_CanTransmitStatus_Canceled, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);


        returnCode = SilKit_CanController_RemoveFrameHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_RemoveFrameTransmitHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_RemoveStateChangeHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_CanController_RemoveErrorStateChangeHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    }

TEST_F(CapiCanTest, send_with_invalud_struct_header)
{
    SilKit_CanFrame cf{};// we do not call SilKit_Struct_Init(SilKit_CanFrame, cf) here
    cf.id = 1;
    auto returnCode = SilKit_CanController_SendFrame((SilKit_CanController*)&mockController, &cf, nullptr);
    ASSERT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

} //namespace
