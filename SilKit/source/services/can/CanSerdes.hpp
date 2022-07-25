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

#pragma once

#include "MessageBuffer.hpp"
#include "WireCanMessages.hpp"

#include "silkit/services/can/CanDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Can {

void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::WireCanFrameEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::CanFrameTransmitEvent& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::CanControllerStatus& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::CanConfigureBaudrate& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer,const Services::Can::CanSetControllerMode& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::WireCanFrameEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanFrameTransmitEvent& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanControllerStatus& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanConfigureBaudrate& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, Services::Can::CanSetControllerMode& out);

} // namespace Can    
} // namespace Services
} // namespace SilKit
