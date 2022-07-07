// Copyright (c) Vector Informatik GmbH. All rights reserved.

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

}//namespace
