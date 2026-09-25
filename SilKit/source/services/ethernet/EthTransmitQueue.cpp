// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "services/ethernet/EthTransmitQueue.hpp"

#include <atomic>

namespace SilKit {
namespace Services {
namespace Ethernet {

struct EthTransmitQueue::State
{
    std::atomic<size_t> used{0};
};

struct EthTransmitQueue::Reservation
{
    Reservation(std::shared_ptr<State> state, size_t size)
        : _state{std::move(state)}
        , _size{size}
    {
    }

    Reservation(const Reservation&) = delete;
    Reservation& operator=(const Reservation&) = delete;

    ~Reservation()
    {
        _state->used.fetch_sub(_size, std::memory_order_acq_rel);
    }

private:
    std::shared_ptr<State> _state;
    size_t _size;
};

EthTransmitQueue::EthTransmitQueue(size_t capacity)
    : _capacity{capacity}
    , _state{std::make_shared<State>()}
{
}

auto EthTransmitQueue::TryReserve(size_t size) -> std::shared_ptr<const void>
{
    auto used = _state->used.load(std::memory_order_acquire);
    do
    {
        // NB: an empty queue always accepts, so frames larger than the capacity still pass
        if (used > 0 && used + size > _capacity)
        {
            return nullptr;
        }
    } while (!_state->used.compare_exchange_weak(used, used + size, std::memory_order_acq_rel));

    return std::make_shared<Reservation>(_state, size);
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
