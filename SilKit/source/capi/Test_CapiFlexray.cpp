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
#include "silkit/capi/Flexray.h"
#include "silkit/capi/SilKit.h"
#include "silkit/services/flexray/all.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "MockParticipant.hpp"

namespace
{
using namespace SilKit::Core;
using namespace SilKit::Config;
using namespace SilKit::Services::Flexray;
using SilKit::Util::HandlerId;
using ::SilKit::Core::Tests::DummyParticipant;
using ::testing::_;

class MockParticipant : public DummyParticipant
{
};

class MockFlexrayController : public SilKit::Services::Flexray::IFlexrayController
{
public:
    MOCK_METHOD1(Configure, void(const FlexrayControllerConfig& config));
    MOCK_METHOD2(ReconfigureTxBuffer, void(uint16_t txBufferIdx, const FlexrayTxBufferConfig& config));
    MOCK_METHOD1(UpdateTxBuffer, void(const FlexrayTxBufferUpdate& update));
    MOCK_METHOD0(Run, void());
    MOCK_METHOD0(DeferredHalt, void());
    MOCK_METHOD0(Freeze, void());
    MOCK_METHOD0(AllowColdstart, void());
    MOCK_METHOD0(AllSlots, void());
    MOCK_METHOD0(Wakeup, void());
    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameHandler, (FrameHandler));
    MOCK_METHOD(void, RemoveFrameHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddFrameTransmitHandler, (FrameTransmitHandler));
    MOCK_METHOD(void, RemoveFrameTransmitHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddWakeupHandler, (WakeupHandler));
    MOCK_METHOD(void, RemoveWakeupHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddPocStatusHandler, (PocStatusHandler));
    MOCK_METHOD(void, RemovePocStatusHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddSymbolHandler, (SymbolHandler));
    MOCK_METHOD(void, RemoveSymbolHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddSymbolTransmitHandler, (SymbolTransmitHandler));
    MOCK_METHOD(void, RemoveSymbolTransmitHandler, (SilKit::Services::HandlerId));
    MOCK_METHOD(SilKit::Services::HandlerId, AddCycleStartHandler, (CycleStartHandler));
    MOCK_METHOD(void, RemoveCycleStartHandler, (SilKit::Services::HandlerId));
};

class Test_CapiFlexray : public testing::Test
{
public:
    Test_CapiFlexray() {}

    void SetUp() override {}
    void TearDown() override {}

    struct Callbacks
    {
        static void SilKitCALL FrameHandler(void* /*context*/, SilKit_FlexrayController* /*controller*/,
                                 const SilKit_FlexrayFrameEvent* /*message*/)
        {
        }

        static void SilKitCALL FrameTransmitHandler(void* /*context*/, SilKit_FlexrayController* /*controller*/,
                                         const SilKit_FlexrayFrameTransmitEvent* /*acknowledge*/)
        {
        }

        static void SilKitCALL WakeupHandler(void* /*context*/, SilKit_FlexrayController* /*controller*/,
                                  const SilKit_FlexrayWakeupEvent* /*wakeup*/)
        {
        }

        static void SilKitCALL PocStatusHandler(void* /*context*/, SilKit_FlexrayController* /*controller*/,
                                     const SilKit_FlexrayPocStatusEvent* /*status*/)
        {
        }

        static void SilKitCALL SymbolHandler(void* /*context*/, SilKit_FlexrayController* /*controller*/,
                                  const SilKit_FlexraySymbolEvent* /*symbol*/)
        {
        }

        static void SilKitCALL SymbolTransmitHandler(void* /*context*/, SilKit_FlexrayController* /*controller*/,
                                          const SilKit_FlexraySymbolTransmitEvent* /*acknowledge*/)
        {
        }

        static void SilKitCALL CycleStartHandler(void* /*context*/, SilKit_FlexrayController* /*controller*/,
                                      const SilKit_FlexrayCycleStartEvent* /*cycleStart*/)
        {
        }
    };

protected:
    std::string controllerName;
    std::string networkName;
    MockParticipant participant;
    MockFlexrayController mockController;
};

TEST_F(Test_CapiFlexray, make_flexray_controller)
{
    SilKit_ReturnCode returnCode;
    SilKit_FlexrayController* frController = nullptr;
    returnCode = SilKit_FlexrayController_Create(&frController, (SilKit_Participant*)&participant, controllerName.c_str(),
                                              networkName.c_str());
    // needs NullConnectionParticipant, which won't link with C-API. So just expect a general failure here.
    EXPECT_EQ(returnCode, SilKit_ReturnCode_UNSPECIFIEDERROR);
    EXPECT_EQ(frController, nullptr);
    // When using the NullConnectionParticipant, enable this:
    //EXPECT_NE(frController, nullptr);
}

TEST_F(Test_CapiFlexray, fr_controller_function_mapping)
{
    SilKit_ReturnCode returnCode;
    SilKit_FlexrayClusterParameters clusterParameters;
    SilKit_FlexrayNodeParameters nodeParameters;
    SilKit_FlexrayControllerConfig cfg;
    SilKit_HandlerId handlerId;

    SilKit_Struct_Init(SilKit_FlexrayClusterParameters, clusterParameters);
    SilKit_Struct_Init(SilKit_FlexrayNodeParameters, nodeParameters);

    SilKit_Struct_Init(SilKit_FlexrayControllerConfig, cfg);
    cfg.clusterParams = &clusterParameters;
    cfg.nodeParams = &nodeParameters;

    EXPECT_CALL(mockController, Configure(_)).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_Configure((SilKit_FlexrayController*)&mockController, &cfg);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_AddFrameHandler((SilKit_FlexrayController*)&mockController, NULL,
                                                       &Callbacks::FrameHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveFrameHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_RemoveFrameHandler((SilKit_FlexrayController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_AddFrameTransmitHandler((SilKit_FlexrayController*)&mockController, NULL,
                                                               &Callbacks::FrameTransmitHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveFrameTransmitHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_RemoveFrameTransmitHandler((SilKit_FlexrayController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddWakeupHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_AddWakeupHandler((SilKit_FlexrayController*)&mockController, NULL,
                                                        &Callbacks::WakeupHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveWakeupHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_RemoveWakeupHandler((SilKit_FlexrayController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddPocStatusHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_AddPocStatusHandler((SilKit_FlexrayController*)&mockController, NULL,
                                                           &Callbacks::PocStatusHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemovePocStatusHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_RemovePocStatusHandler((SilKit_FlexrayController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddSymbolHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_AddSymbolHandler((SilKit_FlexrayController*)&mockController, NULL,
                                                        &Callbacks::SymbolHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveSymbolHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_RemoveSymbolHandler((SilKit_FlexrayController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddSymbolTransmitHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_AddSymbolTransmitHandler((SilKit_FlexrayController*)&mockController, NULL,
                                                                &Callbacks::SymbolTransmitHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveSymbolTransmitHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_RemoveSymbolTransmitHandler((SilKit_FlexrayController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AddCycleStartHandler(testing::_)).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_AddCycleStartHandler((SilKit_FlexrayController*)&mockController, NULL,
                                                            &Callbacks::CycleStartHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, RemoveCycleStartHandler(static_cast<HandlerId>(0))).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_RemoveCycleStartHandler((SilKit_FlexrayController*)&mockController, 0);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Run()).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_ExecuteCmd((SilKit_FlexrayController*)&mockController, SilKit_FlexrayChiCommand_RUN);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, DeferredHalt()).Times(testing::Exactly(1));
    returnCode =
        SilKit_FlexrayController_ExecuteCmd((SilKit_FlexrayController*)&mockController, SilKit_FlexrayChiCommand_DEFERRED_HALT);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Freeze()).Times(testing::Exactly(1));
    returnCode =
        SilKit_FlexrayController_ExecuteCmd((SilKit_FlexrayController*)&mockController, SilKit_FlexrayChiCommand_FREEZE);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AllowColdstart()).Times(testing::Exactly(1));
    returnCode = SilKit_FlexrayController_ExecuteCmd((SilKit_FlexrayController*)&mockController,
                                                  SilKit_FlexrayChiCommand_ALLOW_COLDSTART);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, AllSlots()).Times(testing::Exactly(1));
    returnCode =
        SilKit_FlexrayController_ExecuteCmd((SilKit_FlexrayController*)&mockController, SilKit_FlexrayChiCommand_ALL_SLOTS);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    EXPECT_CALL(mockController, Wakeup()).Times(testing::Exactly(1));
    returnCode =
        SilKit_FlexrayController_ExecuteCmd((SilKit_FlexrayController*)&mockController, SilKit_FlexrayChiCommand_WAKEUP);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(Test_CapiFlexray, fr_controller_nullpointer_params)
{
    auto cMockParticipant = (SilKit_Participant*)&participant;
    SilKit_ReturnCode returnCode;
    SilKit_FlexrayClusterParameters clusterParameters;
    SilKit_FlexrayNodeParameters nodeParameters;
    SilKit_FlexrayControllerConfig cfg;
    SilKit_HandlerId handlerId;

    memset(&cfg, 0, sizeof(cfg));
    cfg.clusterParams = &clusterParameters;
    cfg.nodeParams = &nodeParameters;

    SilKit_FlexrayController* cController = (SilKit_FlexrayController*)&mockController;
    SilKit_FlexrayController* cControllerReturn = nullptr;

    returnCode = SilKit_FlexrayController_Create(nullptr, nullptr, nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_Create(nullptr, nullptr, "bad", nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_Create(&cControllerReturn, nullptr, "bad", nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_Create(nullptr, cMockParticipant, "bad", "bad");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_Create(&cControllerReturn, cMockParticipant, nullptr, "bad");
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_Create(&cControllerReturn, cMockParticipant, "bad", nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_Configure(cController, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_Configure(nullptr, &cfg);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_Configure(nullptr, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_ExecuteCmd(nullptr, SilKit_FlexrayChiCommand_RUN);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_AddFrameHandler(nullptr, NULL, &Callbacks::FrameHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddFrameHandler(cController, NULL, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddFrameHandler(cController, NULL, &Callbacks::FrameHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_FlexrayController_AddFrameTransmitHandler(nullptr, NULL, &Callbacks::FrameTransmitHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddFrameTransmitHandler(cController, NULL, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode =
        SilKit_FlexrayController_AddFrameTransmitHandler(cController, NULL, &Callbacks::FrameTransmitHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_AddWakeupHandler(nullptr, NULL, &Callbacks::WakeupHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddWakeupHandler(cController, NULL, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddWakeupHandler(cController, NULL, &Callbacks::WakeupHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_AddPocStatusHandler(nullptr, NULL, &Callbacks::PocStatusHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddPocStatusHandler(cController, NULL, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddPocStatusHandler(cController, NULL, &Callbacks::PocStatusHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_AddSymbolHandler(nullptr, NULL, &Callbacks::SymbolHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddSymbolHandler(cController, NULL, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddSymbolHandler(cController, NULL, &Callbacks::SymbolHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_FlexrayController_AddSymbolTransmitHandler(nullptr, NULL, &Callbacks::SymbolTransmitHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddSymbolTransmitHandler(cController, NULL, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode =
        SilKit_FlexrayController_AddSymbolTransmitHandler(cController, NULL, &Callbacks::SymbolTransmitHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_AddCycleStartHandler(nullptr, NULL, &Callbacks::CycleStartHandler, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddCycleStartHandler(cController, NULL, nullptr, &handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_AddCycleStartHandler(cController, NULL, &Callbacks::CycleStartHandler, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_FlexrayController_RemoveFrameHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_RemoveFrameTransmitHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_RemoveWakeupHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_RemovePocStatusHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_RemoveSymbolHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_RemoveSymbolTransmitHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    returnCode = SilKit_FlexrayController_RemoveCycleStartHandler(nullptr, handlerId);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

} // namespace
