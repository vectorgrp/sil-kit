// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "SilKitVersionImpl.hpp"

#include "silkit/capi/SilKit.h"
#include "silkit/capi/Version.h"

namespace {

class Test_CapiVersion : public testing::Test
{
protected:
    Test_CapiVersion() {}
};

TEST_F(Test_CapiVersion, version_equality)
{
    SilKit_ReturnCode returnCode;
    uint32_t outNum;
    const char* outString;

    returnCode = SilKit_Version_Major(&outNum);
    EXPECT_EQ(outNum, SilKit::Version::MajorImpl());
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Version_Minor(&outNum);
    EXPECT_EQ(outNum, SilKit::Version::MinorImpl());
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Version_Patch(&outNum);
    EXPECT_EQ(outNum, SilKit::Version::PatchImpl());
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Version_BuildNumber(&outNum);
    EXPECT_EQ(outNum, SilKit::Version::BuildNumberImpl());
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Version_String(&outString);
    EXPECT_EQ(outString, SilKit::Version::StringImpl());
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Version_VersionSuffix(&outString);
    EXPECT_EQ(outString, SilKit::Version::VersionSuffixImpl());
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);

    returnCode = SilKit_Version_GitHash(&outString);
    EXPECT_EQ(outString, SilKit::Version::GitHashImpl());
    EXPECT_EQ(returnCode, SilKit_ReturnCode_SUCCESS);
}

TEST_F(Test_CapiVersion, version_nullpointer_params)
{
    SilKit_ReturnCode returnCode;

    returnCode = SilKit_Version_Major(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Version_Minor(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Version_Patch(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Version_BuildNumber(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Version_String(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Version_VersionSuffix(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);

    returnCode = SilKit_Version_GitHash(nullptr);
    EXPECT_EQ(returnCode, SilKit_ReturnCode_BADPARAMETER);
}

} //namespace
