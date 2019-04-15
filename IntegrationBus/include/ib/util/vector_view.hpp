// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cassert>
#include <vector>

namespace ib {
namespace util {

template<typename T>
class vector_view
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors, Destructor, and assignment operator
    vector_view() = delete;
    vector_view(const vector_view&) = default;
    vector_view(vector_view&&) = default;
    vector_view(std::vector<T>& vector);
    vector_view(const std::vector<std::remove_cv_t<T>>& vector);
    
    auto operator=(vector_view& other) -> vector_view& = default;
    auto operator=(vector_view&& other) -> vector_view& = default;

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
    auto begin() -> T*;
    auto begin() const -> const T*;
    auto cbegin() const -> const T*;
    auto end() -> T*;
    auto end() const -> const T*;
    auto cend() const -> const T*;

    // Capacity
    auto empty() const -> bool;
    auto size() const -> size_t;

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
    T* _data;
    size_t _size;
};

template <typename T>
auto make_vector_view(std::vector<T>& vector) -> vector_view<T>;
template <typename T>
auto make_vector_view(const std::vector<T>& vector) -> vector_view<const T>;
    
// ================================================================================
//  Inline Implementations
// ================================================================================

template <typename T>
vector_view<T>::vector_view(std::vector<T>& vector) :
    _data{vector.data()},
    _size{vector.size()}
{
}
    
template <typename T>
vector_view<T>::vector_view(const std::vector<std::remove_cv_t<T>>& vector) :
    _data{vector.data()},
    _size{vector.size()}
{
}


template <typename T>
auto vector_view<T>::at(size_t pos) -> T&
{
    if (pos >= _size)
        throw std::out_of_range("invalid vector_view<T> subscript");

    return _data[pos];
}

template <typename T>
auto vector_view<T>::at(size_t pos) const -> const T&
{
    if (pos >= _size)
        throw std::out_of_range("invalid vector_view<T> subscript");

    return _data[pos];
}

template <typename T>
auto vector_view<T>::operator[](size_t pos) -> T&
{
    return _data[pos];
}

template <typename T>
auto vector_view<T>::operator[](size_t pos) const -> const T&
{
    return _data[pos];
}

template <typename T>
auto vector_view<T>::front() -> T&
{
    return _data[0];
}

template <typename T>
auto vector_view<T>::front() const -> const T&
{
    return _data[0];
}

template <typename T>
auto vector_view<T>::back() -> T&
{
    return _data[_size - 1];
}

template <typename T>
auto vector_view<T>::back() const -> const T&
{
    return _data[_size - 1];
}

template <typename T>
auto vector_view<T>::data() -> T*
{
    return _data;
}

template <typename T>
auto vector_view<T>::data() const -> const T*
{
    return _data;
}


template <typename T>
auto vector_view<T>::begin() -> T*
{
    return _data;
}

template <typename T>
auto vector_view<T>::begin() const -> const T*
{
    return _data;
}

template <typename T>
auto vector_view<T>::cbegin() const -> const T*
{
    return _data;
}

template <typename T>
auto vector_view<T>::end() -> T*
{
    return _data + _size;
}

template <typename T>
auto vector_view<T>::end() const -> const T*
{
    return _data + _size;
}

template <typename T>
auto vector_view<T>::cend() const -> const T*
{
    return _data + _size;
}


template <typename T>
auto vector_view<T>::empty() const -> bool
{
    return _size == 0;
}

template <typename T>
auto vector_view<T>::size() const -> size_t
{
    return _size;
}


template <typename T>
void vector_view<T>::trim_front(size_t len)
{
    assert(len <= _size);
    _data += len;
    _size -= len;
}

template <typename T>
void vector_view<T>::trim_back(size_t len)
{
    assert(len <= _size);
    _size -= len;
}


template <typename T>
auto make_vector_view(std::vector<T>& vector) -> vector_view<T>
{
    return vector_view<T>(vector);
}

template <typename T>
auto make_vector_view(const std::vector<T>& vector) -> vector_view<const T>
{
    return vector_view<const T>(vector);
}

} // namespace util
} // namespace ib
