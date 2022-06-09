// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/can/all.hpp"
#include "MockParticipant.hpp"

namespace {
    using namespace ib::sim::can; 
    using ib::mw::test::DummyParticipant;

    MATCHER_P(CanFrameMatcher, controlFrame, "") {
        *result_listener << "matches can frames of the c-api to the cpp api";
        auto frame2 = controlFrame;
        ib::sim::can::CanFrame frame;
        auto frame1 = arg;
        if (frame1.canId != frame2.id || frame1.dlc != frame2.dlc || frame1.dataField.size() != frame2.data.size) 
        {
            return false;
        }
        for (size_t i = 0; i < frame1.dataField.size(); i++)
        {
            if (frame1.dataField[i] != frame2.data.data[i]) {
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
        MOCK_METHOD(void, SetBaudRate, (uint32_t rate, uint32_t fdRate));
        MOCK_METHOD(void, Reset, ());
        MOCK_METHOD(void, Start, ());
        MOCK_METHOD(void, Stop, ());
        MOCK_METHOD(void, Sleep, ());
        MOCK_METHOD(CanTxId,SendFrame, (const CanFrame&, void*));
        MOCK_METHOD(ib::sim::HandlerId, AddFrameHandler, (FrameHandler, ib::sim::DirectionMask));
        MOCK_METHOD(void, RemoveFrameHandler, (ib::sim::HandlerId));
        MOCK_METHOD(ib::sim::HandlerId, AddStateChangeHandler, (StateChangeHandler));
        MOCK_METHOD(void, RemoveStateChangeHandler, (ib::sim::HandlerId));
        MOCK_METHOD(ib::sim::HandlerId, AddErrorStateChangeHandler, (ErrorStateChangeHandler));
        MOCK_METHOD(void, RemoveErrorStateChangeHandler, (ib::sim::HandlerId));
        MOCK_METHOD(ib::sim::HandlerId, AddFrameTransmitHandler, (FrameTransmitHandler, CanTransmitStatusMask));
        MOCK_METHOD(void, RemoveFrameTransmitHandler, (ib::sim::HandlerId));
    };

    void FrameTransmitHandler(void* /*context*/, ib_Can_Controller* /*controller*/, ib_Can_FrameTransmitEvent* /*ack*/)
    {
    }

    void FrameHandler(void* /*context*/, ib_Can_Controller* /*controller*/, ib_Can_FrameEvent* /*metaData*/)
    {
    }

    void StateChangeHandler(void* /*context*/, ib_Can_Controller* /*controller*/, ib_Can_StateChangeEvent* /*state*/)
    {
    }

    void ErrorStateChangeHandler(void* /*context*/, ib_Can_Controller* /*controller*/, ib_Can_ErrorStateChangeEvent* /*state*/)
    {
    }

    class MockParticipant : public DummyParticipant
    {
    public:
        MOCK_METHOD2(CreateCanController, ib::sim::can::ICanController*(const std::string& /*canonicalName*/,
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

        ib_ReturnCode returnCode;
        MockParticipant mockParticipant;
        ib_HandlerId handlerId;

        EXPECT_CALL(mockParticipant, CreateCanController("ControllerName", "NetworkName")).Times(testing::Exactly(1));
        ib_Can_Controller* testParam;
        returnCode = ib_Can_Controller_Create(&testParam, (ib_Participant*)&mockParticipant, "ControllerName",
                                              "NetworkName");
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

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

        EXPECT_CALL(mockController, SendFrame(testing::_, testing::_)).Times(testing::Exactly(1));
        ib_Can_Frame cf{ ib_InterfaceIdentifier_CanFrame, 0,0,0,{0,0} };
        cf.id = 1;
        returnCode = ib_Can_Controller_SendFrame((ib_Can_Controller*)&mockController, &cf, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddFrameHandler(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_AddFrameHandler((ib_Can_Controller*)&mockController, NULL, &FrameHandler,
                                                       ib_Direction_SendReceive, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveFrameHandler(0)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_RemoveFrameHandler((ib_Can_Controller*)&mockController, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddStateChangeHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_AddStateChangeHandler((ib_Can_Controller*)&mockController, NULL,
                                                             &StateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveStateChangeHandler(0)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_RemoveStateChangeHandler((ib_Can_Controller*)&mockController, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddErrorStateChangeHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_AddErrorStateChangeHandler((ib_Can_Controller*)&mockController, NULL,
                                                                  &ErrorStateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveErrorStateChangeHandler(0)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_RemoveErrorStateChangeHandler((ib_Can_Controller*)&mockController, 0);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_, testing::_)).Times(testing::Exactly(1));
        returnCode =
            ib_Can_Controller_AddFrameTransmitHandler((ib_Can_Controller*)&mockController, NULL, &FrameTransmitHandler,
                                                      ib_Can_TransmitStatus_Transmitted, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RemoveFrameTransmitHandler(0)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_RemoveFrameTransmitHandler((ib_Can_Controller*)&mockController, 0);
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
        EXPECT_CALL(mockController, SendFrame(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
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
        EXPECT_CALL(mockController, SendFrame(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
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
        EXPECT_CALL(mockController, SendFrame(CanFrameMatcher(cf1), testing::_)).Times(testing::Exactly(1));
        returnCode = ib_Can_Controller_SendFrame((ib_Can_Controller*)&mockController, &cf1, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }


    TEST_F(CapiCanTest, can_controller_nullpointer_params)
    {
        ib_ReturnCode returnCode;
        ib_HandlerId handlerId;

        returnCode = ib_Can_Controller_SetBaudRate(nullptr, 123, 456);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_Start(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_Stop(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_Can_Controller_Sleep(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        ib_Can_Frame cf{ ib_InterfaceIdentifier_CanFrame, 1,0,0,{0,0} };
        cf.id = 1;
        returnCode = ib_Can_Controller_SendFrame(nullptr, &cf, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_SendFrame((ib_Can_Controller*)&mockController, nullptr, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);


        returnCode =
            ib_Can_Controller_AddFrameHandler(nullptr, NULL, &FrameHandler, ib_Direction_SendReceive, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_AddFrameHandler((ib_Can_Controller*)&mockController, NULL, nullptr,
                                                       ib_Direction_SendReceive, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_AddFrameHandler((ib_Can_Controller*)&mockController, NULL, &FrameHandler,
                                                       ib_Direction_SendReceive, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);


        returnCode = ib_Can_Controller_AddStateChangeHandler(nullptr, NULL, &StateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode =
            ib_Can_Controller_AddStateChangeHandler((ib_Can_Controller*)&mockController, NULL, nullptr, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_AddStateChangeHandler((ib_Can_Controller*)&mockController, NULL,
                                                             &StateChangeHandler, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);


        returnCode = ib_Can_Controller_AddErrorStateChangeHandler(nullptr, NULL, &ErrorStateChangeHandler, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_AddErrorStateChangeHandler((ib_Can_Controller*)&mockController, NULL, nullptr,
                                                                  &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_AddErrorStateChangeHandler((ib_Can_Controller*)&mockController, NULL,
                                                                  &ErrorStateChangeHandler, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);


        returnCode = ib_Can_Controller_AddFrameTransmitHandler(nullptr, NULL, &FrameTransmitHandler,
                                                               ib_Can_TransmitStatus_Canceled, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_AddFrameTransmitHandler((ib_Can_Controller*)&mockController, NULL, nullptr,
                                                               ib_Can_TransmitStatus_Canceled, &handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_AddFrameTransmitHandler(
            (ib_Can_Controller*)&mockController, NULL, &FrameTransmitHandler, ib_Can_TransmitStatus_Canceled, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);


        returnCode = ib_Can_Controller_RemoveFrameHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_RemoveFrameTransmitHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_RemoveStateChangeHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Can_Controller_RemoveErrorStateChangeHandler(nullptr, handlerId);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

}
