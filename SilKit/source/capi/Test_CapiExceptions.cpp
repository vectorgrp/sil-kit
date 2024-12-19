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
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_CONFIGURATIONERROR), SilKit::ConfigurationError);
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
