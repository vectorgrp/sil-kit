// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/capi/SilKit.h"
#include "silkit/services/orchestration/all.hpp"

#include <string>
#include <algorithm>

thread_local std::string SilKit_error_string = "";

#define SilKit_ReturnCode_SUCCESS_str "Operation succeeded"
#define SilKit_ReturnCode_UNSPECIFIEDERROR_str "An unspecified error occurred"
#define SilKit_ReturnCode_NOTSUPPORTED_str "Operation is not supported"
#define SilKit_ReturnCode_NOTIMPLEMENTED_str "Operation is not implemented"
#define SilKit_ReturnCode_BADPARAMETER_str "Operation failed due to a bad parameter"
#define SilKit_ReturnCode_BUFFERTOOSMALL_str "Operation failed because a buffer is too small"
#define SilKit_ReturnCode_TIMEOUT_str "Operation timed out"
#define SilKit_ReturnCode_UNSUPPORTEDSERVICE_str "The requested service is not supported"
#define SilKit_ReturnCode_WRONGSTATE_str "State error"
#define SilKit_ReturnCode_TYPECONVERSIONERROR_str "Invalid type conversion"
#define SilKit_ReturnCode_CONFIGURATIONERROR_str "Configuration has errors"
#define SilKit_ReturnCode_PROTOCOLERROR_str "Protocol error"
#define SilKit_ReturnCode_ASSERTIONERROR_str "Assertion error"
#define SilKit_ReturnCode_EXTENSIONERROR_str "Extension could not be loaded"
#define SilKit_ReturnCode_LOGICERROR_str "Violation of logical preconditions"
#define SilKit_ReturnCode_LENGTHERROR_str "Length limits violated"
#define SilKit_ReturnCode_OUTOFRANGEERROR_str "Access of elements outside the defined range"

SilKit_ReturnCode SilKitCALL SilKit_ReturnCodeToString(const char** outString, SilKit_ReturnCode returnCode)
{
    if (outString == nullptr)
    {
        return SilKit_ReturnCode_BADPARAMETER;
    }

    switch (returnCode)
    {
    case SilKit_ReturnCode_SUCCESS:
        *outString = SilKit_ReturnCode_SUCCESS_str;
        break;
    case SilKit_ReturnCode_UNSPECIFIEDERROR:
        *outString = SilKit_ReturnCode_UNSPECIFIEDERROR_str;
        break;
    case SilKit_ReturnCode_NOTSUPPORTED:
        *outString = SilKit_ReturnCode_NOTSUPPORTED_str;
        break;
    case SilKit_ReturnCode_NOTIMPLEMENTED:
        *outString = SilKit_ReturnCode_NOTIMPLEMENTED_str;
        break;
    case SilKit_ReturnCode_BADPARAMETER:
        *outString = SilKit_ReturnCode_BADPARAMETER_str;
        break;
    case SilKit_ReturnCode_BUFFERTOOSMALL:
        *outString = SilKit_ReturnCode_BUFFERTOOSMALL_str;
        break;
    case SilKit_ReturnCode_TIMEOUT:
        *outString = SilKit_ReturnCode_TIMEOUT_str;
        break;
    case SilKit_ReturnCode_UNSUPPORTEDSERVICE:
        *outString = SilKit_ReturnCode_UNSUPPORTEDSERVICE_str;
        break;
    case SilKit_ReturnCode_WRONGSTATE:
        *outString = SilKit_ReturnCode_WRONGSTATE_str;
        break;
    case SilKit_ReturnCode_TYPECONVERSIONERROR:
        *outString = SilKit_ReturnCode_TYPECONVERSIONERROR_str;
        break;
    case SilKit_ReturnCode_CONFIGURATIONERROR:
        *outString = SilKit_ReturnCode_CONFIGURATIONERROR_str;
        break;
    case SilKit_ReturnCode_PROTOCOLERROR:
        *outString = SilKit_ReturnCode_PROTOCOLERROR_str;
        break;
    case SilKit_ReturnCode_ASSERTIONERROR:
        *outString = SilKit_ReturnCode_ASSERTIONERROR_str;
        break;
    case SilKit_ReturnCode_EXTENSIONERROR:
        *outString = SilKit_ReturnCode_EXTENSIONERROR_str;
        break;
    case SilKit_ReturnCode_LOGICERROR:
        *outString = SilKit_ReturnCode_LOGICERROR_str;
        break;
    case SilKit_ReturnCode_LENGTHERROR:
        *outString = SilKit_ReturnCode_LENGTHERROR_str;
        break;
    case SilKit_ReturnCode_OUTOFRANGEERROR:
        *outString = SilKit_ReturnCode_OUTOFRANGEERROR_str;
        break;
    default:
        return SilKit_ReturnCode_BADPARAMETER;
    }

    return SilKit_ReturnCode_SUCCESS;
}

const char* SilKitCALL SilKit_GetLastErrorString()
{
    const char* error_string = SilKit_error_string.c_str();
    return error_string;
}
