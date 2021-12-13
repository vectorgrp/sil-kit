/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include "ib/capi/Types.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

#define CAPI_ENTER \
    try

#define CAPI_LEAVE \
    catch (const std::runtime_error& e) { \
        ib_error_string = e.what(); \
        return ib_ReturnCode_UNSPECIFIEDERROR; \
    } \
    catch (const std::exception&) { \
        return ib_ReturnCode_UNSPECIFIEDERROR; \
    }

#define kInvalidFunctionPointer  "Handler function parameter must not be null."

#define ASSERT_VALID_POINTER_PARAMETER(p)                     \
  if (p == nullptr)                                           \
  {                                                           \
    ib_error_string = "Parameter '" #p "' must not be null."; \
    return ib_ReturnCode_BADPARAMETER;                        \
  }
#define ASSERT_VALID_POINTER_TO_POINTER_PARAMETER(p)          \
  if (p == nullptr)                                           \
  {                                                           \
    ib_error_string = "Parameter '" #p "' must not be null."; \
    return ib_ReturnCode_BADPARAMETER;                        \
  }                                                           \
  if (*p == nullptr)                                          \
  {                                                           \
    ib_error_string =                                         \
      "Parameter '" #p "' must not point to a null value.";   \
    return ib_ReturnCode_BADPARAMETER;                        \
  }
#define ASSERT_VALID_OUT_PARAMETER(p)                         \
  if (p == nullptr)                                           \
  {                                                           \
    ib_error_string =                                         \
      "Return parameter '" #p "' must not be null.";          \
    return ib_ReturnCode_BADPARAMETER;                        \
  }
#define ASSERT_VALID_HANDLER_PARAMETER(handler)               \
  if (handler == nullptr)                                     \
  {                                                           \
    ib_error_string = kInvalidFunctionPointer;                \
    return ib_ReturnCode_BADPARAMETER;                        \
  }

extern thread_local std::string ib_error_string;

IB_END_DECLS

#pragma pack(pop)
