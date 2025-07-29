// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

#include "silkit/util/Span.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class IDataPublisher
{
public:
    virtual ~IDataPublisher() = default;

    /*! \brief Publish a new value
     *
     * Convenience method to publish data. Creates a new std::vector
     * with content copied from \p data. For highest efficiency,
     * use \ref Publish(Util::Span<const uint8_t>) in combination with
     * std::move.
     *
     * \param data A non-owning reference to an opaque block of raw data
     */
    virtual void Publish(Util::Span<const uint8_t> data) = 0;
};

} // namespace PubSub
} // namespace Services
} // namespace SilKit
