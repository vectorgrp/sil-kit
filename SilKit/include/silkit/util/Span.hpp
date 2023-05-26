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

#include <algorithm>
#include <vector>
#include <array>
#include <string>
#include <stdexcept>
#include <type_traits>

#include "silkit/capi/Types.h"
#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Util {

template <typename T>
class Span
{
    static_assert(!std::is_reference<T>::value, "T must not be reference");
    static_assert(!std::is_volatile<T>::value, "T must not be volatile");

    template <typename U>
    using enable_if_non_const_T_t =
        std::enable_if_t<!std::is_const<U>::value && std::is_same<U, std::remove_const_t<T>>::value, bool>;

    template <typename U>
    using enable_if_non_const_T_while_T_const_t = std::enable_if_t<
        !std::is_const<U>::value && std::is_const<T>::value && std::is_same<U, std::remove_const_t<T>>::value, bool>;

public:
    // ----------------------------------------
    // Public Data Types
    using iterator = T*;
    using const_iterator = const T*;

public:
    // ----------------------------------------
    // Constructors, Destructor, and assignment operator
    Span() = default;
    Span(const Span&) = default;
    Span(Span&&) noexcept = default;

    Span(T* data, std::size_t size);

    template <typename U, enable_if_non_const_T_while_T_const_t<U> = true>
    Span(Span<U> other)
        : Span(other.data(), other.size())
    {
    }

    template <typename U, enable_if_non_const_T_while_T_const_t<U> = true>
    Span(const std::vector<U>& vector)
        : Span(vector.data(), vector.size())
    {
    }

    template <typename U, enable_if_non_const_T_t<U> = true>
    Span(std::vector<U>& vector)
        : Span(vector.data(), vector.size())
    {
    }

    auto operator=(const Span& other) -> Span& = default;
    auto operator=(Span&& other) noexcept -> Span& = default;

    template <typename U, enable_if_non_const_T_while_T_const_t<U> = true>
    auto operator=(Span<U> other) -> Span&
    {
        _data = other.data();
        _size = other.size();
        return *this;
    }

    template <typename U, enable_if_non_const_T_while_T_const_t<U> = true>
    auto operator=(const std::vector<U>& vector) -> Span&
    {
        _data = vector.data();
        _size = vector.size();
        return *this;
    }

    template <typename U, enable_if_non_const_T_t<U> = true>
    auto operator=(std::vector<U>& vector) -> Span&
    {
        _data = vector.data();
        _size = vector.size();
        return *this;
    }

public:
    // ----------------------------------------
    // container methods

    // Element access
    // If pos is not within the range of the container, an exception of type std::out_of_range is thrown.
    auto at(size_t pos) -> T&;
    auto at(size_t pos) const -> const T&;
    auto operator[](size_t pos) -> T&;
    auto operator[](size_t pos) const -> const T&;

    auto front() -> T&;
    auto front() const -> const T&;
    auto back() -> T&;
    auto back() const -> const T&;

    auto data() -> T*;
    auto data() const -> const T*;

    // Iterators
    auto begin() -> iterator;
    auto begin() const -> const_iterator;
    auto cbegin() const -> const_iterator;
    auto end() -> iterator;
    auto end() const -> const_iterator;
    auto cend() const -> const_iterator;

    // Capacity
    auto empty() const -> bool;
    auto size() const -> size_t;

    // Operations
    void trim_front(size_t len);
    void trim_back(size_t len);

private:
    // ----------------------------------------
    // private data types

private:
    // ----------------------------------------
    // private methods

private:
    // ----------------------------------------
    // private members
    T* _data = nullptr;
    size_t _size = 0;
};

// Non-member functions

template <typename T>
auto ToSpan(std::vector<T>& vector) -> Span<T>;

template <typename T>
auto ToSpan(const std::vector<T>& vector) -> Span<const T>;

inline auto ToSpan(const SilKit_ByteVector& skByteVector) -> Span<const uint8_t>;

inline auto ToSilKitByteVector(Span<const uint8_t> span) -> SilKit_ByteVector;

template <typename T>
auto ToStdVector(Span<T> span) -> std::vector<std::remove_cv_t<T>>;

// ================================================================================
//  Inline Implementations
// ================================================================================

template <typename T>
Span<T>::Span(T* data, std::size_t size)
    : _data{data}
    , _size{size}
{
}

// Element access

template <typename T>
auto Span<T>::at(size_t pos) -> T&
{
    if (pos >= _size)
        throw OutOfRangeError("invalid Span<T> subscript");

    return _data[pos];
}

template <typename T>
auto Span<T>::at(size_t pos) const -> const T&
{
    if (pos >= _size)
        throw OutOfRangeError("invalid Span<T> subscript");

    return _data[pos];
}

template <typename T>
auto Span<T>::operator[](size_t pos) -> T&
{
    return _data[pos];
}

template <typename T>
auto Span<T>::operator[](size_t pos) const -> const T&
{
    return _data[pos];
}

template <typename T>
auto Span<T>::front() -> T&
{
    return _data[0];
}

template <typename T>
auto Span<T>::front() const -> const T&
{
    return _data[0];
}

template <typename T>
auto Span<T>::back() -> T&
{
    return _data[_size - 1];
}

template <typename T>
auto Span<T>::back() const -> const T&
{
    return _data[_size - 1];
}

template <typename T>
auto Span<T>::data() -> T*
{
    return _data;
}

template <typename T>
auto Span<T>::data() const -> const T*
{
    return _data;
}

// Iterators

template <typename T>
auto Span<T>::begin() -> T*
{
    return _data;
}

template <typename T>
auto Span<T>::begin() const -> const T*
{
    return _data;
}

template <typename T>
auto Span<T>::cbegin() const -> const T*
{
    return _data;
}

template <typename T>
auto Span<T>::end() -> T*
{
    return _data + _size;
}

template <typename T>
auto Span<T>::end() const -> const T*
{
    return _data + _size;
}

template <typename T>
auto Span<T>::cend() const -> const T*
{
    return _data + _size;
}

// Capacity

template <typename T>
auto Span<T>::empty() const -> bool
{
    return _size == 0;
}

template <typename T>
auto Span<T>::size() const -> size_t
{
    return _size;
}

// Operations

template <typename T>
void Span<T>::trim_front(size_t len)
{
    if(!(len <= _size))
    {
        throw AssertionError("Span::trim_front assertion 'len <= _size' failed");
    }
    _data += len;
    _size -= len;
}

template <typename T>
void Span<T>::trim_back(size_t len)
{
    if(!(len <= _size))
    {
        throw AssertionError("Span::trim_back assertion 'len <= _size' failed");
    }
    _size -= len;
}

// Non-member functions

template <typename T, std::size_t N>
auto MakeSpan(std::array<T, N>& array) -> Span<T>
{
    return {array.data(), N};
}

template <typename T, std::size_t N>
auto MakeSpan(const std::array<T, N>& array) -> Span<const T>
{
    return {array.data(), N};
}

template <typename T>
auto ToSpan(std::vector<T>& vector) -> Span<T>
{
    return {vector.data(), vector.size()};
}

template <typename T>
auto ToSpan(const std::vector<T>& vector) -> Span<const T>
{
    return {vector.data(), vector.size()};
}

auto ToSpan(const SilKit_ByteVector& skByteVector) -> Span<const uint8_t>
{
    return {skByteVector.data, skByteVector.size};
}

auto ToSilKitByteVector(Span<const uint8_t> span) -> SilKit_ByteVector
{
    return SilKit_ByteVector{span.empty() ? nullptr : span.data(), span.size()};
}

template <typename T>
auto ToStdVector(Span<T> span) -> std::vector<std::remove_cv_t<T>>
{
    return {span.begin(), span.end()};
}

template <typename T, typename U>
bool ItemsAreEqual(const Span<T>& lhs, const Span<U>& rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

} // namespace Util
} // namespace SilKit
