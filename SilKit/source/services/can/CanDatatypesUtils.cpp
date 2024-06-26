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

#include "CanDatatypesUtils.hpp"

namespace SilKit {
namespace Services {
namespace Can {

bool operator==(const CanFrame& lhs, const CanFrame& rhs)
{
    return lhs.canId == rhs.canId && lhs.flags == rhs.flags && lhs.dlc == rhs.dlc && lhs.sdt == rhs.sdt
           && lhs.vcid == rhs.vcid && lhs.af == rhs.af && Util::ItemsAreEqual(lhs.dataField, rhs.dataField);
}

bool operator==(const CanFrameEvent& lhs, const CanFrameEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.frame == rhs.frame && lhs.userContext == rhs.userContext;
    ;
}

bool operator==(const CanFrameTransmitEvent& lhs, const CanFrameTransmitEvent& rhs)
{
    return lhs.status == rhs.status && lhs.userContext == rhs.userContext;
}

bool operator==(const CanSetControllerMode& lhs, const CanSetControllerMode& rhs)
{
    return lhs.mode == rhs.mode && lhs.flags.cancelTransmitRequests == rhs.flags.cancelTransmitRequests
           && lhs.flags.resetErrorHandling == rhs.flags.resetErrorHandling;
}

bool operator==(const CanConfigureBaudrate& lhs, const CanConfigureBaudrate& rhs)
{
    return lhs.baudRate == rhs.baudRate && lhs.fdBaudRate == rhs.fdBaudRate;
}

bool operator==(const CanStateChangeEvent& lhs, const CanStateChangeEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.state == rhs.state;
}

bool operator==(const CanErrorStateChangeEvent& lhs, const CanErrorStateChangeEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.errorState == rhs.errorState;
}

bool operator==(const WireCanFrameEvent& lhs, const WireCanFrameEvent& rhs)
{
    return ToCanFrameEvent(lhs) == ToCanFrameEvent(rhs);
}

} // namespace Can
} // namespace Services
} // namespace SilKit
