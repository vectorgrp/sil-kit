// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"

#include "MockParticipant.hpp"

namespace {

using SilKit::Core::Tests::DummyParticipant;

class MockParticipant : public SilKit::Core::Tests::DummyParticipant
{
public:
    MOCK_METHOD(std::string, GetParameter, (SilKit::Parameter /*parameter*/), (override));
};

class Test_CapiGetParameter : public testing::Test
{
public:
    MockParticipant mockParticipant;
    Test_CapiGetParameter() {}
};

TEST_F(Test_CapiGetParameter, getparameter_bad_params)
{
    SilKit_ReturnCode returnCode;
    auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
    char* parameterValue{nullptr};
    size_t parameterSize;

    returnCode =
        SilKit_Participant_GetParameter(nullptr, &parameterSize, SilKit_Parameter_ParticipantName, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode =
        SilKit_Participant_GetParameter(parameterValue, nullptr, SilKit_Parameter_ParticipantName, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode =
        SilKit_Participant_GetParameter(parameterValue, &parameterSize, SilKit_Parameter_ParticipantName, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiGetParameter, getparameter_function_mapping)
{
    SilKit_ReturnCode returnCode;
    auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
    char* parameterValue{nullptr};
    size_t parameterSize;

    EXPECT_CALL(mockParticipant, GetParameter(SilKit::Parameter::ParticipantName)).Times(testing::Exactly(1));
    returnCode = SilKit_Participant_GetParameter(parameterValue, & parameterSize, SilKit_Parameter_ParticipantName,
                                                 cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

} // namespace
