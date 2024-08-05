// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>

#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Core {

template <typename T>
class RingBuffer
{
public:
    // types
    using BufArray = std::pair<T*, size_t>;

public:
    // constructors and destructors
    RingBuffer(size_t capacity);

public:
    // public methods
    size_t Capacity();
    size_t Size();

    bool Read(std::vector<T>& elem, bool advanceRPos = true);

    // write perspective (arrays of free memory in ring buffer)
    auto GetArrayOne() -> BufArray; // first physically contiguous array (starting at index _wPos)
    auto GetArrayTwo() -> BufArray; // second physically contiguous array (starting at index 0)

    void AdvanceWPos(size_t numBytes); // public for access from VAsioPeer

    void Reserve(size_t newCapacity);

private:
    // private member fcns
    bool Empty();
    void AdvanceRPos(size_t numBytes);
    void SizeCheck(); // sanity checks

    // write perspective (size of arrays of free memory in ring buffer)
    size_t GetSizeArrayOne() const;
    size_t GetSizeArrayTwo() const;

private:
    // member variables
    std::vector<T> _buffer;

    size_t _size{0};

    size_t _wPos{0};
    size_t _rPos{0};
};

template <class T>
RingBuffer<T>::RingBuffer(size_t capacity)
{
    _buffer.resize(capacity);
}

template <class T>
void RingBuffer<T>::AdvanceWPos(size_t numBytes)
{
    _wPos = (_wPos + numBytes) % Capacity();
    _size += numBytes;

    // size checks
    SizeCheck();
}

template <class T>
void RingBuffer<T>::AdvanceRPos(size_t numBytes)
{
    _rPos = (_rPos + numBytes) % Capacity();
    _size -= numBytes;

    // size checks
    SizeCheck();
}

template <class T>
bool RingBuffer<T>::Read(std::vector<T>& elem, bool advanceRPos)
{
    // make sure, we only copy as many bytes as are contained in the buffer
    if (elem.size() > _size)
    {
        return false;
    }

    // copy data from first contiguous array
    size_t numBytesArrayOne = std::min((Capacity() - _rPos), elem.size());
    std::memcpy(elem.data(), _buffer.data() + _rPos, numBytesArrayOne);

    // copy data from second contiguous array (if necessary)
    if (numBytesArrayOne < elem.size())
    {
        std::memcpy(elem.data() + numBytesArrayOne, _buffer.data(), elem.size() - numBytesArrayOne);
    }

    if (advanceRPos)
    {
        AdvanceRPos(elem.size());
    }

    return true;
}

template <class T>
size_t RingBuffer<T>::GetSizeArrayOne() const
{
    size_t arrayOneSize = std::min(Capacity() - _wPos, Capacity() - _size);

    // handle edge cases
    if (Empty())
    {
        arrayOneSize = Capacity() - _wPos;
    }

    return arrayOneSize;
}

template <class T>
size_t RingBuffer<T>::GetSizeArrayTwo() const
{
    size_t arrayTwoSize = (Capacity() - _size) - GetSizeArrayOne();

    // handle edge cases
    if (Empty())
    {
        arrayTwoSize = _wPos;
    }

    return arrayTwoSize;
}

// get first contiguous array contained in _buffer (free slots)
template <class T>
auto RingBuffer<T>::GetArrayOne() -> RingBuffer<T>::BufArray
{
    return BufArray{_buffer.data() + _wPos, GetSizeArrayOne()};
}

// get second contiguous array contained in _buffer (free slots)
template <class T>
auto RingBuffer<T>::GetArrayTwo() -> RingBuffer<T>::BufArray
{
    return BufArray{_buffer.data(), GetSizeArrayTwo()};
}

template <class T>
size_t RingBuffer<T>::Capacity()
{
    return _buffer.size();
}

template <class T>
size_t RingBuffer<T>::Size()
{
    return _size;
}

template <class T>
bool RingBuffer<T>::Empty()
{
    return _size == 0;
}

template <class T>
void RingBuffer<T>::SizeCheck()
{
    // size must not exceed buffer capacity
    if (_size > Capacity())
    {
        throw SilKitError{"Buffer size must not exceed capacity!"};
    }
}

template <class T>
void RingBuffer<T>::Reserve(size_t newCapacity)
{
    // no need for increase if capacity is already large enough
    if (newCapacity <= Capacity())
    {
        return;
    }

    // copy all data available to temporary vector (we aim at contiguous memory)
    std::vector<T> newBuffer(_size);
    Read(newBuffer, false);

    _buffer = std::move(newBuffer);
    _buffer.resize(newCapacity);

    _rPos = 0;
    _wPos = _size;
}

} // namespace Core
} // namespace SilKit