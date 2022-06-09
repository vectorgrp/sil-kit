#include "ib/capi/Flexray.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/sim/fr/all.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "MockParticipant.hpp"

namespace
{
using namespace ib::mw;
using namespace ib::cfg;
using namespace ib::sim::fr;
using ::ib::mw::test::DummyParticipant;
using ::testing::_;


class MockParticipant : public DummyParticipant
{
};

class MockFlexrayController : public ib::sim::fr::IFlexrayController
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
  MOCK_METHOD(ib::sim::HandlerId, AddFrameHandler, (FrameHandler));
  MOCK_METHOD(void, RemoveFrameHandler, (ib::sim::HandlerId));
  MOCK_METHOD(ib::sim::HandlerId, AddFrameTransmitHandler, (FrameTransmitHandler));
  MOCK_METHOD(void, RemoveFrameTransmitHandler, (ib::sim::HandlerId));
  MOCK_METHOD(ib::sim::HandlerId, AddWakeupHandler, (WakeupHandler));
  MOCK_METHOD(void, RemoveWakeupHandler, (ib::sim::HandlerId));
  MOCK_METHOD(ib::sim::HandlerId, AddPocStatusHandler, (PocStatusHandler));
  MOCK_METHOD(void, RemovePocStatusHandler, (ib::sim::HandlerId));
  MOCK_METHOD(ib::sim::HandlerId, AddSymbolHandler, (SymbolHandler));
  MOCK_METHOD(void, RemoveSymbolHandler, (ib::sim::HandlerId));
  MOCK_METHOD(ib::sim::HandlerId, AddSymbolTransmitHandler, (SymbolTransmitHandler));
  MOCK_METHOD(void, RemoveSymbolTransmitHandler, (ib::sim::HandlerId));
  MOCK_METHOD(ib::sim::HandlerId, AddCycleStartHandler, (CycleStartHandler));
  MOCK_METHOD(void, RemoveCycleStartHandler, (ib::sim::HandlerId));
};

class CapiFlexrayTest : public testing::Test
{
public:
  CapiFlexrayTest()
  {
  }

  void SetUp() override
  {
  }
  void TearDown() override
  {
  }

  struct Callbacks
  {
    static void FrameHandler(void* /*context*/, ib_Flexray_Controller* /*controller*/, const ib_Flexray_FrameEvent* /*message*/)
    {
    }

    static void FrameTransmitHandler(void* /*context*/, ib_Flexray_Controller* /*controller*/, const ib_Flexray_FrameTransmitEvent* /*acknowledge*/)
    {
    }

    static void WakeupHandler(void* /*context*/, ib_Flexray_Controller* /*controller*/, const ib_Flexray_WakeupEvent* /*wakeup*/)
    {
    }

    static void PocStatusHandler(void* /*context*/, ib_Flexray_Controller* /*controller*/, const ib_Flexray_PocStatusEvent* /*status*/)
    {
    }

    static void SymbolHandler(void* /*context*/, ib_Flexray_Controller* /*controller*/, const ib_Flexray_SymbolEvent* /*symbol*/)
    {
    }

    static void SymbolTransmitHandler(void* /*context*/, ib_Flexray_Controller* /*controller*/, const ib_Flexray_SymbolTransmitEvent* /*acknowledge*/)
    {
    }

    static void CycleStartHandler(void* /*context*/, ib_Flexray_Controller* /*controller*/, const ib_Flexray_CycleStartEvent* /*cycleStart*/)
    {
    }
  };

protected:
    std::string controllerName;
    std::string networkName;
    MockParticipant participant;
    MockFlexrayController mockController;
};

TEST_F(CapiFlexrayTest, make_flexray_controller)
{
  ib_ReturnCode returnCode;
  ib_Flexray_Controller* frController = nullptr;
  returnCode = ib_Flexray_Controller_Create(&frController, (ib_Participant*)&participant, controllerName.c_str(), networkName.c_str());
  // needs NullConnectionParticipant, which won't link with C-API. So just expect a general failure here.
  EXPECT_EQ(returnCode, ib_ReturnCode_UNSPECIFIEDERROR);
  EXPECT_EQ(frController, nullptr);
  // When using the NullConnectionParticipant, enable this:
  //EXPECT_NE(frController, nullptr);
}

TEST_F(CapiFlexrayTest, fr_controller_function_mapping)
{
  ib_ReturnCode returnCode;
  ib_Flexray_ClusterParameters clusterParameters;
  ib_Flexray_NodeParameters nodeParameters;
  ib_Flexray_ControllerConfig cfg;
  ib_HandlerId handlerId;

  memset(&cfg, 0, sizeof(cfg));
  cfg.clusterParams = &clusterParameters;
  cfg.nodeParams = &nodeParameters;

  EXPECT_CALL(mockController, Configure(_)).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_Configure((ib_Flexray_Controller*)&mockController, &cfg);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, AddFrameHandler(testing::_)).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_AddFrameHandler((ib_Flexray_Controller*)&mockController, NULL,
                                                     &Callbacks::FrameHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, AddFrameTransmitHandler(testing::_)).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_AddFrameTransmitHandler((ib_Flexray_Controller*)&mockController, NULL,
                                                             &Callbacks::FrameTransmitHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, AddWakeupHandler(testing::_)).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_AddWakeupHandler((ib_Flexray_Controller*)&mockController, NULL,
                                                      &Callbacks::WakeupHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, AddPocStatusHandler(testing::_)).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_AddPocStatusHandler((ib_Flexray_Controller*)&mockController, NULL,
                                                         &Callbacks::PocStatusHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, AddSymbolHandler(testing::_)).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_AddSymbolHandler((ib_Flexray_Controller*)&mockController, NULL,
                                                      &Callbacks::SymbolHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, AddSymbolTransmitHandler(testing::_)).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_AddSymbolTransmitHandler((ib_Flexray_Controller*)&mockController, NULL,
                                                              &Callbacks::SymbolTransmitHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, AddCycleStartHandler(testing::_)).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_AddCycleStartHandler((ib_Flexray_Controller*)&mockController, NULL,
                                                          &Callbacks::CycleStartHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, Run()).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_ExecuteCmd((ib_Flexray_Controller*)&mockController, ib_Flexray_ChiCommand_RUN);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, DeferredHalt()).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_ExecuteCmd((ib_Flexray_Controller*)&mockController, ib_Flexray_ChiCommand_DEFERRED_HALT);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, Freeze()).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_ExecuteCmd((ib_Flexray_Controller*)&mockController, ib_Flexray_ChiCommand_FREEZE);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, AllowColdstart()).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_ExecuteCmd((ib_Flexray_Controller*)&mockController, ib_Flexray_ChiCommand_ALLOW_COLDSTART);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, AllSlots()).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_ExecuteCmd((ib_Flexray_Controller*)&mockController, ib_Flexray_ChiCommand_ALL_SLOTS);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);

  EXPECT_CALL(mockController, Wakeup()).Times(testing::Exactly(1));
  returnCode = ib_Flexray_Controller_ExecuteCmd((ib_Flexray_Controller*)&mockController, ib_Flexray_ChiCommand_WAKEUP);
  EXPECT_EQ(returnCode, ib_ReturnCode_SUCCESS);
}

TEST_F(CapiFlexrayTest, fr_controller_nullpointer_params)
{
  auto cMockParticipant = (ib_Participant*)&participant;
  ib_ReturnCode returnCode;
  ib_Flexray_ClusterParameters clusterParameters;
  ib_Flexray_NodeParameters nodeParameters;
  ib_Flexray_ControllerConfig cfg;
  ib_HandlerId handlerId;

  memset(&cfg, 0, sizeof(cfg));
  cfg.clusterParams = &clusterParameters;
  cfg.nodeParams = &nodeParameters;

  ib_Flexray_Controller* cController = (ib_Flexray_Controller*)&mockController;
  ib_Flexray_Controller* cControllerReturn = nullptr;

  returnCode = ib_Flexray_Controller_Create(nullptr, nullptr, nullptr, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_Create(nullptr, nullptr, "bad", nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_Create(&cControllerReturn, nullptr, "bad", nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_Create(nullptr, cMockParticipant, "bad", "bad");
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_Create(&cControllerReturn, cMockParticipant, nullptr, "bad");
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_Create(&cControllerReturn, cMockParticipant, "bad", nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode = ib_Flexray_Controller_Configure(cController, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode = ib_Flexray_Controller_Configure(nullptr, &cfg);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode = ib_Flexray_Controller_Configure(nullptr, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode = ib_Flexray_Controller_ExecuteCmd(nullptr, ib_Flexray_ChiCommand_RUN);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode = ib_Flexray_Controller_AddFrameHandler(nullptr, NULL, &Callbacks::FrameHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddFrameHandler(cController, NULL, nullptr, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddFrameHandler(cController, NULL, &Callbacks::FrameHandler, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode =
      ib_Flexray_Controller_AddFrameTransmitHandler(nullptr, NULL, &Callbacks::FrameTransmitHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddFrameTransmitHandler(cController, NULL, nullptr, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddFrameTransmitHandler(cController, NULL, &Callbacks::FrameTransmitHandler, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode = ib_Flexray_Controller_AddWakeupHandler(nullptr, NULL, &Callbacks::WakeupHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddWakeupHandler(cController, NULL, nullptr, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddWakeupHandler(cController, NULL, &Callbacks::WakeupHandler, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode = ib_Flexray_Controller_AddPocStatusHandler(nullptr, NULL, &Callbacks::PocStatusHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddPocStatusHandler(cController, NULL, nullptr, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddPocStatusHandler(cController, NULL, &Callbacks::PocStatusHandler, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode = ib_Flexray_Controller_AddSymbolHandler(nullptr, NULL, &Callbacks::SymbolHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddSymbolHandler(cController, NULL, nullptr, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddSymbolHandler(cController, NULL, &Callbacks::SymbolHandler, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode =
      ib_Flexray_Controller_AddSymbolTransmitHandler(nullptr, NULL, &Callbacks::SymbolTransmitHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddSymbolTransmitHandler(cController, NULL, nullptr, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode =
      ib_Flexray_Controller_AddSymbolTransmitHandler(cController, NULL, &Callbacks::SymbolTransmitHandler, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

  returnCode = ib_Flexray_Controller_AddCycleStartHandler(nullptr, NULL, &Callbacks::CycleStartHandler, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddCycleStartHandler(cController, NULL, nullptr, &handlerId);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
  returnCode = ib_Flexray_Controller_AddCycleStartHandler(cController, NULL, &Callbacks::CycleStartHandler, nullptr);
  EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
}

} // namespace
