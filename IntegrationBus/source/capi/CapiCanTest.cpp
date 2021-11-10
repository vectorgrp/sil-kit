// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/can/all.hpp"

namespace {
    using namespace ib::sim::can;

    class MockCanController : public ib::sim::can::ICanController {
    public:
        MOCK_METHOD2(SetBaudRate, void(uint32_t rate, uint32_t fdRate));
        MOCK_METHOD0(Reset, void());
        MOCK_METHOD0(Start, void());
        MOCK_METHOD0(Stop, void());
        MOCK_METHOD0(Sleep, void());
        MOCK_METHOD1(SendMessage, CanTxId(const CanMessage&));
        virtual auto SendMessage(CanMessage&& msg)->CanTxId {
            // Gmock does not support rvalues -> workaround
            CanMessage lv = msg;
            this->SendMessage(lv);
            return 0;
        }
        MOCK_METHOD1(RegisterReceiveMessageHandler, void(ReceiveMessageHandler));
        MOCK_METHOD1(RegisterStateChangedHandler, void(StateChangedHandler));
        MOCK_METHOD1(RegisterErrorStateChangedHandler, void(ErrorStateChangedHandler));
        MOCK_METHOD1(RegisterTransmitStatusHandler, void(MessageStatusHandler));
    };

	class CapiCanTest : public testing::Test
	{
	public: 
        MockCanController mockController;
        CapiCanTest()
		{
			
		}
	};

    void AckCallback(void* context, ib_CanController* controller, ib_CanTransmitAcknowledge* ack)
    {
    }

    void ReceiveMessage(void* context, ib_CanController* controller, ib_CanMessage* metaData)
    {
    }

    void StateChangedHandler(void* context, ib_CanController* controller, ib_CanControllerState state)
    {
    }

    void ErrorStateHandler(void* context, ib_CanController* controller, ib_CanErrorState state)
    {
    }

    TEST_F(CapiCanTest, can_controller_function_mapping)
    {

        ib_ReturnCode returnCode;

        EXPECT_CALL(mockController, SetBaudRate(123,456)).Times(testing::Exactly(1));
        returnCode = ib_CanController_SetBaudRate((ib_CanController*)&mockController, 123, 456);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Start()).Times(testing::Exactly(1));
        returnCode = ib_CanController_Start((ib_CanController*)&mockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Stop()).Times(testing::Exactly(1));
        returnCode = ib_CanController_Stop((ib_CanController*)&mockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, Sleep()).Times(testing::Exactly(1));
        returnCode = ib_CanController_Sleep((ib_CanController*)&mockController);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, SendMessage(testing::_)).Times(testing::Exactly(1));
        ib_CanFrame cf{ 1,0,0,{0,0} };
        cf.id = 1;
        returnCode = ib_CanController_SendFrame((ib_CanController*)&mockController, &cf, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterReceiveMessageHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_CanController_RegisterReceiveMessageHandler((ib_CanController*)&mockController, NULL, &ReceiveMessage);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterStateChangedHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_CanController_RegisterStateChangedHandler((ib_CanController*)&mockController, NULL, &StateChangedHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterErrorStateChangedHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_CanController_RegisterErrorStateChangedHandler((ib_CanController*)&mockController, NULL, &ErrorStateHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

        EXPECT_CALL(mockController, RegisterTransmitStatusHandler(testing::_)).Times(testing::Exactly(1));
        returnCode = ib_CanController_RegisterTransmitStatusHandler((ib_CanController*)&mockController, NULL, &AckCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
    }


    TEST_F(CapiCanTest, can_controller_nullpointer_params)
    {

        ib_ReturnCode returnCode;

        returnCode = ib_CanController_SetBaudRate(nullptr, 123, 456);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_CanController_Start(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_CanController_Stop(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_CanController_Sleep(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        ib_CanFrame cf{ 1,0,0,{0,0} };
        cf.id = 1;
        returnCode = ib_CanController_SendFrame(nullptr, &cf, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_CanController_SendFrame((ib_CanController*)&mockController, nullptr, NULL);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_CanController_RegisterReceiveMessageHandler(nullptr, NULL, &ReceiveMessage);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_CanController_RegisterReceiveMessageHandler((ib_CanController*)&mockController, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_CanController_RegisterStateChangedHandler(nullptr, NULL, &StateChangedHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_CanController_RegisterStateChangedHandler((ib_CanController*)&mockController, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_CanController_RegisterErrorStateChangedHandler(nullptr, NULL, &ErrorStateHandler);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_CanController_RegisterErrorStateChangedHandler((ib_CanController*)&mockController, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_CanController_RegisterTransmitStatusHandler(nullptr, NULL, &AckCallback);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_CanController_RegisterTransmitStatusHandler((ib_CanController*)&mockController, NULL, nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

}