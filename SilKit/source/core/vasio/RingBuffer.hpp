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
    // public member fcns
    size_t Capacity();
    size_t Size();

    bool Empty();
    bool Full();

    bool Write(const T& elem);
    bool Read(T& elem);

    bool Write(const std::vector<T>& elem);
    bool Read(std::vector<T>& elem, bool advanceRPos = true);

    void PrintRingBuffer(); // physical memory representation

    // write perspective (arrays of free memory in ring buffer)
    auto GetArrayOne() -> BufArray; // first physically contiguous array (starting at index _wPos)
    auto GetArrayTwo() -> BufArray; // second physically contiguous array (starting at index 0)

    void AdvanceWPos(size_t numBytes); // public for access from VAsioPeer

    void SetCapacity(size_t newCapacity);

private:
    // private member fcns
    void AdvanceRPos(size_t numBytes);
    void SizeCheck(); // sanity checks

    // write perspective (size of arrays of free memory in ring buffer)
    size_t GetSizeArrayOne();
    size_t GetSizeArrayTwo();

private:
    // member variables
    std::vector<T> _buffer;

    size_t _capacity;
    size_t _size;

    size_t _wPos;
    size_t _rPos;
};

template <class T>
RingBuffer<T>::RingBuffer(size_t capacity)
    : _capacity{capacity}
    , _size{0}
    , _wPos{0}
    , _rPos{0}
{
    _buffer.resize(capacity);
}

template <class T>
void RingBuffer<T>::AdvanceWPos(size_t numBytes)
{
    if (numBytes <= 0)
    {
        throw SilKitError{"Only positive number of bytes allowed."};
    }

    _wPos = (_wPos + numBytes) % _capacity;
    _size += numBytes;

    // size checks
    SizeCheck();
}

template <class T>
void RingBuffer<T>::AdvanceRPos(size_t numBytes)
{
    if (numBytes <= 0)
    {
        throw SilKitError{"Only positive number of bytes allowed."};
    }

    _rPos = (_rPos + numBytes) % _capacity;
    _size -= numBytes;

    // size checks
    SizeCheck();
}

template <class T>
bool RingBuffer<T>::Write(const T& elem)
{
    if (Full()) // we don't allow overwriting
    {
        return false;
    }

    _buffer.at(_wPos) = elem;
    AdvanceWPos(1);

    return true;
}

template <class T>
bool RingBuffer<T>::Read(T& elem)
{
    if (Empty()) // no data to read
    {
        return false;
    }

    elem = _buffer.at(_rPos);
    AdvanceRPos(1);

    return true;
}

template <class T>
bool RingBuffer<T>::Write(const std::vector<T>& elem)
{
    // we don't allow overwriting
    if (elem.size() > (_capacity - _size))
    {
        return false;
    }

    // copy data to first contiguous array
    size_t numBytesArrayOne = std::min((_capacity - _wPos), elem.size());
    std::memcpy(_buffer.data() + _wPos, elem.data(), numBytesArrayOne);

    // copy data to second contiguous array (if necessary)
    if (numBytesArrayOne < elem.size())
    {
        std::memcpy(_buffer.data(), elem.data() + numBytesArrayOne, elem.size() - numBytesArrayOne);
    }

    AdvanceWPos(elem.size());

    return true;
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
    size_t numBytesArrayOne = std::min((_capacity - _rPos), elem.size());
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
size_t RingBuffer<T>::GetSizeArrayOne()
{
    size_t arrayOneSize = std::min(_capacity - _wPos, _capacity - _size);

    // handle edge cases
    if (Empty())
    {
        arrayOneSize = _capacity - _wPos;
    }

    return arrayOneSize;
}

template <class T>
size_t RingBuffer<T>::GetSizeArrayTwo()
{
    size_t arrayTwoSize = (_capacity - _size) - GetSizeArrayOne();

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
    return _capacity;
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
bool RingBuffer<T>::Full()
{
    return _size == _capacity;
}

template <class T>
void RingBuffer<T>::SizeCheck()
{
    // size must be non-negative
    if (_size < 0)
    {
        throw SilKitError{"Buffer size must be non-negative!"};
    }

    // size must not exceed buffer capacity
    if (_size > _capacity)
    {
        throw SilKitError{"Buffer size must not exceed capacity!"};
    }
}

template <class T>
void RingBuffer<T>::SetCapacity(size_t newCapacity)
{
    // no need for increase if capacity is already large enough
    if (newCapacity <= _capacity)
    {
        return;
    }

    // copy all data available to temporary vector (we aim at contiguous memory)
    std::vector<T> newBuffer(_size);
    Read(newBuffer, false);

    _buffer = std::move(newBuffer);
    _buffer.resize(newCapacity);

    _capacity = newCapacity;
    _rPos = 0;
    _wPos = _size;
}

template <class T>
void RingBuffer<T>::PrintRingBuffer()
{
    std::vector<std::string> bufferValues(_capacity, "-");

    if ((_rPos + _size) <= _capacity) // written data is contiguous
    {
        for (size_t i = _rPos; i < (_rPos + _size); i++)
        {
            bufferValues.at(i) = std::to_string(_buffer.at(i));
        }
    }
    else // non-contiguous case
    {
        // array one
        size_t sizeArrayTwo{_size};
        for (size_t i = _rPos; i < _capacity; i++)
        {
            bufferValues.at(i) = std::to_string(_buffer.at(i));
            sizeArrayTwo--;
        }

        // array two
        for (size_t i = 0; i < sizeArrayTwo; i++)
        {
            bufferValues.at(i) = std::to_string(_buffer.at(i));
        }
    }

    // print index, value (if present), memory address
    std::cout << "----- buffer start (current size: " << Size() << ", current capacity: " << Capacity() << ") -----"
              << std::endl;
    for (size_t i = 0; i < bufferValues.capacity(); i++)
    {
        std::cout << "Index: " << i << ", Value: " << bufferValues.at(i)
                  << ", Memory Address: " << static_cast<void*>(_buffer.data() + i) << std::endl;
    }
    std::cout << "----- buffer end -----" << std::endl;
}

} // namespace Core
} // namespace SilKit