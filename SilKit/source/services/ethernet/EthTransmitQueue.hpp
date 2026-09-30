// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <memory>

namespace SilKit {
namespace Services {
namespace Ethernet {

//! \brief Byte budget for the frames of a controller that the transport has not yet written.
class EthTransmitQueue
{
public:
    explicit EthTransmitQueue(size_t capacity);

    //! \brief Returns nullptr if the queue is full. The bytes are freed when the returned reservation is released.
    auto TryReserve(size_t size) -> std::shared_ptr<const void>;

private:
    struct State;
    struct Reservation;

    size_t _capacity;
    std::shared_ptr<State> _state;
};

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
