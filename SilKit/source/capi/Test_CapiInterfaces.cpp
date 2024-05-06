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

#include <set>
#include <functional>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/capi/InterfaceIdentifiers.h"


namespace {

TEST(Test_CapiInterfaces, compile_test_capi_interface_ids)
{
	constexpr const auto foo = SilKit_CanStateChangeEvent_STRUCT_VERSION;
    static_assert(SK_ID_GET_SERVICE(foo) == SK_ID_SERVICE_Can, "service id extraction");
	static_assert(SK_ID_GET_DATATYPE(foo) == SilKit_CanStateChangeEvent_DATATYPE_ID, "datatype id extraction");
	static_assert(SK_ID_GET_VERSION(foo) == SilKit_CanStateChangeEvent_VERSION, "datatype version extraction");
	ASSERT_TRUE(SK_ID_IS_VALID(foo));
}

// Generate this table by script, to verify that there are no copy-pastos.
constexpr uint64_t allSilkidIds[]= {
    SilKit_CanFrame_STRUCT_VERSION,
    SilKit_CanFrameTransmitEvent_STRUCT_VERSION,
    SilKit_CanFrameEvent_STRUCT_VERSION,
    SilKit_CanStateChangeEvent_STRUCT_VERSION,
    SilKit_CanErrorStateChangeEvent_STRUCT_VERSION,
    SilKit_EthernetFrameEvent_STRUCT_VERSION,
    SilKit_EthernetFrameTransmitEvent_STRUCT_VERSION,
    SilKit_EthernetStateChangeEvent_STRUCT_VERSION,
    SilKit_EthernetBitrateChangeEvent_STRUCT_VERSION,
    SilKit_EthernetFrame_STRUCT_VERSION,
    SilKit_FlexrayFrameEvent_STRUCT_VERSION,
    SilKit_FlexrayFrameTransmitEvent_STRUCT_VERSION,
    SilKit_FlexraySymbolEvent_STRUCT_VERSION,
    SilKit_FlexraySymbolTransmitEvent_STRUCT_VERSION,
    SilKit_FlexrayCycleStartEvent_STRUCT_VERSION,
    SilKit_FlexrayPocStatusEvent_STRUCT_VERSION,
    SilKit_FlexrayWakeupEvent_STRUCT_VERSION,
    SilKit_FlexrayControllerConfig_STRUCT_VERSION,
    SilKit_FlexrayClusterParameters_STRUCT_VERSION,
    SilKit_FlexrayNodeParameters_STRUCT_VERSION,
    SilKit_FlexrayHostCommand_STRUCT_VERSION,
    SilKit_FlexrayHeader_STRUCT_VERSION,
    SilKit_FlexrayFrame_STRUCT_VERSION,
    SilKit_FlexrayTxBufferConfig_STRUCT_VERSION,
    SilKit_FlexrayTxBufferUpdate_STRUCT_VERSION,
    SilKit_LinFrame_STRUCT_VERSION,
    SilKit_LinFrameResponse_STRUCT_VERSION,
    SilKit_LinControllerConfig_STRUCT_VERSION,
    SilKit_LinFrameStatusEvent_STRUCT_VERSION,
    SilKit_LinGoToSleepEvent_STRUCT_VERSION,
    SilKit_LinWakeupEvent_STRUCT_VERSION,
    SilKit_DataMessageEvent_STRUCT_VERSION,
    SilKit_DataSpec_STRUCT_VERSION,
    SilKit_RpcSpec_STRUCT_VERSION,
    SilKit_RpcCallEvent_STRUCT_VERSION,
    SilKit_RpcCallResultEvent_STRUCT_VERSION,
    SilKit_ParticipantStatus_STRUCT_VERSION,
    SilKit_LifecycleConfiguration_STRUCT_VERSION,
    SilKit_WorkflowConfiguration_STRUCT_VERSION,
    SilKit_ParticipantConnectionInformation_STRUCT_VERSION,
    SilKit_Experimental_EventReceivers_STRUCT_VERSION,
    SilKit_Experimental_SimulatedNetworkFunctions_STRUCT_VERSION,
    SilKit_Experimental_SimulatedCanControllerFunctions_STRUCT_VERSION,
    SilKit_Experimental_SimulatedFlexRayControllerFunctions_STRUCT_VERSION,
    SilKit_Experimental_SimulatedEthernetControllerFunctions_STRUCT_VERSION,
    SilKit_Experimental_SimulatedLinControllerFunctions_STRUCT_VERSION,
    SilKit_Experimental_NetSim_CanConfigureBaudrate_STRUCT_VERSION,
    SilKit_Experimental_NetSim_CanControllerMode_STRUCT_VERSION,
    SilKit_Experimental_NetSim_CanFrameRequest_STRUCT_VERSION,
    SilKit_Experimental_NetSim_FlexrayControllerConfig_STRUCT_VERSION,
    SilKit_Experimental_NetSim_FlexrayHostCommand_STRUCT_VERSION,
    SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate_STRUCT_VERSION,
    SilKit_Experimental_NetSim_FlexrayTxBufferUpdate_STRUCT_VERSION,
    SilKit_Experimental_NetSim_EthernetFrameRequest_STRUCT_VERSION,
    SilKit_Experimental_NetSim_EthernetControllerMode_STRUCT_VERSION,
    SilKit_Experimental_NetSim_LinFrameRequest_STRUCT_VERSION,
    SilKit_Experimental_NetSim_LinFrameHeaderRequest_STRUCT_VERSION,
    SilKit_Experimental_NetSim_LinWakeupPulse_STRUCT_VERSION,
    SilKit_Experimental_NetSim_LinControllerConfig_STRUCT_VERSION,
    SilKit_Experimental_NetSim_LinFrameResponseUpdate_STRUCT_VERSION,
    SilKit_Experimental_NetSim_LinControllerStatusUpdate_STRUCT_VERSION,
};
constexpr auto allSilkidIdsSize = sizeof(allSilkidIds) / sizeof(uint64_t);

TEST(Test_CapiInterfaces, interface_identifiers_are_unique)
{
    std::set<uint64_t> all{std::begin(allSilkidIds), std::end(allSilkidIds)};
    ASSERT_EQ(allSilkidIdsSize, all.size()) << "The IDs must not be duplicate";
}

TEST(Test_CapiInterfaces, silkit_struct_init_zeros_whole_structure)
{
    SilKit_CanFrameEvent value;
    // first member
    value.timestamp = 0x1234;
    // last member
    value.userContext = reinterpret_cast<void*>(static_cast<uintptr_t>(0x5678));

    ASSERT_NE(value.timestamp, 0);
    ASSERT_NE(value.userContext, nullptr);
    SilKit_Struct_Init(SilKit_CanFrameEvent, value);
    EXPECT_EQ(value.timestamp, 0);
    EXPECT_EQ(value.userContext, nullptr);
}

}//namespace
