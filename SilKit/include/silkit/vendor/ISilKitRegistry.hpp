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

#include <functional>
#include <string>

#include "silkit/services/logging/fwd_decl.hpp"

namespace SilKit {
namespace Vendor {
namespace Vector {

//! \brief Dedicated SIL Kit registry for the Vector SIL Kit middleware.
class ISilKitRegistry
{
public:
    virtual ~ISilKitRegistry() = default;

    //! \brief Register the handler that is called when all participants are connected
    virtual void SetAllConnectedHandler(std::function<void()> handler) = 0;

    //! \brief Register the handler that is called when all participants are disconnected
    virtual void SetAllDisconnectedHandler(std::function<void()> handler) = 0;

    //! \brief Returns the logger that is used by the SIL Kit registry.
    virtual auto GetLogger() -> Services::Logging::ILogger* = 0;

    //! \brief Start to listen on the URI with scheme silkit://. e.g. silkit://localhost:8500
    virtual auto StartListening(const std::string& listenUri) -> std::string = 0;
};


}//end namespace Vector
}//end namespace Vendor
}//end namespace SilKit
