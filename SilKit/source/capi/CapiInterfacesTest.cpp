// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <set>
#include <functional>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/capi/InterfaceIdentifiers.h"


namespace {

TEST(TestCapi_Interfaces, compile_test_capi_interface_ids)
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
    SilKit_NewDataPublisherEvent_STRUCT_VERSION,
    SilKit_RpcDiscoveryResult_STRUCT_VERSION,
    SilKit_RpcCallEvent_STRUCT_VERSION,
    SilKit_RpcCallResultEvent_STRUCT_VERSION,
    SilKit_RpcDiscoveryResultList_STRUCT_VERSION,
    SilKit_ParticipantStatus_STRUCT_VERSION,
    SilKit_LifecycleConfiguration_STRUCT_VERSION,
};
constexpr auto allSilkidIdsSize = sizeof(allSilkidIds) / sizeof(uint64_t);

TEST(TestCapi_Interfaces, interface_identifiers_are_unique)
{
    std::set<uint64_t> all{std::begin(allSilkidIds), std::end(allSilkidIds)};
    ASSERT_EQ(allSilkidIdsSize, all.size()) << "The IDs must not be duplicate";
}

}//namespace
