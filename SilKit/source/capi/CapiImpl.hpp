// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/participant/exception.hpp"

#include "capi/CapiExceptions.hpp"

#include <string_view>

#include "fmt/format.h"


namespace VSilKit {

void ApiTraceEventImpl(std::string_view func, std::string_view data);

template <typename... Args>
void ApiTraceEvent(const std::string_view func, Args&&... args)
{
    thread_local std::string data;

    data.clear();
    fmt::format_to(std::back_inserter(data), std::forward<Args>(args)...);

    ApiTraceEventImpl(func, data);
}

} // namespace VSilKit

#ifdef SILKIT_ENABLE_API_TRACING_INSTRUMENTATION

#define VSILKIT_API_TRACE(...) ::VSilKit::ApiTraceEvent((__func__), __VA_ARGS__)

#else

#define VSILKIT_API_TRACE(...) \
do \
{ \
} while (false)

#endif


#define CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(ErrorType, ReturnCode) \
    catch (const ErrorType& e) \
    { \
        SilKit_error_string = e.what(); \
        VSILKIT_API_TRACE("ERROR {}", #ReturnCode); \
        return ReturnCode; \
    }


#define CAPI_CATCH_EXCEPTIONS \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::CapiBadParameterError, SilKit_ReturnCode_BADPARAMETER) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::StateError, SilKit_ReturnCode_WRONGSTATE) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::TypeConversionError, SilKit_ReturnCode_TYPECONVERSIONERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::ConfigurationError, SilKit_ReturnCode_CONFIGURATIONERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::ProtocolError, SilKit_ReturnCode_PROTOCOLERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::AssertionError, SilKit_ReturnCode_ASSERTIONERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::ExtensionError, SilKit_ReturnCode_EXTENSIONERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::LengthError, SilKit_ReturnCode_LENGTHERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::OutOfRangeError, SilKit_ReturnCode_OUTOFRANGEERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::LogicError, SilKit_ReturnCode_LOGICERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(SilKit::SilKitError, SilKit_ReturnCode_UNSPECIFIEDERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(std::runtime_error, SilKit_ReturnCode_UNSPECIFIEDERROR) \
    CAPI_CATCH_EXCEPTIONS_CATCH_BLOCK(std::exception, SilKit_ReturnCode_UNSPECIFIEDERROR) \
    catch (...) \
    { \
        VSILKIT_API_TRACE("ERROR SilKit_ReturnCode_UNSPECIFIEDERROR"); \
        return SilKit_ReturnCode_UNSPECIFIEDERROR; \
    }

#define ASSERT_VALID_POINTER_PARAMETER(p) \
    do \
    { \
        if (p == nullptr) \
        { \
            throw SilKit::CapiBadParameterError{"Parameter '" #p "' must not be null."}; \
        } \
    } while (false)

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
    } while (false)

#define ASSERT_VALID_OUT_PARAMETER(p) \
    do \
    { \
        if (p == nullptr) \
        { \
            throw SilKit::CapiBadParameterError{"Return parameter '" #p "' must not be null."}; \
        } \
    } while (false)

#define kInvalidFunctionPointer "Handler function parameter must not be null."

#define ASSERT_VALID_HANDLER_PARAMETER(handler) \
    do \
    { \
        if (handler == nullptr) \
        { \
            throw SilKit::CapiBadParameterError{kInvalidFunctionPointer}; \
        } \
    } while (false)

#define ASSERT_VALID_BOOL_PARAMETER(b) \
    do \
    { \
        if (!(b == SilKit_True || b == SilKit_False)) \
        { \
            throw SilKit::CapiBadParameterError{"The parameter '" #b "' is not a valid SilKit_Bool."}; \
        } \
    } while (false)

#define ASSERT_VALID_STRUCT_HEADER(p) \
    do \
    { \
        if (!HasValidStructHeader(p)) \
        { \
            throw SilKit::CapiBadParameterError{"The parameter '" #p \
                                                "' has no valid SilKit_StructHeader. Check your library version"}; \
        } \
    } while (false)

#define ASSERT_VALID_PLAIN_STRUCT_HEADER(p) \
    do \
    { \
        if (!IsValidStructHeader(p)) \
        { \
            throw SilKit::CapiBadParameterError{"The parameter '" #p "' is not a valid SilKit_StructHeader."}; \
        } \
    } while (false)


extern thread_local std::string SilKit_error_string;


// Utility to verify a CAPI struct header

namespace detail {
template <typename T, typename = void>
struct HasStructHeader : std::false_type
{
};

template <typename T>
struct HasStructHeader<T, std::void_t<decltype(std::declval<std::decay_t<T>>().structHeader = SilKit_StructHeader{})>>
    : std::true_type
{
};
} //namespace detail

template <typename StructT>
bool HasValidStructHeader(const StructT*, std::enable_if_t<!detail::HasStructHeader<StructT>::value, bool> = false)
{
    //struct type has no header, ignored.
    return false;
}

template <typename StructT>
bool HasValidStructHeader(const StructT* s, std::enable_if_t<detail::HasStructHeader<StructT>::value, bool> = true)
{
    return SK_ID_IS_VALID(SilKit_Struct_GetId(*s));
}

inline bool IsValidStructHeader(const SilKit_StructHeader* s)
{
    return SK_ID_IS_VALID(s->version);
}
