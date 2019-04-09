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
    
enum class MessageKind : uint8_t
{
    Invalid = 0,
    NewConnection = 1,
    IbData = 2
};
    
struct MessageHeader
{
    MessageKind kind;
};
    
struct MessageBuffer
{
    inline MessageBuffer();
    inline MessageBuffer(MessageKind kind);

    MessageBuffer(const MessageBuffer& other) = default;
    MessageBuffer(MessageBuffer&& other) = default;

    MessageBuffer& operator=(const MessageBuffer& other) = default;
    MessageBuffer& operator=(MessageBuffer&& other) = default;

    inline void SetMessageKind(MessageKind kind);
    inline auto GetMessageKind() const -> MessageKind;


    // --------------------------------------------------------------------------------
    // Integral Types
    template<typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int> = 0>
    MessageBuffer& operator<<(IntegerT t);
    template<typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int> = 0>
    MessageBuffer& operator>>(IntegerT& t);

    // --------------------------------------------------------------------------------
    // Enums
    template<typename T, typename std::enable_if_t<std::is_enum_v<T>, int> = 0>
    MessageBuffer& operator<<(T t);
    template<typename T, typename std::enable_if_t<std::is_enum_v<T>, int> = 0>
    MessageBuffer& operator>>(T& t);

    // --------------------------------------------------------------------------------
    // std::string
    MessageBuffer& operator<<(const std::string& str);
    MessageBuffer& operator>>(std::string& str);

    // std::vector<uint8_t>
    MessageBuffer& operator<<(const std::vector<uint8_t>& vector)
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
    MessageBuffer& operator>>(std::vector<uint8_t>& vector)
    {
        uint32_t vectorSize{0u};
        *this >> vectorSize;

        if (rPos + vectorSize > store.size())
            throw end_of_buffer{};

        vector = std::vector<uint8_t>(store.begin() + rPos, store.begin() + rPos + vectorSize);
        rPos += vectorSize;

        return *this;
    }

    // std::vector<T>
    template<typename ValueT>
    MessageBuffer& operator<<(const std::vector<ValueT>& vector)
    {
        assert(vector.size() <= std::numeric_limits<uint32_t>::max());
        *this << static_cast<uint32_t>(vector.size());

        for (auto&& value : vector)
            *this << value;

        return *this;
    }
    template<typename ValueT>
    MessageBuffer& operator>>(std::vector<ValueT>& vector)
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
    // std::chrono::duration<Rep, Period> and system_clock::time_point
    template <class Rep, class Period>
    MessageBuffer& operator<<(std::chrono::duration<Rep, Period> duration);
    template <class Rep, class Period>
    MessageBuffer& operator>>(std::chrono::duration<Rep, Period>& duration);
    MessageBuffer& operator<<(std::chrono::system_clock::time_point time);
    MessageBuffer& operator>>(std::chrono::system_clock::time_point& time);

private:
    std::vector<uint8_t> store;
    std::size_t wPos{1u};
    std::size_t rPos{1u};
};

// ================================================================================
//  Inline Implementations
// ================================================================================
MessageBuffer::MessageBuffer()
    : store{1, static_cast<uint8_t>(MessageKind::Invalid)}
{
}

MessageBuffer::MessageBuffer(MessageKind kind)
    : store{1, static_cast<uint8_t>(kind)}
{
}

void MessageBuffer::SetMessageKind(MessageKind kind)
{
    store[0] = static_cast<std::underlying_type_t<MessageKind>>(kind);
}

auto MessageBuffer::GetMessageKind() const -> MessageKind
{
    return static_cast<MessageKind>(store[0]);
}

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
