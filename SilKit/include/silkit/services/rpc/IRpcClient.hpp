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

class IRpcClient
{
public:
    virtual ~IRpcClient() = default;

    /*! \brief Initiate a remote procedure call.
     *
     * \param data A non-owning reference to an opaque block of raw data
     * \param userContext An optional user provided pointer that is
     * reobtained when receiving the call result.
     */
    virtual void Call(Util::Span<const uint8_t> data, void* userContext = nullptr) = 0;

    /*! \brief Overwrite the call return handler of this client
     *
     * The signature of the handler is void(IRpcClient* client, RpcCallResultEvent event).
     *
     * \param handler A std::function with the above signature
     */
    virtual void SetCallResultHandler(RpcCallResultHandler handler) = 0;

    /*! \brief Initiate a remote procedure call with a specified timeout.
     *
     *  In a synchronized execution, simulation time is used for the timeout, 
     *  in an unsynchronized execution, system time is used for the timeout.
     *  If a timeout occurs the CallResultHandler is called with status timeout. 
     *  After the timeout occurred, no further call result events will be triggered for this call. 
     * 
     * \param data A non-owning reference to an opaque block of raw data
     * \param timeout A duration in nanoseconds after which the call runs into a timeout and is canceled 
     * \param userContext An optional user provided pointer that is
     * reobtained when receiving the call result.
     */
    virtual void CallWithTimeout(Util::Span<const uint8_t> data,
                                 std::chrono::nanoseconds timeout, void* userContext = nullptr) = 0;
};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
