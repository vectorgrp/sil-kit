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

    class Test_CapiSilKit : public testing::Test
    {
    public: 
        SilKit::Core::Tests::DummyParticipant mockParticipant;
        Test_CapiSilKit()
        {
            
        }
    };

    TEST_F(Test_CapiSilKit, silkit_function_mapping)
    {
        SilKit_ReturnCode returnCode;

        SilKit_ParticipantConfiguration* participantConfiguration = nullptr;
        returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfiguration, SILKIT_CONFIG_STRING);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
        EXPECT_NE(participantConfiguration, nullptr);

        SilKit_ParticipantConfiguration* participantConfigurationFromFile = nullptr;
        returnCode = SilKit_ParticipantConfiguration_FromFile(&participantConfigurationFromFile, "ParticipantConfiguration_FullIncludes.yaml");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
        EXPECT_NE(participantConfigurationFromFile, nullptr);

        SilKit_Participant* participant = nullptr;
        returnCode = SilKit_Participant_Create(&participant, participantConfiguration, "Participant1", "42");
        // since there is no SIL Kit Registry, the call should fail
        EXPECT_EQ(returnCode, SilKit_ReturnCode_UNSPECIFIEDERROR);
        EXPECT_TRUE(participant == nullptr);

        // since there is no SIL Kit Registry with which one could create a Participant, we check against nullptr
        returnCode = SilKit_Participant_Destroy(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        // destory the participant configuration to satisfy ASAN
        returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfiguration);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
        returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfigurationFromFile);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    }


    TEST_F(Test_CapiSilKit, silkit_bad_params)
    {
        SilKit_ReturnCode returnCode;

        SilKit_ParticipantConfiguration* participantConfiguration = nullptr;

        returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfiguration, SILKIT_CONFIG_STRING);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
        EXPECT_NE(participantConfiguration, nullptr);

        SilKit_ParticipantConfiguration* participantConfigurationFromAFile = nullptr;
        returnCode = SilKit_ParticipantConfiguration_FromFile(&participantConfigurationFromAFile, "ParticipantConfiguration_FullIncludes.yaml");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
        EXPECT_NE(participantConfigurationFromAFile, nullptr);
        returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfigurationFromAFile);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);


        SilKit_Participant* participant = nullptr;
        returnCode = SilKit_Participant_Create(nullptr, participantConfiguration, "Participant1", "42");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Participant_Create(&participant, nullptr, "Participant1", "42");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Participant_Create(&participant, participantConfiguration, nullptr, "42");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_Participant_Create(&participant, participantConfiguration, "Participant1", nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        // Bad Parameter ParticipantConfiguration_FromString
        returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfiguration, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_ParticipantConfiguration_FromString(nullptr, SILKIT_CONFIG_STRING);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        // Bad Parameter ParticipantConfiguration_FromFile
        returnCode = SilKit_ParticipantConfiguration_FromFile(&participantConfigurationFromAFile, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        returnCode = SilKit_ParticipantConfiguration_FromFile(nullptr, "ParticipantConfiguration_FullIncludes.yaml");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode =
            SilKit_Participant_Create(&participant, participantConfiguration, "ParticipantNotExisting", "42");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_UNSPECIFIEDERROR);

        returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfiguration);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

        returnCode = SilKit_ParticipantConfiguration_Destroy(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        participantConfiguration = nullptr;

        returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfiguration, SILKIT_MALFORMED_CONFIG_STRING);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_UNSPECIFIEDERROR);
        EXPECT_EQ(participantConfiguration, nullptr);

        returnCode = SilKit_ParticipantConfiguration_FromFile(&participantConfiguration, "this_file_does_not_exist.yaml");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_UNSPECIFIEDERROR);
        EXPECT_EQ(participantConfiguration, nullptr);

        // since there is no SIL Kit Registry with which one could create a Participant, we check against nullptr
        returnCode = SilKit_Participant_Destroy(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    }

}
