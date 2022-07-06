// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "silkit/capi/SilKit.h"
#include "silkit/SilKit.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>
#include "CapiImpl.hpp"

extern "C" {

thread_local std::string SilKit_error_string = "";

#define SilKit_ReturnCode_SUCCESS_str "Operation succeeded."
#define SilKit_ReturnCode_UNSPECIFIEDERROR_str "An unspecified error occured."
#define SilKit_ReturnCode_NOTSUPPORTED_str "Operation is not supported."
#define SilKit_ReturnCode_NOTIMPLEMENTED_str "Operation is not implemented."
#define SilKit_ReturnCode_BADPARAMETER_str "Operation failed due to a bad parameter."
#define SilKit_ReturnCode_BUFFERTOOSMALL_str "Operation failed because a buffer is too small."
#define SilKit_ReturnCode_TIMEOUT_str "Operation timed out."
#define SilKit_ReturnCode_UNSUPPORTEDSERVICE_str "The requested service is not supported."

SilKit_ReturnCode SilKit_ReturnCodeToString(const char** outString, SilKit_ReturnCode returnCode)
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
    default:
        return SilKit_ReturnCode_BADPARAMETER;
    }

    return SilKit_ReturnCode_SUCCESS;
}


const char* SilKit_GetLastErrorString() {
    const char* error_string = SilKit_error_string.c_str();
    return error_string;
}


}//extern "C"
