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
