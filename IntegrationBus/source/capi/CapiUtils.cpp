// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ib/capi/IntegrationBus.h"
#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

#include <string>
#include <iostream>
#include <algorithm>
#include <map>
#include <mutex>
#include <cstring>
#include "CapiImpl.hpp"

extern "C" {

thread_local std::string ib_error_string = "";

#define ib_ReturnCode_SUCCESS_str "Operation succeeded."
#define ib_ReturnCode_UNSPECIFIEDERROR_str "An unspecified error occured."
#define ib_ReturnCode_NOTSUPPORTED_str "Operation is not supported."
#define ib_ReturnCode_NOTIMPLEMENTED_str "Operation is not implemented."
#define ib_ReturnCode_BADPARAMETER_str "Operation failed due to a bad parameter."
#define ib_ReturnCode_BUFFERTOOSMALL_str "Operation failed because a buffer is too small."
#define ib_ReturnCode_TIMEOUT_str "Operation timed out."
#define ib_ReturnCode_UNSUPPORTEDSERVICE_str "The requested service is not supported."

ib_ReturnCode ib_ReturnCodeToString(const char** outString, ib_ReturnCode returnCode)
{
    if (outString == nullptr)
    {
        return ib_ReturnCode_BADPARAMETER;
    }

    switch (returnCode)
    {
    case ib_ReturnCode_SUCCESS:
        *outString = ib_ReturnCode_SUCCESS_str;
        break;
    case ib_ReturnCode_UNSPECIFIEDERROR:
        *outString = ib_ReturnCode_UNSPECIFIEDERROR_str;
        break;
    case ib_ReturnCode_NOTSUPPORTED:
        *outString = ib_ReturnCode_NOTSUPPORTED_str;
        break;
    case ib_ReturnCode_NOTIMPLEMENTED:
        *outString = ib_ReturnCode_NOTIMPLEMENTED_str;
        break;
    case ib_ReturnCode_BADPARAMETER:
        *outString = ib_ReturnCode_BADPARAMETER_str;
        break;
    case ib_ReturnCode_BUFFERTOOSMALL:
        *outString = ib_ReturnCode_BUFFERTOOSMALL_str;
        break;
    case ib_ReturnCode_TIMEOUT:
        *outString = ib_ReturnCode_TIMEOUT_str;
        break;
    case ib_ReturnCode_UNSUPPORTEDSERVICE:
        *outString = ib_ReturnCode_UNSUPPORTEDSERVICE_str;
        break;
    default:
        return ib_ReturnCode_BADPARAMETER;
    }

    return ib_ReturnCode_SUCCESS;
}


const char* ib_GetLastErrorString() {
    const char* error_string = ib_error_string.c_str();
    return error_string;
}


}//extern "C"
