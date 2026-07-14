// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "silkit/capi/SilKit.h"

#include "MockParticipant.hpp"

namespace {

using SilKit::Core::Tests::DummyParticipant;

class MockParticipant : public SilKit::Core::Tests::DummyParticipant{};

class Test_CapiGetParameter : public testing::Test
{
public:
    MockParticipant mockParticipant;
    Test_CapiGetParameter() {}
};

TEST_F(Test_CapiGetParameter, getparticipantname_bad_params)
{
    SilKit_ReturnCode returnCode;
    auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
    char* parameterValue{nullptr};
    size_t parameterSize;

    returnCode = SilKit_Participant_GetParticipantName(nullptr, &parameterSize, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Participant_GetParticipantName(parameterValue, nullptr, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetParticipantName(parameterValue, &parameterSize, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiGetParameter, getregistryuri_bad_params)
{
    SilKit_ReturnCode returnCode;
    auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
    char* parameterValue{nullptr};
    size_t parameterSize;

    returnCode = SilKit_Participant_GetRegistryUri(nullptr, &parameterSize, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Participant_GetRegistryUri(parameterValue, nullptr, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Participant_GetRegistryUri(parameterValue, &parameterSize, nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

TEST_F(Test_CapiGetParameter, getparticipantname_returns_required_size_and_null_terminated_value)
{
    auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
    const std::string value{"MockParticipant"};

    size_t parameterSize{0};
    auto returnCode = SilKit_Participant_GetParticipantName(nullptr, &parameterSize, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(parameterSize, value.size() + 1);

    char buffer[32]{};
    auto bufferSize = sizeof(buffer);
    returnCode = SilKit_Participant_GetParticipantName(buffer, &bufferSize, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(bufferSize, value.size() + 1);
    EXPECT_STREQ(buffer, value.c_str());
}

TEST_F(Test_CapiGetParameter, getregistryuri_returns_required_size_and_null_terminated_value)
{
    auto cMockParticipant = (SilKit_Participant*)&mockParticipant;
    const std::string value{"silkit://mock.participant.silkit:0"};

    size_t parameterSize{0};
    auto returnCode = SilKit_Participant_GetRegistryUri(nullptr, &parameterSize, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(parameterSize, value.size() + 1);

    char buffer[64]{};
    auto bufferSize = sizeof(buffer);
    returnCode = SilKit_Participant_GetRegistryUri(buffer, &bufferSize, cMockParticipant);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(bufferSize, value.size() + 1);
    EXPECT_STREQ(buffer, value.c_str());
}

TEST_F(Test_CapiGetParameter, getparticipantname_truncates_and_still_null_terminates)
{
    auto cMockParticipant = (SilKit_Participant*)&mockParticipant;

    char buffer[5]{};
    size_t bufferSize = sizeof(buffer);
    const auto returnCode = SilKit_Participant_GetParticipantName(buffer, &bufferSize, cMockParticipant);

    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
    EXPECT_EQ(buffer[sizeof(buffer) - 1], '\0');
}

} // namespace
