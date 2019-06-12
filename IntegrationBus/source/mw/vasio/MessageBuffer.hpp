// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <string>
#include <type_traits>
#include <vector>

namespace ib {
namespace mw {

struct end_of_buffer : public std::exception {};    

class MessageBuffer
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    inline MessageBuffer() = default;
    inline MessageBuffer(std::vector<uint8_t> data);

    MessageBuffer(const MessageBuffer& other) = default;
    MessageBuffer(MessageBuffer&& other) = default;

public:
    // ----------------------------------------
    // Operator Implementations
    MessageBuffer& operator=(const MessageBuffer& other) = default;
    MessageBuffer& operator=(MessageBuffer&& other) = default;

public:
    // ----------------------------------------
    // Public methods

    //! \brief Return the underlying data storage by std::move and reset pointers
    inline auto ReleaseStorage()->std::vector<uint8_t>;

public:
    // ----------------------------------------
    // Elementary streaming operators

    // ----------------------------------------
    // Integral Types
    template<typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int> = 0>
    inline MessageBuffer& operator<<(IntegerT t)
    {
        if (_wPos + sizeof(IntegerT) > _storage.size())
        {
            _storage.resize(_storage.size() + sizeof(IntegerT));
        }
        IntegerT* dst = reinterpret_cast<IntegerT*>(_storage.data() + _wPos);
        *dst = t;
        _wPos += sizeof(IntegerT);

        return *this;
    }
    template<typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int> = 0>
    inline MessageBuffer& operator>>(IntegerT& t)
    {
        if (_rPos + sizeof(IntegerT) > _storage.size())
            throw end_of_buffer{};

        const IntegerT* src = reinterpret_cast<const IntegerT*>(_storage.data() + _rPos);
        t = *src;
        _rPos += sizeof(IntegerT);

        return *this;
    }

    // --------------------------------------------------------------------------------
    // Enums
    template<typename T, typename std::enable_if_t<std::is_enum_v<T>, int> = 0>
    inline MessageBuffer& operator<<(T t)
    {
        return operator<<(static_cast<std::underlying_type_t<T>>(t));
    }
    template<typename T, typename std::enable_if_t<std::is_enum_v<T>, int> = 0>
    inline MessageBuffer& operator>>(T& t)
    {
        return operator>>(reinterpret_cast<std::underlying_type_t<T>&>(t));
    }

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
    // ----------------------------------------
    // private members
    std::vector<uint8_t> _storage;
    std::size_t _wPos{0u};
    std::size_t _rPos{0u};
};

// ================================================================================
//  Inline Implementations
// ================================================================================
MessageBuffer::MessageBuffer(std::vector<uint8_t> data)
    : _storage{std::move(data)}
    , _wPos{_storage.size()}
    , _rPos{0u}
{
}

auto MessageBuffer::ReleaseStorage() -> std::vector<uint8_t>
{
    _wPos = 0u;
    _rPos = 0u;
    return std::move(_storage);
}

// --------------------------------------------------------------------------------
// std::string
MessageBuffer& MessageBuffer::operator<<(const std::string& str)
{
    if (str.size() > std::numeric_limits<uint32_t>::max())
        throw end_of_buffer{};

    *this << static_cast<uint32_t>(str.length());

    if (_wPos + str.size() > _storage.size())
    {
        _storage.resize(_wPos + str.size());
    }

    std::copy(str.begin(), str.end(), _storage.begin() + _wPos);
    _wPos += str.size();

    return *this;
}
MessageBuffer& MessageBuffer::operator>>(std::string& str)
{
    uint32_t strLength{0u};
    *this >> strLength;

    if (_rPos + strLength > _storage.size())
        throw end_of_buffer{};

    str = std::string(_storage.begin() + _rPos, _storage.begin() + _rPos + strLength);
    _rPos += strLength;

    return *this;
}

// --------------------------------------------------------------------------------
// std::vector<uint8_t>
MessageBuffer& MessageBuffer::operator<<(const std::vector<uint8_t>& vector)
{
    if (vector.size() > std::numeric_limits<uint32_t>::max())
        throw end_of_buffer{};

    *this << static_cast<uint32_t>(vector.size());

    if (_wPos + vector.size() > _storage.size())
    {
        _storage.resize(_wPos + vector.size());
    }

    std::copy(vector.begin(), vector.end(), _storage.begin() + _wPos);
    _wPos += vector.size();

    return *this;
}
MessageBuffer& MessageBuffer::operator>>(std::vector<uint8_t>& vector)
{
    uint32_t vectorSize{0u};
    *this >> vectorSize;

    if (_rPos + vectorSize > _storage.size())
        throw end_of_buffer{};

    vector = std::vector<uint8_t>(_storage.begin() + _rPos, _storage.begin() + _rPos + vectorSize);
    _rPos += vectorSize;

    return *this;
}

// --------------------------------------------------------------------------------
// std::vector<T>
template<typename ValueT>
MessageBuffer& MessageBuffer::operator<<(const std::vector<ValueT>& vector)
{
    if (vector.size() > std::numeric_limits<uint32_t>::max())
        throw end_of_buffer{};

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
