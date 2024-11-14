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

SilKit_ReturnCode throw_SilKitError()
try
{
    throw SilKit::SilKitError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode throw_TypeConversionError()
try
{
    throw SilKit::TypeConversionError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode throw_ConfigurationError()
try
{
    throw SilKit::ConfigurationError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode throw_StateError()
try
{
    throw SilKit::StateError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode throw_ProtocolError()
try
{
    throw SilKit::ProtocolError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode throw_AssertionError()
try
{
    throw SilKit::AssertionError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode throw_ExtensionError()
try
{
    throw SilKit::ExtensionError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode throw_LogicError()
try
{
    throw SilKit::LogicError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode throw_LengthError()
try
{
    throw SilKit::LengthError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS

SilKit_ReturnCode throw_OutOfRangeError()
try
{
    throw SilKit::OutOfRangeError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode throw_CapiBadParameterError()
try
{
    throw SilKit::CapiBadParameterError{"error msg"};
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode throw_std_runtime_error()
try
{
    throw std::runtime_error{"error msg"};
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode throw_std_exception()
try
{
    throw std::exception();
}
CAPI_CATCH_EXCEPTIONS


class UnknownException : public std::exception
{
};
SilKit_ReturnCode throw_unknown_exception()
try
{
    throw UnknownException();
}
CAPI_CATCH_EXCEPTIONS

// Each catch branch of CapiImpl.hpp CAPI_CATCH_EXCEPTIONS is tested for the expected return code
TEST_F(Test_CapiExceptions, catch_exception_macro)
{
    EXPECT_EQ(throw_CapiBadParameterError(), SilKit_ReturnCode_BADPARAMETER);

    EXPECT_EQ(throw_TypeConversionError(), SilKit_ReturnCode_TYPECONVERSION_ERROR);
    EXPECT_EQ(throw_ConfigurationError(), SilKit_ReturnCode_CONFIGURATION_ERROR);
    EXPECT_EQ(throw_StateError(), SilKit_ReturnCode_WRONGSTATE);
    EXPECT_EQ(throw_ProtocolError(), SilKit_ReturnCode_PROTOCOL_ERROR);
    EXPECT_EQ(throw_AssertionError(), SilKit_ReturnCode_ASSERTION_ERROR);
    EXPECT_EQ(throw_ExtensionError(), SilKit_ReturnCode_EXTENSION_ERROR);
    EXPECT_EQ(throw_LogicError(), SilKit_ReturnCode_LOGIC_ERROR);
    EXPECT_EQ(throw_LengthError(), SilKit_ReturnCode_LENGTH_ERROR);
    EXPECT_EQ(throw_OutOfRangeError(), SilKit_ReturnCode_OUTOFRANGE_ERROR);

    EXPECT_EQ(throw_SilKitError(), SilKit_ReturnCode_UNSPECIFIEDERROR);
    EXPECT_EQ(throw_std_runtime_error(), SilKit_ReturnCode_UNSPECIFIEDERROR);
    EXPECT_EQ(throw_std_exception(), SilKit_ReturnCode_UNSPECIFIEDERROR);
    EXPECT_EQ(throw_unknown_exception(), SilKit_ReturnCode_UNSPECIFIEDERROR);
}

// Test that the C-API return code results in the correct exception
TEST_F(Test_CapiExceptions, throw_on_error)
{
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_TYPECONVERSION_ERROR),
                 SilKit::TypeConversionError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_CONFIGURATION_ERROR), SilKit::ConfigurationError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_WRONGSTATE), SilKit::StateError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_PROTOCOL_ERROR), SilKit::ProtocolError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_ASSERTION_ERROR), SilKit::AssertionError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_EXTENSION_ERROR), SilKit::ExtensionError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_LOGIC_ERROR), SilKit::LogicError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_LENGTH_ERROR), SilKit::LengthError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_OUTOFRANGE_ERROR), SilKit::OutOfRangeError);
    EXPECT_THROW(SilKit::_detail_v1::Impl::ThrowOnError(SilKit_ReturnCode_UNSPECIFIEDERROR), SilKit::SilKitError);
}


} // namespace
