// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/mw/all.hpp"

#include "MockComAdapter.hpp"

namespace {
        const char* IB_CONFIG_STRING = "{"
"            \"ConfigVersion\": \"0.0.1\","
"            \"ConfigName\" : \"ConfigDemo\","
"            \"Description\" : \"Sample configuration for testing puposes\","
""
"            \"SimulationSetup\" : {"
""
"            \"Participants\": ["
"            {"
"                \"Name\": \"Participant1\","
"                \"Description\" : \"Demo Participant with nothing going on\","
"            }"
"            ],"
""
"        },"
""
"            \"MiddlewareConfig\": {"
"            \"ActiveMiddleware\": \"VAsio\""
"        }"
""
"    }";

const char* IB_MALFORMED_CONFIG_STRING = ""
"            \"ConfigVersion\": \"0.0.1\","
"            \"ConfigName\" : \"ConfigDemo\","
"            \"Description\" : \"Sample configuration for testing puposes\","
""
"            \"SimulationSetup\" : {"
""
"            \"Participants\": ["
"            {"
"                \"Name\": \"Participant1\","
"                \"Description\" : \"Demo Participant with nothing going on\","
"            }"
"            ],"
""
"        },"
""
"            \"MiddlewareConfig\": {"
"            \"ActiveMiddleware\": \"VAsio\""
"        }"
""
"    }";


    using namespace ib::sim::can;

    using ib::mw::test::DummyComAdapter;

	class CapiIntegrationBusTest : public testing::Test
	{
	public: 
        ib::mw::test::DummyComAdapter mockComAdapter;
        CapiIntegrationBusTest()
		{
			
		}
	};

    TEST_F(CapiIntegrationBusTest, integration_bus_function_mapping)
    {
        ib_ReturnCode returnCode;

        ib_SimulationParticipant* participant = nullptr;
        returnCode = ib_SimulationParticipant_create(&participant, IB_CONFIG_STRING, "Participant1", "42");
        // since there is no IbRegistry, the call should fail
        EXPECT_EQ(returnCode, ib_ReturnCode_UNSPECIFIEDERROR);
        EXPECT_TRUE(participant == nullptr);

        // since there is no IbRegistry with which one could create a ComAdapter, we check against nullptr
        returnCode = ib_SimulationParticipant_destroy(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }


    TEST_F(CapiIntegrationBusTest, integration_bus_bad_params)
    {
        ib_ReturnCode returnCode;

        ib_SimulationParticipant* participant = nullptr;
        returnCode = ib_SimulationParticipant_create(nullptr, IB_CONFIG_STRING, "Participant1", "42");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_SimulationParticipant_create(&participant, nullptr, "Participant1", "42");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_SimulationParticipant_create(&participant, IB_CONFIG_STRING, nullptr, "42");
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_SimulationParticipant_create(&participant, IB_CONFIG_STRING, "Participant1", nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode = ib_SimulationParticipant_create(&participant, IB_MALFORMED_CONFIG_STRING, "Participant1", "42");
        EXPECT_EQ(returnCode, ib_ReturnCode_UNSPECIFIEDERROR);

        returnCode = ib_SimulationParticipant_create(&participant, IB_CONFIG_STRING, "ParticipantNotExisting", "42");
        EXPECT_EQ(returnCode, ib_ReturnCode_UNSPECIFIEDERROR);

        // since there is no IbRegistry with which one could create a ComAdapter, we check against nullptr
        returnCode = ib_SimulationParticipant_destroy(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

}