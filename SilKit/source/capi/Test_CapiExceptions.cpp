// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CapiImpl.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"

namespace {

class Test_CapiExceptions : public testing::Test
{
public:
    Test_CapiExceptions() {}
};

class UnknownException : public std::exception
{
public:
    UnknownException() {}
    UnknownException(const char* /*message*/) {}
};

template <typename T>
SilKit_ReturnCode TestExceptionToErrorCode()
try
{
    throw T{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

// Not all compilers support std::exception with const char* initialization, so treat that std::exception separately
SilKit_ReturnCode TestStdExceptionToErrorCode()
try
{
    throw std::exception();
}
CAPI_CATCH_EXCEPTIONS


// Each catch branch of CapiImpl.hpp CAPI_CATCH_EXCEPTIONS is tested for the expected return code
TEST_F(Test_CapiExceptions, catch_exception_macro)
{
    EXPECT_EQ(TestExceptionToErrorCode<SilKit::CapiBadParameterError>(), SilKit_ReturnCode_BADPARAMETER);

    EXPECT_EQ(TestExceptionToErrorCode<SilKit::TypeConversionError>(), SilKit_ReturnCode_TYPECONVERSIONERROR);
    EXPECT_EQ(TestExceptionToErrorCode<SilKit::ConfigurationError>(), SilKit_ReturnCode_CONFIGURATIONERROR);
    EXPECT_EQ(TestExceptionToErrorCode<SilKit::StateError>(), SilKit_ReturnCode_WRONGSTATE);
    EXPECT_EQ(TestExceptionToErrorCode<SilKit::ProtocolError>(), SilKit_ReturnCode_PROTOCOLERROR);
    EXPECT_EQ(TestExceptionToErrorCode<SilKit::AssertionError>(), SilKit_ReturnCode_ASSERTIONERROR);
    EXPECT_EQ(TestExceptionToErrorCode<SilKit::ExtensionError>(), SilKit_ReturnCode_EXTENSIONERROR);
    EXPECT_EQ(TestExceptionToErrorCode<SilKit::LogicError>(), SilKit_ReturnCode_LOGICERROR);
    EXPECT_EQ(TestExceptionToErrorCode<SilKit::LengthError>(), SilKit_ReturnCode_LENGTHERROR);
    EXPECT_EQ(TestExceptionToErrorCode<SilKit::OutOfRangeError>(), SilKit_ReturnCode_OUTOFRANGEERROR);

    EXPECT_EQ(TestExceptionToErrorCode<SilKit::SilKitError>(), SilKit_ReturnCode_UNSPECIFIEDERROR);
    EXPECT_EQ(TestExceptionToErrorCode<std::runtime_error>(), SilKit_ReturnCode_UNSPECIFIEDERROR);
    EXPECT_EQ(TestExceptionToErrorCode<UnknownException>(), SilKit_ReturnCode_UNSPECIFIEDERROR);
    EXPECT_EQ(TestStdExceptionToErrorCode(), SilKit_ReturnCode_UNSPECIFIEDERROR);
}

// Test that the C-API return code results in the correct exception
TEST_F(Test_CapiExceptions, throw_on_error)
{
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_TYPECONVERSIONERROR),
                 SilKit::TypeConversionError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_CONFIGURATIONERROR),
                 SilKit::ConfigurationError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_WRONGSTATE), SilKit::StateError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_PROTOCOLERROR), SilKit::ProtocolError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_ASSERTIONERROR), SilKit::AssertionError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_EXTENSIONERROR), SilKit::ExtensionError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_LOGICERROR), SilKit::LogicError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_LENGTHERROR), SilKit::LengthError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_OUTOFRANGEERROR), SilKit::OutOfRangeError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_UNSPECIFIEDERROR), SilKit::SilKitError);
}


} // namespace
