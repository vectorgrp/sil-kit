// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <set>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"
#include "silkit/capi/InterfaceIdentifiers.h"

namespace {

TEST(TestCapi_Interfaces, compile_test_capi_interface_ids)
{
	constexpr const auto foo = SilKit_InterfaceIdentifier_CanStateChangeEvent;
    static_assert(SK_ID_GET_SERVICE(foo) == SK_ID_SERVICE_Can, "service id extraction");
	static_assert(SK_ID_GET_DATATYPE(foo) == SK_ID_DATATYPE_CanStateChangeEvent, "datatype id extraction");
	static_assert(SK_ID_GET_VERSION(foo) == SK_ID_VERSION_CanStateChangeEvent, "datatype version extraction");
	ASSERT_TRUE(SK_ID_IS_VALID(foo));
}

// Generate this table by script, to verify that there are no copy-pastos.
// E.g. awk '/^#define SilKit_Interface.*SK_ID_MAKE/{print $2}' InterfaceIdentifiers.h
constexpr SilKit_InterfaceIdentifier allSilkidIds[]= {
    SilKit_InterfaceIdentifier_CanFrame,
    SilKit_InterfaceIdentifier_CanFrameTransmitEvent,
    SilKit_InterfaceIdentifier_CanFrameEvent,
    SilKit_InterfaceIdentifier_CanStateChangeEvent,
    SilKit_InterfaceIdentifier_CanErrorStateChangeEvent,
    SilKit_InterfaceIdentifier_EthernetFrameEvent,
    SilKit_InterfaceIdentifier_EthernetFrameTransmitEvent,
    SilKit_InterfaceIdentifier_EthernetStateChangeEvent,
    SilKit_InterfaceIdentifier_EthernetBitrateChangeEvent,
    SilKit_InterfaceIdentifier_EthernetFrame,
    SilKit_InterfaceIdentifier_FlexrayFrameEvent,
    SilKit_InterfaceIdentifier_FlexrayFrameTransmitEvent,
    SilKit_InterfaceIdentifier_FlexraySymbolEvent,
    SilKit_InterfaceIdentifier_FlexraySymbolTransmitEvent,
    SilKit_InterfaceIdentifier_FlexrayCycleStartEvent,
    SilKit_InterfaceIdentifier_FlexrayPocStatusEvent,
    SilKit_InterfaceIdentifier_FlexrayWakeupEvent,
    SilKit_InterfaceIdentifier_FlexrayControllerConfig,
    SilKit_InterfaceIdentifier_FlexrayClusterParameters,
    SilKit_InterfaceIdentifier_FlexrayNodeParameters,
    SilKit_InterfaceIdentifier_FlexrayHostCommand,
    SilKit_InterfaceIdentifier_FlexrayHeader,
    SilKit_InterfaceIdentifier_FlexrayFrame,
    SilKit_InterfaceIdentifier_FlexrayTxBufferConfig,
    SilKit_InterfaceIdentifier_FlexrayTxBufferUpdate,
    SilKit_InterfaceIdentifier_LinFrame,
    SilKit_InterfaceIdentifier_LinFrameResponse,
    SilKit_InterfaceIdentifier_LinControllerConfig,
    SilKit_InterfaceIdentifier_LinFrameStatusEvent,
    SilKit_InterfaceIdentifier_LinGoToSleepEvent,
    SilKit_InterfaceIdentifier_LinWakeupEvent,
    SilKit_InterfaceIdentifier_DataMessageEvent,
    SilKit_InterfaceIdentifier_NewDataPublisherEvent,
    SilKit_InterfaceIdentifier_RpcDiscoveryResult,
    SilKit_InterfaceIdentifier_RpcCallEvent,
    SilKit_InterfaceIdentifier_RpcCallResultEvent,
    SilKit_InterfaceIdentifier_RpcDiscoveryResultList,
    SilKit_InterfaceIdentifier_ParticipantStatus,
};
constexpr auto allSilkidIdsSize = sizeof(allSilkidIds) / sizeof(SilKit_InterfaceIdentifier);

TEST(TestCapi_Interfaces, interface_identifiers_are_unique)
{
    std::set<SilKit_InterfaceIdentifier> all{std::begin(allSilkidIds), std::end(allSilkidIds)};
    ASSERT_EQ(allSilkidIdsSize, all.size()) << "The IDs must not be duplicate";
}

}//namespace
