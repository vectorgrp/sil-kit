// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"
#include "silkit/SilKitVersion.hpp"

#include "MockCapiTest.hpp"

namespace {

using testing::DoAll;
using testing::SetArgPointee;
using testing::Return;

class Test_HourglassVersion : public SilKitHourglassTests::MockCapiTest
{
public:
    const uint32_t mockVersionMajor{1};
    const uint32_t mockVersionMinor{2};
    const uint32_t mockVersionPatch{3};
    const uint32_t mockVersionBuildNumber{4};
    const std::string mockVersionString{"VersionString"};
    const std::string mockVersionSuffix{"VersionSuffix"};
    const std::string mockVersionGitHash{"GitHash"};

    Test_HourglassVersion()
    {
        using testing::_;
        ON_CALL(capi, SilKit_Version_Major(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mockVersionMajor), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Version_Minor(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mockVersionMinor), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Version_Patch(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mockVersionPatch), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Version_BuildNumber(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mockVersionBuildNumber), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Version_String(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mockVersionString.c_str()), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Version_VersionSuffix(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mockVersionSuffix.c_str()), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Version_GitHash(_))
            .WillByDefault(DoAll(SetArgPointee<0>(mockVersionGitHash.c_str()), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

TEST_F(Test_HourglassVersion, SilKit_Version_Major)
{
    EXPECT_CALL(capi, SilKit_Version_Major(testing::_));

    EXPECT_EQ(SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::Major(), mockVersionMajor);
}

TEST_F(Test_HourglassVersion, SilKit_Version_Minor)
{
    EXPECT_CALL(capi, SilKit_Version_Minor(testing::_));

    EXPECT_EQ(SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::Minor(), mockVersionMinor);
}

TEST_F(Test_HourglassVersion, SilKit_Version_Patch)
{
    EXPECT_CALL(capi, SilKit_Version_Patch(testing::_));

    EXPECT_EQ(SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::Patch(), mockVersionPatch);
}

TEST_F(Test_HourglassVersion, SilKit_Version_BuildNumber)
{
    EXPECT_CALL(capi, SilKit_Version_BuildNumber(testing::_));

    EXPECT_EQ(SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::BuildNumber(), mockVersionBuildNumber);
}

TEST_F(Test_HourglassVersion, SilKit_Version_String)
{
    EXPECT_CALL(capi, SilKit_Version_String(testing::_));

    EXPECT_EQ(SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::String(), mockVersionString);
}

TEST_F(Test_HourglassVersion, SilKit_Version_VersionSuffix)
{
    EXPECT_CALL(capi, SilKit_Version_VersionSuffix(testing::_));

    EXPECT_EQ(SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::VersionSuffix(), mockVersionSuffix);
}

TEST_F(Test_HourglassVersion, SilKit_Version_GitHash)
{
    EXPECT_CALL(capi, SilKit_Version_GitHash(testing::_));

    EXPECT_EQ(SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Version::GitHash(), mockVersionGitHash);
}

} //namespace
