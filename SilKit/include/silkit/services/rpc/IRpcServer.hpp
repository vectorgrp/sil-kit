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

#include "RpcDatatypes.hpp"

#include "silkit/util/Span.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class IRpcServer
{
public:
    virtual ~IRpcServer() = default;

    /*! \brief Answers an already received call from remote with arbitrary data
    *
    * Using the call handle obtained in the call handler, the result is send back to the calling client.
    * This can happen directly in the call handler or at a later point in time.
    *
    * \param callHandle A unique identifier of this call
    * \param resultData The byte vector to be returned to the client
    */
    virtual void SubmitResult(IRpcCallHandle* callHandle, Util::Span<const uint8_t> resultData) = 0;

    /*! \brief Overwrite the call handler of this server
     *
     * The signature of the call handler is void(IRpcServer* server, RpcCallEvent event).
     *
     * \param handler A std::function with the above signature
     */
    virtual void SetCallHandler(RpcCallHandler handler) = 0;
};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
