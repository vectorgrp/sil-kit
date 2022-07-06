/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include "silkit/capi/Types.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

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
    }

#define kInvalidFunctionPointer  "Handler function parameter must not be null."

#define ASSERT_VALID_POINTER_PARAMETER(p)                     \
  if (p == nullptr)                                           \
  {                                                           \
    SilKit_error_string = "Parameter '" #p "' must not be null."; \
    return SilKit_ReturnCode_BADPARAMETER;                        \
  }
#define ASSERT_VALID_POINTER_TO_POINTER_PARAMETER(p)          \
  if (p == nullptr)                                           \
  {                                                           \
    SilKit_error_string = "Parameter '" #p "' must not be null."; \
    return SilKit_ReturnCode_BADPARAMETER;                        \
  }                                                           \
  if (*p == nullptr)                                          \
  {                                                           \
    SilKit_error_string =                                         \
      "Parameter '" #p "' must not point to a null value.";   \
    return SilKit_ReturnCode_BADPARAMETER;                        \
  }
#define ASSERT_VALID_OUT_PARAMETER(p)                         \
  if (p == nullptr)                                           \
  {                                                           \
    SilKit_error_string =                                         \
      "Return parameter '" #p "' must not be null.";          \
    return SilKit_ReturnCode_BADPARAMETER;                        \
  }
#define ASSERT_VALID_HANDLER_PARAMETER(handler)               \
  if (handler == nullptr)                                     \
  {                                                           \
    SilKit_error_string = kInvalidFunctionPointer;                \
    return SilKit_ReturnCode_BADPARAMETER;                        \
  }

#define ASSERT_VALID_BOOL_PARAMETER(b)                        \
  if (!(b == SilKit_True || b == SilKit_False))                        \
  {                                                           \
    SilKit_error_string =                                         \
        "The parameter '" #b "' is not a valid SilKit_Bool.";     \
    return SilKit_ReturnCode_BADPARAMETER;                        \
  }

extern thread_local std::string SilKit_error_string;

SILKIT_END_DECLS

#pragma pack(pop)
