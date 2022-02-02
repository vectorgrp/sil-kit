// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/can/all.hpp"

namespace {
    using namespace ib::sim::can; 

    MATCHER_P(CanFrameMatcher, controlFrame, "matches can frames of the c-api to the cpp api") {
        auto frame2 = controlFrame;
        ib::sim::can::CanMessage frame;
        auto frame1 = arg;
        if (frame1.canId != frame2.id || frame1.dlc != frame2.dlc || frame1.dataField.size() != frame2.data.size) 
        {
            return false;
        }
        for (int i = 0; i < frame1.dataField.size(); i++)
        {
            if (frame1.dataField[i] != frame2.data.pointer[i]) {
                return false;
            }
        }
        if((frame1.flags.ide != 0) != ((frame2.flags & ib_Can_FrameFlag_ide) != 0))
        {
            return false;
        }
        if ((frame1.flags.fdf != 0) != ((frame2.flags & ib_Can_FrameFlag_fdf) != 0))
        {
            return false;
        }
        if ((frame1.flags.brs != 0) != ((frame2.flags & ib_Can_FrameFlag_brs) != 0))
        {
            return false;
        }
        if ((frame1.flags.esi != 0) != ((frame2.flags & ib_Can_FrameFlag_esi) != 0))
        {
            return false;
        }
        if ((frame1.flags.rtr != 0) != ((frame2.flags & ib_Can_FrameFlag_rtr) != 0))
        {
            return false;
        }
        return true;
    }

    class MockCanController : public ib::sim::can::ICanController
    {
    public:
        MOCK_METHOD2(SetBaudRate, void(uint32_t rate, uint32_t fdRate));
        MOCK_METHOD0(Reset, void());
        MOCK_METHOD0(Start, void());
        MOCK_METHOD0(Stop, void());
        MOCK_METHOD0(Sleep, void());
        MOCK_METHOD2(SendMessage, CanTxId(const CanMessage&, void*));
        virtual auto SendMessage(CanMessage&& msg, void* userContext)->CanTxId {
            // Gmock does not support rvalues -> workaround
            CanMessage lv = msg;
            this->SendMessage(lv, userContext);
            return 0;
        }
        MOCK_METHOD2(RegisterReceiveMessageHandler, void(ReceiveMessageHandler, ib::sim::DirectionMask));
        MOCK_METHOD1(RegisterStateChangedHandler, void(StateChangedHandler));
        MOCK_METHOD1(RegisterErrorStateChangedHandler, void(ErrorStateChangedHandler));
        MOCK_METHOD2(RegisterTransmitStatusHandler, void(MessageStatusHandler, CanTransmitStatusMask));
    };

    void AckCallback(void* context, ib_Can_Controller* controller, ib_Can_TransmitAcknowledge* ack)
    {
    }

    void ReceiveMessage(void* context, ib_Can_Controller* controller, ib_Can_Message* metaData)
    {
    }

    void StateChangedHandler(void* context, ib_Can_Controller* controller, ib_Can_ControllerState state)
    {
    }

    void ErrorStateHandler(void* context, ib_Can_Controller* controller, ib_Can_ErrorState state)
    {
    }

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

        ib_ReturnCode returnCode;

        EXPECT_CALL(mockController, SetBaudRate(123,456)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_SetBaudRate((ib_Can_Controller*)&mockController, 123, 456);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Start()).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_Start((ib_Can_Controller*)&mockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Stop()).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_Stop((ib_Can_Controller*)&mockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Sleep()).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_Sleep((ib_Can_Controller*)&mockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendMessage(testing::_, testing::_)).Times(testing::Exactly(1));
        ib_Can_Frame cf{ 1,0,0,{0,0} };
        cf.id = 1;
        returnCode = ib_Can_Controller_SendFrame((ib_Can_Controller*)&mockController, &cf, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterReceiveMessageHandler(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_RegisterReceiveMessageHandler((ib_Can_Controller*)&mockController, NULL, &ReceiveMessage, ib_Direction_SendReceive);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterStateChangedHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_RegisterStateChangedHandler((ib_Can_Controller*)&mockController, NULL, &StateChangedHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterErrorStateChangedHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_RegisterErrorStateChangedHandler((ib_Can_Controller*)&mockController, NULL, &ErrorStateHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterTransmitStatusHandler(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_RegisterTransmitStatusHandler((ib_Can_Controller*)&mockController, NULL, &AckCallback, ib_Can_TransmitStatus_Transmitted);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }

    TEST_F(CapiCanTest, can_controller_send_frame_no_flags)
    {

        ib_ReturnCode returnCode;

        ib_Can_Frame cf1{};
        cf1.id = 1;
        cf1.data = { 0,0 };
        cf1.dlc = 1;
        cf1.flags = 0;
        EXPECT_CALL(mockController, SendMessage(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_SendFrame((ib_Can_Controller*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }

    TEST_F(CapiCanTest, can_controller_send_frame_flags1)
    {

        ib_ReturnCode returnCode;

        ib_Can_Frame cf1{};
        cf1.id = 1;
        cf1.data = { 0,0 };
        cf1.dlc = 1;
        cf1.flags = ib_Can_FrameFlag_ide | ib_Can_FrameFlag_rtr | ib_Can_FrameFlag_esi;
        EXPECT_CALL(mockController, SendMessage(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_SendFrame((ib_Can_Controller*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }

    TEST_F(CapiCanTest, can_controller_send_frame_flags2)
    {

        ib_ReturnCode returnCode;

        ib_Can_Frame cf1{};
        cf1.id = 1;
        cf1.data = { 0,0 };
        cf1.dlc = 1;
        cf1.flags = ib_Can_FrameFlag_fdf | ib_Can_FrameFlag_brs;
        EXPECT_CALL(mockController, SendMessage(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_SendFrame((ib_Can_Controller*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }


    TEST_F(CapiCanTest, can_controller_nullpointer_params)
    {

        ib_ReturnCode returnCode;

        returnCode = ib_Can_Controller_SetBaudRate(nullptr, 123, 456);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_Start(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_Stop(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_Sleep(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        ib_Can_Frame cf{ 1,0,0,{0,0} };
        cf.id = 1;
        returnCode = ib_Can_Controller_SendFrame(nullptr, &cf, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_SendFrame((ib_Can_Controller*)&mockController, nullptr, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_RegisterReceiveMessageHandler(nullptr, NULL, &ReceiveMessage, ib_Direction_SendReceive);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_RegisterReceiveMessageHandler((ib_Can_Controller*)&mockController, NULL, nullptr, ib_Direction_SendReceive);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_RegisterStateChangedHandler(nullptr, NULL, &StateChangedHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_RegisterStateChangedHandler((ib_Can_Controller*)&mockController, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_RegisterErrorStateChangedHandler(nullptr, NULL, &ErrorStateHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_RegisterErrorStateChangedHandler((ib_Can_Controller*)&mockController, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_RegisterTransmitStatusHandler(nullptr, NULL, &AckCallback, ib_Can_TransmitStatus_Canceled);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_RegisterTransmitStatusHandler((ib_Can_Controller*)&mockController, NULL, nullptr, ib_Can_TransmitStatus_Canceled);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

}
