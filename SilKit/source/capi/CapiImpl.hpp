// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/participant/exception.hpp"

#include "CapiExceptions.hpp"

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
    catch (const SilKit::TypeConversionError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_TYPECONVERSIONERROR; \
    } \
    catch (const SilKit::ConfigurationError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_CONFIGURATIONERROR; \
    } \
    catch (const SilKit::ProtocolError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_PROTOCOLERROR; \
    } \
    catch (const SilKit::AssertionError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_ASSERTIONERROR; \
    } \
    catch (const SilKit::ExtensionError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_EXTENSIONERROR; \
    } \
    catch (const SilKit::LengthError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_LENGTHERROR; \
    } \
    catch (const SilKit::OutOfRangeError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_OUTOFRANGEERROR; \
    } \
    catch (const SilKit::LogicError& e) \
    { \
        SilKit_error_string = e.what(); \
        return SilKit_ReturnCode_LOGICERROR; \
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
