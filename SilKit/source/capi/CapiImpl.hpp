/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/participant/exception.hpp"

#include "TypeUtils.hpp"

#define CAPI_ENTER \
    try

#define CAPI_LEAVE \
    catch (const SilKit::StateError& e) { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_WRONGSTATE; \
    } \
    catch (const std::runtime_error& e) { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_UNSPECIFIEDERROR; \
    } \
    catch (const std::exception&) { \
        return SilKit_ReturnCode_UNSPECIFIEDERROR; \
    } \
    catch (...) { \
        return SilKit_ReturnCode_UNSPECIFIEDERROR; \
    } \

#define kInvalidFunctionPointer  "Handler function parameter must not be null."

#define ASSERT_VALID_POINTER_PARAMETER(p) \
    if (p == nullptr) \
    { \
        SilKit_error_string = "Parameter '" #p "' must not be null."; \
        return SilKit_ReturnCode_BADPARAMETER; \
    }
#define ASSERT_VALID_POINTER_TO_POINTER_PARAMETER(p) \
    if (p == nullptr) \
    { \
        SilKit_error_string = "Parameter '" #p "' must not be null."; \
        return SilKit_ReturnCode_BADPARAMETER; \
    } \
    if (*p == nullptr) \
    { \
        SilKit_error_string = "Parameter '" #p "' must not point to a null value."; \
        return SilKit_ReturnCode_BADPARAMETER; \
    }
#define ASSERT_VALID_OUT_PARAMETER(p) \
    if (p == nullptr) \
    { \
        SilKit_error_string = "Return parameter '" #p "' must not be null."; \
        return SilKit_ReturnCode_BADPARAMETER; \
    }
#define ASSERT_VALID_HANDLER_PARAMETER(handler) \
    if (handler == nullptr) \
    { \
        SilKit_error_string = kInvalidFunctionPointer; \
        return SilKit_ReturnCode_BADPARAMETER; \
    }

#define ASSERT_VALID_BOOL_PARAMETER(b) \
    if (!(b == SilKit_True || b == SilKit_False)) \
    { \
        SilKit_error_string = "The parameter '" #b "' is not a valid SilKit_Bool."; \
        return SilKit_ReturnCode_BADPARAMETER; \
    }

#define ASSERT_VALID_STRUCT_HEADER(p) \
    if (!HasValidStructHeader(p))\
    {\
        SilKit_error_string = "The parameter '" #p "' has no valid SilKit_StructHeader. Check your library version";\
        return SilKit_ReturnCode_BADPARAMETER;\
    }
    

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
