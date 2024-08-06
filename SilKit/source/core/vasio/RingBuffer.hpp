// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <stdint.h>

namespace SilKit {
namespace Core {

class RingBuffer
{
public:
    // types
    using BufArray = std::pair<uint8_t*, std::size_t>;

public:
    // constructors and destructors
    RingBuffer(std::size_t capacity);

public:
    // public methods
    std::size_t Capacity() const;
    std::size_t Size() const;

    bool Peek(std::vector<uint8_t>& elem);
    bool Read(std::vector<uint8_t>& elem);

    // write perspective (arrays of free memory in ring buffer)
    BufArray GetFreeMemoryArrayOne(); // first physically contiguous array (starting at index _wPos)
    BufArray GetFreeMemoryArrayTwo(); // second physically contiguous array (starting at index 0)

    void AdvanceWPos(std::size_t numBytes); // public for access from VAsioPeer

    void Reserve(std::size_t newCapacity);

private:
    // private methods
    bool Empty() const;
    void AdvanceRPos(std::size_t numBytes);
    void SizeCheck() const; // sanity checks

    // write perspective (size of arrays of free memory in ring buffer)
    std::size_t GetFreeMemorySizeArrayOne() const;
    std::size_t GetFreeMemorySizeArrayTwo() const;

private:
    // member variables
    std::vector<uint8_t> _buffer;

    std::size_t _size{0};

    std::size_t _wPos{0};
    std::size_t _rPos{0};
};

} // namespace Core
} // namespace SilKit