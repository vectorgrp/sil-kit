// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <set>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/capi/InterfaceIdentifiers.h"

namespace {

TEST(TestCapi_Interfaces, compile_test_capi_interface_ids)
{
	constexpr const auto foo = SilKit_CanStateChangeEvent_InterfaceIdentifier;
    static_assert(SK_ID_GET_SERVICE(foo) == SK_ID_SERVICE_Can, "service id extraction");
	static_assert(SK_ID_GET_DATATYPE(foo) == SilKit_CanStateChangeEvent_DATATYPE_ID, "datatype id extraction");
	static_assert(SK_ID_GET_VERSION(foo) == SilKit_CanStateChangeEvent_VERSION, "datatype version extraction");
	ASSERT_TRUE(SK_ID_IS_VALID(foo));
}

// Generate this table by script, to verify that there are no copy-pastos.
// E.g. awk '/^#define SilKit_Interface.*SK_ID_MAKE/{print $2}' InterfaceIdentifiers.h
constexpr SilKit_InterfaceIdentifier allSilkidIds[]= {
    SilKit_CanFrame_InterfaceIdentifier,
    SilKit_CanFrameTransmitEvent_InterfaceIdentifier,
    SilKit_CanFrameEvent_InterfaceIdentifier,
    SilKit_CanStateChangeEvent_InterfaceIdentifier,
    SilKit_CanErrorStateChangeEvent_InterfaceIdentifier,
    SilKit_EthernetFrameEvent_InterfaceIdentifier,
    SilKit_EthernetFrameTransmitEvent_InterfaceIdentifier,
    SilKit_EthernetStateChangeEvent_InterfaceIdentifier,
    SilKit_EthernetBitrateChangeEvent_InterfaceIdentifier,
    SilKit_EthernetFrame_InterfaceIdentifier,
    SilKit_FlexrayFrameEvent_InterfaceIdentifier,
    SilKit_FlexrayFrameTransmitEvent_InterfaceIdentifier,
    SilKit_FlexraySymbolEvent_InterfaceIdentifier,
    SilKit_FlexraySymbolTransmitEvent_InterfaceIdentifier,
    SilKit_FlexrayCycleStartEvent_InterfaceIdentifier,
    SilKit_FlexrayPocStatusEvent_InterfaceIdentifier,
    SilKit_FlexrayWakeupEvent_InterfaceIdentifier,
    SilKit_FlexrayControllerConfig_InterfaceIdentifier,
    SilKit_FlexrayClusterParameters_InterfaceIdentifier,
    SilKit_FlexrayNodeParameters_InterfaceIdentifier,
    SilKit_FlexrayHostCommand_InterfaceIdentifier,
    SilKit_FlexrayHeader_InterfaceIdentifier,
    SilKit_FlexrayFrame_InterfaceIdentifier,
    SilKit_FlexrayTxBufferConfig_InterfaceIdentifier,
    SilKit_FlexrayTxBufferUpdate_InterfaceIdentifier,
    SilKit_LinFrame_InterfaceIdentifier,
    SilKit_LinFrameResponse_InterfaceIdentifier,
    SilKit_LinControllerConfig_InterfaceIdentifier,
    SilKit_LinFrameStatusEvent_InterfaceIdentifier,
    SilKit_LinGoToSleepEvent_InterfaceIdentifier,
    SilKit_LinWakeupEvent_InterfaceIdentifier,
    SilKit_DataMessageEvent_InterfaceIdentifier,
    SilKit_NewDataPublisherEvent_InterfaceIdentifier,
    SilKit_RpcDiscoveryResult_InterfaceIdentifier,
    SilKit_RpcCallEvent_InterfaceIdentifier,
    SilKit_RpcCallResultEvent_InterfaceIdentifier,
    SilKit_RpcDiscoveryResultList_InterfaceIdentifier,
    SilKit_ParticipantStatus_InterfaceIdentifier,
};
constexpr auto allSilkidIdsSize = sizeof(allSilkidIds) / sizeof(SilKit_InterfaceIdentifier);

TEST(TestCapi_Interfaces, interface_identifiers_are_unique)
{
    std::set<SilKit_InterfaceIdentifier> all{std::begin(allSilkidIds), std::end(allSilkidIds)};
    ASSERT_EQ(allSilkidIdsSize, all.size()) << "The IDs must not be duplicate";
}

}//namespace
