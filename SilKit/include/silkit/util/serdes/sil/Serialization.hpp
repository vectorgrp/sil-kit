// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Serializer.hpp"
#include "Deserializer.hpp"

namespace SilKit {
namespace Util {
namespace SerDes {

inline namespace v1 {

/*! \brief The data media / mime type the serializer / deserializer can be used for.
 *  \returns the data media / mime type the serializer / deserializer can be used for. */
constexpr auto MediaTypeData() -> const char*
{
    return "application/vnd.vector.sil.data; protocolVersion=1";
}

/*! \brief The RPC media / mime type the serializer / deserializer can be used for.
 *  \returns the RPC media / mime type the serializer / deserializer can be used for. */
constexpr auto MediaTypeRpc() -> const char*
{
    return "application/vnd.vector.sil.rpc; protocolVersion=1";
}

} // namespace v1

} // namespace SerDes
} // namespace Util
} // namespace SilKit
