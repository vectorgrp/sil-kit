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

#include "Serializer.hpp"
#include "Deserializer.hpp"

namespace SilKit {
namespace Util {
namespace SerDes {

inline namespace v1 {

/*! \brief The data media / mime type the serializer / deserializer can be used for.
 *  \returns The data media / mime type the serializer / deserializer can be used for. */
constexpr auto MediaTypeData() -> const char*
{
    return "application/vnd.vector.silkit.data; protocolVersion=1";
}

/*! \brief The RPC media / mime type the serializer / deserializer can be used for.
 *  \returns The RPC media / mime type the serializer / deserializer can be used for. */
constexpr auto MediaTypeRpc() -> const char*
{
    return "application/vnd.vector.silkit.rpc; protocolVersion=1";
}

} // namespace v1

} // namespace SerDes
} // namespace Util
} // namespace SilKit
