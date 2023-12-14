// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "silkit/util/Span.hpp"

#include <cstddef>
#include <cstdint>


namespace VSilKit {


namespace Details {

struct BufferImpl
{
    uintptr_t addr{0};
    size_t size{0};

    BufferImpl() = default;

    BufferImpl(uintptr_t addr_, size_t size_)
        : addr{addr_}
        , size{size_}
    {
    }

    auto SliceOff(size_t prefixLength) -> BufferImpl
    {
        if (size >= prefixLength)
        {
            BufferImpl prefix{addr, prefixLength};
            addr += static_cast<uintptr_t>(prefixLength);
            size -= prefixLength;
            return prefix;
        }
        else
        {
            BufferImpl prefix{*this};
            addr = 0;
            size = 0;
            return prefix;
        }
    }
};

} // namespace Details


class ConstBuffer
{
    Details::BufferImpl _impl;

public:
    ConstBuffer() = default;

    ConstBuffer(const void* data, size_t size)
        : _impl{reinterpret_cast<uintptr_t>(data), size}
    {
    }

private:
    explicit ConstBuffer(Details::BufferImpl impl)
        : _impl{impl}
    {
    }

public:
    auto GetData() const -> const void*
    {
        return reinterpret_cast<const void*>(_impl.addr);
    }

    auto GetSize() const -> size_t
    {
        return _impl.size;
    }

    auto SliceOff(size_t prefixSize) -> ConstBuffer
    {
        return ConstBuffer{_impl.SliceOff(prefixSize)};
    }
};


using ConstBufferSequence = SilKit::Util::Span<ConstBuffer>;


class MutableBuffer
{
    Details::BufferImpl _impl;

public:
    MutableBuffer() = default;

    MutableBuffer(const void* data, size_t size)
        : _impl{reinterpret_cast<uintptr_t>(data), size}
    {
    }

private:
    explicit MutableBuffer(Details::BufferImpl impl)
        : _impl{impl}
    {
    }

public:
    auto GetData() const -> void*
    {
        return reinterpret_cast<void*>(_impl.addr);
    }

    auto GetSize() const -> size_t
    {
        return _impl.size;
    }

    auto SliceOff(size_t prefixSize) -> MutableBuffer
    {
        return MutableBuffer{_impl.SliceOff(prefixSize)};
    }
};


using MutableBufferSequence = SilKit::Util::Span<MutableBuffer>;


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::ConstBuffer;
using VSilKit::ConstBufferSequence;
using VSilKit::MutableBuffer;
using VSilKit::MutableBufferSequence;
} // namespace Core
} // namespace SilKit
