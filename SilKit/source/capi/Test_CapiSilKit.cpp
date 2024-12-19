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

const auto SILKIT_CONFIG_STRING = R"(
{
    "ParticipantName": "Participant1", 
    "Logging":{
        "Sinks":[{"Type":"Stdout","Level":"Info"}]
    }
})";

const auto SILKIT_MALFORMED_CONFIG_STRING = R"(
{
{
{
    "ParticipantName: "Participant1", 
    "Logging": {
        "Sinks":[{"Type":"Stdout","Level":"Info"}]
    }
}
)";

using namespace SilKit::Services::Can;
using SilKit::Core::Tests::DummyParticipant;

class Test_CapiSilKit : public testing::Test
{
public:
    SilKit::Core::Tests::DummyParticipant mockParticipant;
    Test_CapiSilKit() {}
};

TEST_F(Test_CapiSilKit, silkit_bad_params)
{
    SilKit_ReturnCode returnCode;
    
    {
        // Bad / Invalid Parameter ParticipantConfiguration_FromString
        SilKit_ParticipantConfiguration* participantConfigFromString = nullptr;
        returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfigFromString, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        EXPECT_TRUE(participantConfigFromString == nullptr);

        returnCode = SilKit_ParticipantConfiguration_FromString(nullptr, SILKIT_CONFIG_STRING);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode =
            SilKit_ParticipantConfiguration_FromString(&participantConfigFromString, SILKIT_MALFORMED_CONFIG_STRING);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_CONFIGURATIONERROR);
        EXPECT_EQ(participantConfigFromString, nullptr);
    }

    {
        // Bad / Invalid Parameter ParticipantConfiguration_FromFile
        SilKit_ParticipantConfiguration* participantConfigFromFile = nullptr;
        returnCode = SilKit_ParticipantConfiguration_FromFile(&participantConfigFromFile, nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        EXPECT_TRUE(participantConfigFromFile == nullptr);

        returnCode = SilKit_ParticipantConfiguration_FromFile(nullptr, "ParticipantConfiguration_TestCapi.yaml");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode =
            SilKit_ParticipantConfiguration_FromFile(&participantConfigFromFile, "this_file_does_not_exist.yaml");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_CONFIGURATIONERROR);
        EXPECT_TRUE(participantConfigFromFile == nullptr);
    }

    {
        // Bad / Invalid Parameter SilKit_Participant_Create

        // Create valid configuration FromString
        SilKit_ParticipantConfiguration* participantConfigFromString = nullptr;
        returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfigFromString, SILKIT_CONFIG_STRING);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
        EXPECT_NE(participantConfigFromString, nullptr);

        // Create valid configuration FromFile
        SilKit_ParticipantConfiguration* participantConfigFromFile = nullptr;
        returnCode = SilKit_ParticipantConfiguration_FromFile(&participantConfigFromFile,
                                                              "ParticipantConfiguration_TestCapi.yaml");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
        EXPECT_NE(participantConfigFromFile, nullptr);

        SilKit_Participant* participant = nullptr;
        returnCode =
            SilKit_Participant_Create(nullptr, participantConfigFromString, "Participant1", "silkit://localhost:7");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

        returnCode = SilKit_Participant_Create(&participant, nullptr, "Participant1", "silkit://localhost:7");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        EXPECT_TRUE(participant == nullptr);

        returnCode =
            SilKit_Participant_Create(&participant, participantConfigFromFile, nullptr, "silkit://localhost:7");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        EXPECT_TRUE(participant == nullptr);

        returnCode = SilKit_Participant_Create(&participant, participantConfigFromString, "Participant1", nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
        EXPECT_TRUE(participant == nullptr);

        // There is no SIL Kit Registry running on port 7, the call should fail
        returnCode =
            SilKit_Participant_Create(&participant, participantConfigFromFile, "Participant1", "silkit://localhost:7");
        EXPECT_EQ(returnCode, SilKit_ReturnCode_UNSPECIFIEDERROR);
        EXPECT_TRUE(participant == nullptr);

        // Clean up the configs
        returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfigFromFile);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
        returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfigFromString);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    }
    
    {
        // Bad Parameter SilKit_Participant_Destroy
        returnCode = SilKit_Participant_Destroy(nullptr);
        EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
    }
}

} // namespace
