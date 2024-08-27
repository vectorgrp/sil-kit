// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "RingBuffer.hpp"

#include <algorithm>
#include <iterator>

#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Core {

RingBuffer::RingBuffer(size_t capacity)
{
    _buffer.resize(capacity);
}

void RingBuffer::AdvanceWPos(size_t numBytes)
{
    _wPos = (_wPos + numBytes) % Capacity();
    _size += numBytes;

    SizeCheck();
}

void RingBuffer::AdvanceRPos(size_t numBytes)
{
    _rPos = (_rPos + numBytes) % Capacity();
    _size -= numBytes;

    SizeCheck();
}

bool RingBuffer::Peek(std::vector<uint8_t>& elem) const
{
    // make sure, we only copy as many bytes as are contained in the buffer
    if (elem.size() > _size)
    {
        return false;
    }

    // copy data from first contiguous array (of occupied memory)
    size_t numBytesArrayOne = std::min((Capacity() - _rPos), elem.size());
    std::copy(std::next(_buffer.begin(), _rPos), std::next(_buffer.begin(), _rPos + numBytesArrayOne), elem.begin());

    // copy data from second contiguous array (of occupied memory)
    if (numBytesArrayOne < elem.size())
    {
        std::copy(_buffer.begin(), std::next(_buffer.begin(), elem.size() - numBytesArrayOne),
                  std::next(elem.begin(), numBytesArrayOne));
    }

    return true;
}

bool RingBuffer::Read(std::vector<uint8_t>& elem)
{
    if (!Peek(elem))
    {
        return false;
    }

    AdvanceRPos(elem.size());

    return true;
}

void RingBuffer::GetWritingBuffers(std::vector<MutableBuffer>& buffers)
{
    auto arrayOne = GetFreeMemoryArrayOne();
    if (arrayOne.GetSize() > 0)
    {
        buffers.push_back(std::move(arrayOne));
    }

    auto arrayTwo = GetFreeMemoryArrayTwo();
    if (arrayTwo.GetSize() > 0)
    {
        buffers.push_back(std::move(arrayTwo));
    }
}

size_t RingBuffer::GetFreeMemorySizeArrayOne() const
{
    size_t arrayOneSize = std::min(Capacity() - _wPos, Capacity() - _size);

    // handle edge cases
    if (Empty())
    {
        arrayOneSize = Capacity() - _wPos;
    }

    return arrayOneSize;
}

size_t RingBuffer::GetFreeMemorySizeArrayTwo() const
{
    size_t arrayTwoSize = (Capacity() - _size) - GetFreeMemorySizeArrayOne();

    // handle edge cases
    if (Empty())
    {
        arrayTwoSize = _wPos;
    }

    return arrayTwoSize;
}

// get first contiguous array contained in _buffer (free slots)
MutableBuffer RingBuffer::GetFreeMemoryArrayOne()
{
    return MutableBuffer{_buffer.data() + _wPos, GetFreeMemorySizeArrayOne()};
}

// get second contiguous array contained in _buffer (free slots)
MutableBuffer RingBuffer::GetFreeMemoryArrayTwo()
{
    return MutableBuffer{_buffer.data(), GetFreeMemorySizeArrayTwo()};
}

size_t RingBuffer::Capacity() const
{
    return _buffer.size();
}

size_t RingBuffer::Size() const
{
    return _size;
}

bool RingBuffer::Empty() const
{
    return _size == 0;
}

void RingBuffer::SizeCheck() const
{
    if (_size > Capacity())
    {
        throw SilKitError{"Buffer size must not exceed capacity!"};
    }
}

void RingBuffer::Reserve(size_t newCapacity)
{
    // no need for increase if capacity is already large enough
    if (newCapacity <= Capacity())
    {
        return;
    }

    // copy all data available to temporary vector (we aim at contiguous memory)
    std::vector<uint8_t> newBuffer(_size);
    Peek(newBuffer);

    _buffer = std::move(newBuffer);
    _buffer.resize(newCapacity);

    _rPos = 0;
    _wPos = _size;
}

} // namespace Core
} // namespace SilKit