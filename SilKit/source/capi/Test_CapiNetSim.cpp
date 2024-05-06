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
#include "silkit/experimental/netsim/all.hpp"
#include "MockParticipant.hpp"

namespace {
    using namespace SilKit::Experimental::NetworkSimulation;

    class MockParticipant
        : public SilKit::Core::Tests::DummyParticipant
    {
    public:
        MOCK_METHOD(INetworkSimulator*, CreateNetworkSimulator,
                    (), (override));
    };

    class DummyNetworkSimulator : public SilKit::Experimental::NetworkSimulation::INetworkSimulator
    {
    public:
        MOCK_METHOD(void, SimulateNetwork,
                    (const std::string& networkName, SimulatedNetworkType networkType,
                     std::unique_ptr<ISimulatedNetwork> simulatedNetwork),
                    (override));
        MOCK_METHOD(void, Start, (), (override));
    };

    class MockSimulatedNetwork : public SilKit::Experimental::NetworkSimulation::ISimulatedNetwork
    {
    public:
        MOCK_METHOD(ISimulatedController*, ProvideSimulatedController, (ControllerDescriptor controllerDescriptor),
                    (override));
        MOCK_METHOD(void, SimulatedControllerRemoved, (ControllerDescriptor controllerDescriptor), (override));
        MOCK_METHOD(void, SetEventProducer, (std::unique_ptr<IEventProducer> eventProducer), (override));
    };

    class MockCanEventProducer : public SilKit::Experimental::NetworkSimulation::Can::ICanEventProducer
    {
    public:
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Can::CanFrameEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Can::CanFrameTransmitEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Can::CanStateChangeEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Can::CanErrorStateChangeEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
    };

    class MockEthernetEventProducer : public SilKit::Experimental::NetworkSimulation::Ethernet::IEthernetEventProducer
    {
    public:
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Ethernet::EthernetFrameEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Ethernet::EthernetFrameTransmitEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Ethernet::EthernetStateChangeEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Ethernet::EthernetBitrateChangeEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
    };

    class MockLinEventProducer : public SilKit::Experimental::NetworkSimulation::Lin::ILinEventProducer
    {
    public:
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Lin::LinFrameStatusEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Lin::LinSendFrameHeaderRequest& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Lin::LinWakeupEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
    };

    class MockFlexRayEventProducer : public SilKit::Experimental::NetworkSimulation::Flexray::IFlexRayEventProducer
    {
    public:
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Flexray::FlexrayFrameEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Flexray::FlexrayFrameTransmitEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Flexray::FlexraySymbolEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Flexray::FlexraySymbolTransmitEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Flexray::FlexrayCycleStartEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
        MOCK_METHOD(void, Produce,
                    (const SilKit::Services::Flexray::FlexrayPocStatusEvent& msg,
                     const SilKit::Util::Span<const ControllerDescriptor>& receivers),
                    (override));
    };

    class Test_CapiNetsim : public testing::Test
    {
    public:
        Test_CapiNetsim() {
            mockSimulatedNetwork = std::make_unique<MockSimulatedNetwork>();
        }

    protected:
        MockParticipant mockParticipant;
        DummyNetworkSimulator mockNetworkSimulator;
        std::unique_ptr<MockSimulatedNetwork> mockSimulatedNetwork;

        MockCanEventProducer mockCanEventProducer;
        MockEthernetEventProducer mockEthernetEventProducer;
        MockLinEventProducer mockLinEventProducer;
        MockFlexRayEventProducer mockFlexRayEventProducer;
    };

    TEST_F(Test_CapiNetsim, netsim_nullpointer_params)
    {
        SilKit_ReturnCode returnCode;
        auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
        auto cMockNetworkSimulator = (SilKit_Experimental_NetworkSimulator*)&mockNetworkSimulator;

        std::string networkName = "networkName";
        SilKit_Experimental_SimulatedNetworkType networkType = SilKit_NetworkType_CAN;
        auto cMockSimulatedNetwork = (void*)(mockSimulatedNetwork.get());
        SilKit_Experimental_SimulatedNetworkFunctions cSimulatedNetworkFunctions;
        SilKit_Struct_Init(SilKit_Experimental_SimulatedNetworkFunctions, cSimulatedNetworkFunctions);

        SilKit_Experimental_EventReceivers receivers;
        SilKit_Struct_Init(SilKit_Experimental_EventReceivers, receivers);
        auto cMockCanEventProducer = (SilKit_Experimental_CanEventProducer*)&mockCanEventProducer;
        auto cMockEthernetEventProducer = (SilKit_Experimental_EthernetEventProducer*)&mockEthernetEventProducer;
        auto cMockLinEventProducer = (SilKit_Experimental_LinEventProducer*)&mockLinEventProducer;
        auto cMockFlexRayEventProducer = (SilKit_Experimental_FlexRayEventProducer*)&mockFlexRayEventProducer;

        SilKit_CanFrameEvent canFrameEvent;
        SilKit_Struct_Init(SilKit_CanFrameEvent, canFrameEvent);
        SilKit_EthernetFrameEvent ethFrameEvent;
        SilKit_Struct_Init(SilKit_EthernetFrameEvent, ethFrameEvent);
        SilKit_LinFrameStatusEvent linFrameStatusEvent;
        SilKit_Struct_Init(SilKit_LinFrameStatusEvent, linFrameStatusEvent);
        SilKit_FlexrayFrameEvent flexrayFrameEvent;
        SilKit_Struct_Init(SilKit_FlexrayFrameEvent, flexrayFrameEvent);

        // SilKit_Experimental_NetworkSimulator_Create
        returnCode = SilKit_Experimental_NetworkSimulator_Create(&cMockNetworkSimulator, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_NetworkSimulator_Create(nullptr, cMockParticipant);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        // SilKit_Experimental_NetworkSimulator_SimulateNetwork
        returnCode = SilKit_Experimental_NetworkSimulator_SimulateNetwork(nullptr, networkName.c_str(), networkType,
                                                             cMockSimulatedNetwork, &cSimulatedNetworkFunctions);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_NetworkSimulator_SimulateNetwork(cMockNetworkSimulator, nullptr, networkType,
                                                             cMockSimulatedNetwork, &cSimulatedNetworkFunctions);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_NetworkSimulator_SimulateNetwork(cMockNetworkSimulator, networkName.c_str(), networkType,
                                                             nullptr, &cSimulatedNetworkFunctions);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_NetworkSimulator_SimulateNetwork(cMockNetworkSimulator, networkName.c_str(), networkType,
                                                             cMockSimulatedNetwork, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        // SilKit_Experimental_NetworkSimulator_Start
        returnCode = SilKit_Experimental_NetworkSimulator_Start(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        
        // SilKit_Experimental_CanEventProducer_Produce
        returnCode = SilKit_Experimental_CanEventProducer_Produce(nullptr, &canFrameEvent.structHeader, &receivers);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_CanEventProducer_Produce(cMockCanEventProducer, nullptr, &receivers);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_CanEventProducer_Produce(cMockCanEventProducer, &canFrameEvent.structHeader, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        // SilKit_Experimental_EthernetEventProducer_Produce
        returnCode = SilKit_Experimental_EthernetEventProducer_Produce(nullptr, &ethFrameEvent.structHeader, &receivers);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_EthernetEventProducer_Produce(cMockEthernetEventProducer, nullptr, &receivers);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode =
            SilKit_Experimental_EthernetEventProducer_Produce(cMockEthernetEventProducer, &ethFrameEvent.structHeader, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        // SilKit_Experimental_LinEventProducer_Produce
        returnCode = SilKit_Experimental_LinEventProducer_Produce(nullptr, &linFrameStatusEvent.structHeader, &receivers);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_LinEventProducer_Produce(cMockLinEventProducer, nullptr, &receivers);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_LinEventProducer_Produce(cMockLinEventProducer, &linFrameStatusEvent.structHeader, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        // SilKit_Experimental_FlexRayEventProducer_Produce
        returnCode = SilKit_Experimental_FlexRayEventProducer_Produce(nullptr, &flexrayFrameEvent.structHeader, &receivers);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Experimental_FlexRayEventProducer_Produce(cMockFlexRayEventProducer, nullptr, &receivers);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode =
            SilKit_Experimental_FlexRayEventProducer_Produce(cMockFlexRayEventProducer, &flexrayFrameEvent.structHeader, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    }

    TEST_F(Test_CapiNetsim, netsim_function_mapping)
    {
        SilKit_ReturnCode returnCode;
        auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
        auto cMockNetworkSimulator = (SilKit_Experimental_NetworkSimulator*)&mockNetworkSimulator;
        auto cMockSimulatedNetwork = (SilKit_Experimental_SimulatedNetwork*)(mockSimulatedNetwork.get());

        SilKit_Experimental_SimulatedNetworkFunctions cSimulatedNetworkFunctions;
        SilKit_Struct_Init(SilKit_Experimental_SimulatedNetworkFunctions, cSimulatedNetworkFunctions);

        std::string networkName = "networkName";
        SilKit_Experimental_SimulatedNetworkType cNetworkType = SilKit_NetworkType_CAN;
        SimulatedNetworkType networkType = static_cast<SimulatedNetworkType>(cNetworkType);

        // SilKit_Experimental_NetworkSimulator_Create
        EXPECT_CALL(mockParticipant, CreateNetworkSimulator()).Times(testing::Exactly(1));
        returnCode = SilKit_Experimental_NetworkSimulator_Create(&cMockNetworkSimulator, cMockParticipant);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        // SilKit_Experimental_NetworkSimulator_SimulateNetwork -> SimulateNetwork
        cMockNetworkSimulator = (SilKit_Experimental_NetworkSimulator*)&mockNetworkSimulator; // Recreate because the mocked SilKit_Experimental_NetworkSimulator_Create invalidated the pointer
        EXPECT_CALL(mockNetworkSimulator, SimulateNetwork(networkName, networkType, testing::_))
            .Times(testing::Exactly(1));
        returnCode = SilKit_Experimental_NetworkSimulator_SimulateNetwork(cMockNetworkSimulator, networkName.c_str(), cNetworkType,
                                                    cMockSimulatedNetwork, &cSimulatedNetworkFunctions);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        // SilKit_Experimental_NetworkSimulator_Start -> Start
        EXPECT_CALL(mockNetworkSimulator, Start()).Times(testing::Exactly(1));
        returnCode = SilKit_Experimental_NetworkSimulator_Start(cMockNetworkSimulator);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    }
    
}
