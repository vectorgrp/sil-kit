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

#include "silkit/capi/SilKit.h"
#include "silkit/services/orchestration/all.hpp"

#include <string>
#include <algorithm>

thread_local std::string SilKit_error_string = "";

#define SilKit_ReturnCode_SUCCESS_str "Operation succeeded."
#define SilKit_ReturnCode_UNSPECIFIEDERROR_str "An unspecified error occured."
#define SilKit_ReturnCode_NOTSUPPORTED_str "Operation is not supported."
#define SilKit_ReturnCode_NOTIMPLEMENTED_str "Operation is not implemented."
#define SilKit_ReturnCode_BADPARAMETER_str "Operation failed due to a bad parameter."
#define SilKit_ReturnCode_BUFFERTOOSMALL_str "Operation failed because a buffer is too small."
#define SilKit_ReturnCode_TIMEOUT_str "Operation timed out."
#define SilKit_ReturnCode_UNSUPPORTEDSERVICE_str "The requested service is not supported."


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
