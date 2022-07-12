// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"

#include "MockParticipant.hpp"

namespace {
const auto SILKIT_CONFIG_STRING = R"aw(
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

const auto SILKIT_MALFORMED_CONFIG_STRING = R"aw(
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


    using namespace SilKit::Services::Can;

    using SilKit::Core::Tests::DummyParticipant;

    class CapiSilKitTest : public testing::Test
    {
    public: 
        SilKit::Core::Tests::DummyParticipant mockParticipant;
        CapiSilKitTest()
        {
            
        }
    };

    TEST_F(CapiSilKitTest, silkit_function_mapping)
    {
        SilKit_ReturnCode returnCode;

        SilKit_Participant* participant = nullptr;
        returnCode = SilKit_Participant_Create(&participant, SILKIT_CONFIG_STRING, "Participant1", "42", SilKit_False);
        // since there is no SIL Kit Registry, the call should fail
        EXPECT_EQ(returnCode, SilKit_ReturnCode_UNSPECIFIEDERROR);
        EXPECT_TRUE(participant == nullptr);

        // since there is no SIL Kit Registry with which one could create a Participant, we check against nullptr
        returnCode = SilKit_Participant_Destroy(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    }


    TEST_F(CapiSilKitTest, silkit_bad_params)
    {
        SilKit_ReturnCode returnCode;

        SilKit_Participant* participant = nullptr;
        returnCode = SilKit_Participant_Create(nullptr, SILKIT_CONFIG_STRING, "Participant1", "42", SilKit_False);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Participant_Create(&participant, nullptr, "Participant1", "42", SilKit_False);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Participant_Create(&participant, SILKIT_CONFIG_STRING, nullptr, "42", SilKit_False);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Participant_Create(&participant, SILKIT_CONFIG_STRING, "Participant1", nullptr, SilKit_False);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode =
            SilKit_Participant_Create(&participant, SILKIT_MALFORMED_CONFIG_STRING, "Participant1", "42", SilKit_False);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_UNSPECIFIEDERROR);

        returnCode =
            SilKit_Participant_Create(&participant, SILKIT_CONFIG_STRING, "ParticipantNotExisting", "42", SilKit_False);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_UNSPECIFIEDERROR);

        // since there is no SIL Kit Registry with which one could create a Participant, we check against nullptr
        returnCode = SilKit_Participant_Destroy(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    }

}
