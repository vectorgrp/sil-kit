// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/util/Span.hpp"

#include <chrono>
#include <memory>
#include <algorithm>

namespace SilKit {
namespace Util {

template <typename T>
class SharedVector
{
    static_assert(!std::is_const<T>::value, "T must not be const");
    static_assert(!std::is_reference<T>::value, "T must not be a reference");

public:
    SharedVector() = default;

    SharedVector(std::initializer_list<T> initializerList);

    SharedVector(const std::vector<T> vector);

    SharedVector(const Span<const T> span);

    auto AsSpan() const& -> Span<const T>;

private:
    std::shared_ptr<std::vector<T>> _data;
};

template <typename T>
bool ItemsAreEqual(const SharedVector<T>& lhs, const SharedVector<T>& rhs);

// ================================================================================
//  Inline Implementations
// ================================================================================

template <typename T>
SharedVector<T>::SharedVector(std::initializer_list<T> initializerList)
    : SharedVector(std::vector<T>{initializerList})
{
}

template <typename T>
SharedVector<T>::SharedVector(const std::vector<T> vector)
    : _data{std::make_shared<std::vector<T>>(std::move(vector))}
{
}

template <typename T>
SharedVector<T>::SharedVector(const Span<const T> span)
    : _data{std::make_shared<std::vector<T>>(span.begin(), span.end())}
{
}

template <typename T>
auto SharedVector<T>::AsSpan() const& -> Span<const T>
{
    if (_data)
    {
        return {_data->data(), _data->size()};
    }
    else
    {
        return {nullptr, 0};
    }
}

template <typename T>
bool ItemsAreEqual(const SharedVector<T>& lhs, const SharedVector<T>& rhs)
{
    return ItemsAreEqual(lhs.AsSpan(), rhs.AsSpan());
}

} // namespace Util
} // namespace SilKit
