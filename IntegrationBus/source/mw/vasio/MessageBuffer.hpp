// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cassert>

#include <chrono>
#include <string>
#include <chrono>
#include <vector>
#include <type_traits>

namespace ib {
namespace mw {

struct end_of_buffer : public std::exception {};    
    
struct MessageBuffer
{
    inline MessageBuffer() = default;

    MessageBuffer(const MessageBuffer& other) = default;
    MessageBuffer(MessageBuffer&& other) = default;

    MessageBuffer& operator=(const MessageBuffer& other) = default;
    MessageBuffer& operator=(MessageBuffer&& other) = default;

    // --------------------------------------------------------------------------------
    // Integral Types
    template<typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int> = 0>
    inline MessageBuffer& operator<<(IntegerT t);
    template<typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int> = 0>
    inline MessageBuffer& operator>>(IntegerT& t);

    // --------------------------------------------------------------------------------
    // Enums
    template<typename T, typename std::enable_if_t<std::is_enum_v<T>, int> = 0>
    inline MessageBuffer& operator<<(T t);
    template<typename T, typename std::enable_if_t<std::is_enum_v<T>, int> = 0>
    inline MessageBuffer& operator>>(T& t);

    // --------------------------------------------------------------------------------
    // std::string
    inline MessageBuffer& operator<<(const std::string& str);
    inline MessageBuffer& operator>>(std::string& str);
    // --------------------------------------------------------------------------------
    // std::vector<uint8_t>
    inline MessageBuffer& operator<<(const std::vector<uint8_t>& vector);
    inline MessageBuffer& operator>>(std::vector<uint8_t>& vector);
    // --------------------------------------------------------------------------------
    // std::vector<T>
    template<typename ValueT>
    inline MessageBuffer& operator<<(const std::vector<ValueT>& vector);
    template<typename ValueT>
    inline MessageBuffer& operator>>(std::vector<ValueT>& vector);
    // --------------------------------------------------------------------------------
    // std::chrono::duration<Rep, Period> and system_clock::time_point
    template <class Rep, class Period>
    inline MessageBuffer& operator<<(std::chrono::duration<Rep, Period> duration);
    template <class Rep, class Period>
    inline MessageBuffer& operator>>(std::chrono::duration<Rep, Period>& duration);
    inline MessageBuffer& operator<<(std::chrono::system_clock::time_point time);
    inline MessageBuffer& operator>>(std::chrono::system_clock::time_point& time);

private:
    std::vector<uint8_t> store;
    std::size_t wPos{0u};
    std::size_t rPos{0u};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

// --------------------------------------------------------------------------------
// Integral Types
template<typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int>>
MessageBuffer& MessageBuffer::operator<<(IntegerT t)
{
    if (wPos + sizeof(IntegerT) > store.size())
    {
        store.resize(store.size() + sizeof(IntegerT));
    }
    IntegerT* dst = reinterpret_cast<IntegerT*>(store.data() + wPos);
    *dst = t;
    wPos += sizeof(IntegerT);

    return *this;
}
template<typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int>>
MessageBuffer& MessageBuffer::operator>>(IntegerT& t)
{
    if (rPos + sizeof(IntegerT) > store.size())
        throw end_of_buffer{};

    const IntegerT* src = reinterpret_cast<const IntegerT*>(store.data() + rPos);
    t = *src;
    rPos += sizeof(IntegerT);

    return *this;
}

// --------------------------------------------------------------------------------
// Enums
template<typename T, typename std::enable_if_t<std::is_enum_v<T>, int>>
MessageBuffer& MessageBuffer::operator<<(T t)
{
    return operator<<(static_cast<std::underlying_type_t<T>>(t));
}
template<typename T, typename std::enable_if_t<std::is_enum_v<T>, int>>
MessageBuffer& MessageBuffer::operator>>(T& t)
{
    return operator>>(reinterpret_cast<std::underlying_type_t<T>&>(t));
}

// --------------------------------------------------------------------------------
// std::string
MessageBuffer& MessageBuffer::operator<<(const std::string& str)
{
    assert(str.size() <= std::numeric_limits<uint32_t>::max());
    *this << static_cast<uint32_t>(str.length());

    if (wPos + str.size() > store.size())
    {
        store.resize(store.size() + str.size());
    }

    std::copy(str.begin(), str.end(), store.begin() + wPos);
    wPos += str.size();

    return *this;
}
MessageBuffer& MessageBuffer::operator>>(std::string& str)
{
    uint32_t strLength{0u};
    *this >> strLength;

    if (rPos + strLength > store.size())
        throw end_of_buffer{};

    str = std::string(store.begin() + rPos, store.begin() + rPos + strLength);
    rPos += strLength;

    return *this;
}

// --------------------------------------------------------------------------------
// std::vector<uint8_t>
MessageBuffer& MessageBuffer::operator<<(const std::vector<uint8_t>& vector)
{
    assert(vector.size() <= std::numeric_limits<uint32_t>::max());
    *this << static_cast<uint32_t>(vector.size());

    if (wPos + vector.size() > store.size())
    {
        store.resize(store.size() + vector.size());
    }

    std::copy(vector.begin(), vector.end(), store.begin() + wPos);
    wPos += vector.size();

    return *this;
}
MessageBuffer& MessageBuffer::operator>>(std::vector<uint8_t>& vector)
{
    uint32_t vectorSize{0u};
    *this >> vectorSize;

    if (rPos + vectorSize > store.size())
        throw end_of_buffer{};

    vector = std::vector<uint8_t>(store.begin() + rPos, store.begin() + rPos + vectorSize);
    rPos += vectorSize;

    return *this;
}

// --------------------------------------------------------------------------------
// std::vector<T>
template<typename ValueT>
MessageBuffer& MessageBuffer::operator<<(const std::vector<ValueT>& vector)
{
    assert(vector.size() <= std::numeric_limits<uint32_t>::max());
    *this << static_cast<uint32_t>(vector.size());

    for (auto&& value : vector)
        *this << value;

    return *this;
}
template<typename ValueT>
MessageBuffer& MessageBuffer::operator>>(std::vector<ValueT>& vector)
{
    uint32_t vectorSize{0u};
    *this >> vectorSize;

    vector.resize(vectorSize);
    for (uint32_t i = 0; i < vectorSize; i++)
    {
        *this >> vector[i];
    }

    return *this;
}

// --------------------------------------------------------------------------------
// std::chrono::duration<Rep, Period>
template <class Rep, class Period>
MessageBuffer& MessageBuffer::operator<<(std::chrono::duration<Rep, Period> duration)
{
    *this << duration.count();
    return *this;
}
template <class Rep, class Period>
MessageBuffer& MessageBuffer::operator>>(std::chrono::duration<Rep, Period>& duration)
{
    Rep count = duration.count();
    *this >> count;
    duration = std::chrono::duration<Rep, Period>{count};
    return *this;
}


MessageBuffer& MessageBuffer::operator<<(std::chrono::system_clock::time_point time)
{
    *this << std::chrono::duration_cast<std::chrono::microseconds>(time.time_since_epoch());
    return *this;
}
MessageBuffer& MessageBuffer::operator>>(std::chrono::system_clock::time_point& time)
{
    std::chrono::microseconds time_since_epoch{};
    *this >> time_since_epoch;
    time = std::chrono::system_clock::time_point{time_since_epoch};
    return *this;
}

    
} // mw
} // namespace ib
