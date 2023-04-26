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

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "MockCapiTest.hpp"

namespace {

using testing::DoAll;
using testing::SetArgPointee;
using testing::StrEq;
using testing::Return;

class HourglassVendorTest : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_ParticipantConfiguration* mockParticipantConfiguration{
        reinterpret_cast<SilKit_ParticipantConfiguration*>(uintptr_t(0x87654321))};
    SilKit_Vendor_Vector_SilKitRegistry* mockRegistry{
        reinterpret_cast<SilKit_Vendor_Vector_SilKitRegistry*>(uintptr_t(0x12345678))};
    SilKit_Logger* mockLogger{reinterpret_cast<SilKit_Logger*>(uintptr_t(0x21436587))};

    HourglassVendorTest()
    {
        using testing::_;
        ON_CALL(capi, SilKit_ParticipantConfiguration_FromString(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockParticipantConfiguration), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Vendor_Vector_SilKitRegistry_Create(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockRegistry), Return(SilKit_ReturnCode_SUCCESS)));
        ON_CALL(capi, SilKit_Vendor_Vector_SilKitRegistry_GetLogger(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockLogger), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

// LifecycleService

TEST_F(HourglassVendorTest, SilKit_Vendor_Vector_SilKitRegistry_Create_Destroy)
{
    auto cppParticipantConfiguration =
        std::make_shared<SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Config::ParticipantConfiguration>(
            mockParticipantConfiguration);

    EXPECT_CALL(capi, SilKit_Vendor_Vector_SilKitRegistry_Create(testing::_, mockParticipantConfiguration));
    EXPECT_CALL(capi, SilKit_Vendor_Vector_SilKitRegistry_Destroy(mockRegistry));

    auto cppRegistry =
        SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Vendor::Vector::CreateSilKitRegistry(cppParticipantConfiguration);
    // resetting the unique_ptr must destroy the registry object
    cppRegistry.reset();
}

TEST_F(HourglassVendorTest, SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Vendor::Vector::SilKitRegistry registry{mockRegistry};

    EXPECT_CALL(capi,
                SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler(mockRegistry, testing::_, testing::_));

    registry.SetAllDisconnectedHandler([] {
        // do nothing
    });
}

TEST_F(HourglassVendorTest, SilKit_Vendor_Vector_SilKitRegistry_GetLogger)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Vendor::Vector::SilKitRegistry registry{mockRegistry};

    EXPECT_CALL(capi, SilKit_Vendor_Vector_SilKitRegistry_GetLogger(testing::_, mockRegistry));

    registry.GetLogger();
}

TEST_F(HourglassVendorTest, SilKit_Vendor_Vector_SilKitRegistry_StartListening)
{
    const std::string listenUri{"silkit://127.0.0.1:0"};
    const std::string mockRegistryUri{"silkit://127.0.0.1:12345"};

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME::Impl::Vendor::Vector::SilKitRegistry registry{mockRegistry};

    EXPECT_CALL(capi, SilKit_Vendor_Vector_SilKitRegistry_StartListening(mockRegistry, StrEq(listenUri), testing::_))
        .WillOnce(DoAll(SetArgPointee<2>(mockRegistryUri.c_str()), Return(SilKit_ReturnCode_SUCCESS)));

    EXPECT_EQ(registry.StartListening(listenUri), mockRegistryUri);
}

} //namespace
