// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ib/capi/IntegrationBus.h"
#include "ib/mw/all.hpp"

#include "MockParticipant.hpp"

namespace {
const auto IB_CONFIG_STRING = R"aw(
{
    "ConfigVersion": "0.0.1",
    "ConfigName" : "ConfigDemo",
    "Description" : "Sample configuration for testing purposes",

    "SimulationSetup" : {
        "Participants": [
            {
                "Name": "Participant1",
                "Description" : "Demo Participant with nothing going on"
            }
        ]
    },

    "MiddlewareConfig": {
        "ActiveMiddleware": "VAsio"
    }
}
)aw";

const auto IB_MALFORMED_CONFIG_STRING = R"aw(
"ConfigVersion": "0.0.1","
"ConfigName" : "ConfigDemo","
"Description" : "Sample configuration for testing purposes",
"SimulationSetup" : {
    "Participants": [
        {
            "Name": "Participant1",
            "Description" : "Demo Participant with nothing going on",
        }
    ],
},
"MiddlewareConfig": {
    "ActiveMiddleware": "VAsio"
}
}
)aw";


    using namespace ib::sim::can;

    using ib::mw::test::DummyParticipant;

    class CapiIntegrationBusTest : public testing::Test
    {
    public: 
        ib::mw::test::DummyParticipant mockParticipant;
        CapiIntegrationBusTest()
        {
            
        }
    };

    TEST_F(CapiIntegrationBusTest, integration_bus_function_mapping)
    {
        ib_ReturnCode returnCode;

        ib_Participant* participant = nullptr;
        returnCode = ib_Participant_Create(&participant, IB_CONFIG_STRING, "Participant1", "42", ib_False);
        // since there is no IbRegistry, the call should fail
        EXPECT_EQ(returnCode, ib_ReturnCode_UNSPECIFIEDERROR);
        EXPECT_TRUE(participant == nullptr);

        // since there is no IbRegistry with which one could create a Participant, we check against nullptr
        returnCode = ib_Participant_Destroy(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }


    TEST_F(CapiIntegrationBusTest, integration_bus_bad_params)
    {
        ib_ReturnCode returnCode;

        ib_Participant* participant = nullptr;
        returnCode = ib_Participant_Create(nullptr, IB_CONFIG_STRING, "Participant1", "42", ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Participant_Create(&participant, nullptr, "Participant1", "42", ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Participant_Create(&participant, IB_CONFIG_STRING, nullptr, "42", ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
        returnCode = ib_Participant_Create(&participant, IB_CONFIG_STRING, "Participant1", nullptr, ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);

        returnCode =
            ib_Participant_Create(&participant, IB_MALFORMED_CONFIG_STRING, "Participant1", "42", ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_UNSPECIFIEDERROR);

        returnCode =
            ib_Participant_Create(&participant, IB_CONFIG_STRING, "ParticipantNotExisting", "42", ib_False);
        EXPECT_EQ(returnCode, ib_ReturnCode_UNSPECIFIEDERROR);

        // since there is no IbRegistry with which one could create a Participant, we check against nullptr
        returnCode = ib_Participant_Destroy(nullptr);
        EXPECT_EQ(returnCode, ib_ReturnCode_BADPARAMETER);
    }

}
