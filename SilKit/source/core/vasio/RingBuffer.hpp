// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <stdint.h>

#include "util/Buffer.hpp"

namespace SilKit {
namespace Core {

class RingBuffer
{
public:
    // constructors and destructors
    RingBuffer(std::size_t capacity);

public:
    // public methods
    std::size_t Capacity() const;
    std::size_t Size() const;

    bool Peek(std::vector<uint8_t>& elem) const;
    bool Read(std::vector<uint8_t>& elem);

    void GetWritingBuffers(std::vector<MutableBuffer>& buffers);

    void AdvanceWPos(std::size_t numBytes); // public for access from VAsioPeer

    void Reserve(std::size_t newCapacity);

private:
    // private methods
    bool Empty() const;
    void AdvanceRPos(std::size_t numBytes);
    void SizeCheck() const;

    // write perspective (size of arrays of free memory in ring buffer)
    std::size_t GetFreeMemorySizeArrayOne() const;
    std::size_t GetFreeMemorySizeArrayTwo() const;

    // write perspective (arrays of free memory in ring buffer)
    MutableBuffer GetFreeMemoryArrayOne(); // first physically contiguous array (starting at index _wPos)
    MutableBuffer GetFreeMemoryArrayTwo(); // second physically contiguous array (starting at index 0)

private:
    // member variables
    std::vector<uint8_t> _buffer;

    std::size_t _size{0};

    std::size_t _wPos{0};
    std::size_t _rPos{0};
};

} // namespace Core
} // namespace SilKit