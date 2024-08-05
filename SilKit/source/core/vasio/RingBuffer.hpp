// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

namespace SilKit {
namespace Core {

class RingBuffer
{
public:
    // types
    using BufArray = std::pair<uint8_t*, size_t>;

public:
    // constructors and destructors
    RingBuffer(size_t capacity);

public:
    // public methods
    size_t Capacity() const;
    size_t Size() const;

    bool Peek(std::vector<uint8_t>& elem);
    bool Read(std::vector<uint8_t>& elem);

    // write perspective (arrays of free memory in ring buffer)
    auto GetFreeMemoryArrayOne() -> BufArray; // first physically contiguous array (starting at index _wPos)
    auto GetFreeMemoryArrayTwo() -> BufArray; // second physically contiguous array (starting at index 0)

    void AdvanceWPos(size_t numBytes); // public for access from VAsioPeer

    void Reserve(size_t newCapacity);

private:
    // private methods
    bool Empty() const;
    void AdvanceRPos(size_t numBytes);
    void SizeCheck() const; // sanity checks

    // write perspective (size of arrays of free memory in ring buffer)
    size_t GetFreeMemorySizeArrayOne() const;
    size_t GetFreeMemorySizeArrayTwo() const;

private:
    // member variables
    std::vector<uint8_t> _buffer;

    size_t _size{0};

    size_t _wPos{0};
    size_t _rPos{0};
};

} // namespace Core
} // namespace SilKit