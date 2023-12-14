// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include <atomic>
#include <type_traits>


namespace VSilKit {


template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
class AtomicEnum
{
    std::atomic<std::underlying_type_t<T>> _atomic{};

public:
    AtomicEnum() = default;

    explicit AtomicEnum(T value)
        : _atomic{value}
    {
    }

    auto Get() const -> T
    {
        return static_cast<T>(_atomic.load());
    }

    auto ExchangeIfExpected(T expected, const T desired) -> bool
    {
        auto expectedValue{static_cast<std::underlying_type_t<T>>(expected)};
        const auto desiredValue{static_cast<std::underlying_type_t<T>>(desired)};
        return _atomic.compare_exchange_strong(expectedValue, desiredValue);
    }
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::AtomicEnum;
} // namespace Core
} // namespace SilKit
