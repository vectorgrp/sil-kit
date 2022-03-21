// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "Serializer.hpp"
#include "Deserializer.hpp"

namespace ib {
namespace util {
namespace serdes {
namespace sil {

inline namespace v4 {

/*! \brief The media / mime type the serializer / deserializer can be used for.
 *  \returns the media / mime type the serializer / deserializer can be used for. */
constexpr auto MediaType() -> const char*
{
    return "application/vnd.vector.sil.data; protocolVersion=1";
}

} // namespace v4

} // namespace sil
} // namespace serdes
} // namespace util
} // namespace ib
