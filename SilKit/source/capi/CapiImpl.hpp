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

#pragma once

#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/participant/exception.hpp"

#include "CapiExceptions.hpp"
#include "TypeUtils.hpp"

#define CAPI_CATCH_EXCEPTIONS \
    catch (const SilKit::CapiBadParameterError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_BADPARAMETER; \
    } \
    catch (const SilKit::StateError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_WRONGSTATE; \
    } \
    catch (const SilKit::SilKitError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_UNSPECIFIEDERROR; \
    } \
    catch (const std::runtime_error& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_UNSPECIFIEDERROR; \
    } \
    catch (const std::exception& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_UNSPECIFIEDERROR; \
    } \
    catch (...) \
    { \
        return SilKit_ReturnCode_UNSPECIFIEDERROR; \
    }

#define ASSERT_VALID_POINTER_PARAMETER(p) \
    do \
    { \
        if (p == nullptr) \
        { \
            throw SilKit::CapiBadParameterError{"Parameter '" #p "' must not be null."}; \
        } \
    } \
    while (false)

#define ASSERT_VALID_POINTER_TO_POINTER_PARAMETER(p) \
    do \
    { \
        if (p == nullptr) \
        { \
            throw SilKit::CapiBadParameterError{"Parameter '" #p "' must not be null."}; \
        } \
        if (*p == nullptr) \
        { \
            throw SilKit::CapiBadParameterError{"Parameter '" #p "' must not point to a null value."}; \
        } \
    } \
    while (false)

#define ASSERT_VALID_OUT_PARAMETER(p) \
    do \
    { \
        if (p == nullptr) \
        { \
            throw SilKit::CapiBadParameterError{"Return parameter '" #p "' must not be null."}; \
        } \
    } \
    while (false)

#define kInvalidFunctionPointer "Handler function parameter must not be null."

#define ASSERT_VALID_HANDLER_PARAMETER(handler) \
    do \
    { \
        if (handler == nullptr) \
        { \
            throw SilKit::CapiBadParameterError{kInvalidFunctionPointer}; \
        } \
    } \
    while (false)

#define ASSERT_VALID_BOOL_PARAMETER(b) \
    do \
    { \
        if (!(b == SilKit_True || b == SilKit_False)) \
        { \
            throw SilKit::CapiBadParameterError{"The parameter '" #b "' is not a valid SilKit_Bool."}; \
        } \
    } \
    while (false)

#define ASSERT_VALID_STRUCT_HEADER(p) \
    do \
    { \
        if (!HasValidStructHeader(p)) \
        { \
            throw SilKit::CapiBadParameterError{"The parameter '" #p \
                                                "' has no valid SilKit_StructHeader. Check your library version"}; \
        } \
    } \
    while (false)


extern thread_local std::string SilKit_error_string;


// Utility to verify a CAPI struct header

namespace detail
{
    template<typename T, typename = void>
    struct HasStructHeader: std::false_type
    {
    };

    template <typename T>
    struct HasStructHeader<T, SilKit::Util::VoidT<decltype(std::declval<std::decay_t<T>>().structHeader = SilKit_StructHeader{})>>
    : std::true_type
    {
    };
} //namespace detail

template<typename StructT>
bool HasValidStructHeader(const StructT*, std::enable_if_t<!detail::HasStructHeader<StructT>::value, bool> = false)
{
    //struct type has no header, ignored.
   return false; 
}

template<typename StructT>
bool HasValidStructHeader(const StructT* s, std::enable_if_t<detail::HasStructHeader<StructT>::value, bool> = true)
{
    return SK_ID_IS_VALID(SilKit_Struct_GetId(*s));
}
