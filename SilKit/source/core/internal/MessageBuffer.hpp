// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <string>
#include <type_traits>
#include <vector>
#include <array>
#include <limits>
#include <cstring>
#include <stdexcept>
#include <map>
#include <unordered_map>

#include "silkit/util/Span.hpp"

#include "util/Uuid.hpp"
#include "core/internal/ProtocolVersion.hpp"
#include "util/SharedSpan.hpp"

namespace SilKit {
namespace Core {

// The protocol version is directly tied to the MessageBuffer for backward compatibility in Ser/Des
struct end_of_buffer : public std::exception
{
};


class MessageBuffer;


/// Captures a reference to a MessageBuffer object and stores its current read position on construction. On
/// destruction, the read position of the captured MessageBuffer is reset to the stored value.
class MessageBufferPeeker
{
public:
    inline MessageBufferPeeker(MessageBuffer& messageBuffer);
    inline ~MessageBufferPeeker();

private:
    MessageBuffer& _messageBuffer;
    size_t _readPos;
};


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
    //! \brief Read from a shared blob without copying it. The buffer is read-only in this state.
    inline explicit MessageBuffer(Util::SharedSpan<uint8_t> blob);

    MessageBuffer(const MessageBuffer& other) = default;
    MessageBuffer(MessageBuffer&& other) = default;

public:
    // ----------------------------------------
    // Operator Implementations
    MessageBuffer& operator=(const MessageBuffer& other) = default;
    MessageBuffer& operator=(MessageBuffer&& other) = default;

public:
    // ----------------------------------------
    // Public methods for backward compatibility.

    // peek into raw data, e.g. for retrieving headers without modifying the buffer
    inline auto PeekData() const -> SilKit::Util::Span<const uint8_t>;
    inline auto ReadPos() const -> size_t;
    inline auto WritePos() const -> size_t;

    //! Set the format version to use for ser/des.
    inline void SetProtocolVersion(ProtocolVersion version);
    inline auto GetProtocolVersion() -> ProtocolVersion;

    inline void SetReadPos(size_t newReadPos);

public:
    // ----------------------------------------
    // Public methods

    //! \brief Return the underlying data storage by std::move and reset pointers
    inline auto ReleaseStorage() -> std::vector<uint8_t>;
    inline auto RemainingBytesLeft() const noexcept -> size_t;

public:
    // ----------------------------------------
    // Elementary streaming operators

    // ----------------------------------------
    // Integral Types
    template <typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int> = 0>
    inline MessageBuffer& operator<<(IntegerT t)
    {
        AssertWritable();
        if (_wPos + sizeof(IntegerT) > _storage.size())
        {
            _storage.resize(_storage.size() + sizeof(IntegerT));
        }
        std::memcpy(_storage.data() + _wPos, &t, sizeof(IntegerT));

        _wPos += sizeof(IntegerT);

        return *this;
    }
    template <typename IntegerT, typename std::enable_if_t<std::is_integral_v<IntegerT>, int> = 0>
    inline MessageBuffer& operator>>(IntegerT& t)
    {
        if (_rPos + sizeof(IntegerT) > Size())
            throw end_of_buffer{};

        std::memcpy(&t, Data() + _rPos, sizeof(IntegerT));
        _rPos += sizeof(IntegerT);

        return *this;
    }

    // ----------------------------------------
    // Floating-Point Types
    template <typename DoubleT, typename std::enable_if_t<std::is_floating_point<DoubleT>::value, int> = 0>
    inline MessageBuffer& operator<<(DoubleT t)
    {
        static_assert(std::numeric_limits<double>::is_iec559,
                      "This compiler does not support IEEE 754 standard for floating points.");

        AssertWritable();
        if (_wPos + sizeof(DoubleT) > _storage.size())
        {
            _storage.resize(_storage.size() + sizeof(DoubleT));
        }

        std::memcpy(_storage.data() + _wPos, &t, sizeof(DoubleT));
        _wPos += sizeof(DoubleT);

        return *this;
    }
    template <typename DoubleT, typename std::enable_if_t<std::is_floating_point<DoubleT>::value, int> = 0>
    inline MessageBuffer& operator>>(DoubleT& t)
    {
        static_assert(std::numeric_limits<double>::is_iec559,
                      "This compiler does not support IEEE 754 standard for floating points.");

        if (_rPos + sizeof(DoubleT) > Size())
            throw end_of_buffer{};

        std::memcpy(&t, Data() + _rPos, sizeof(DoubleT));
        _rPos += sizeof(DoubleT);

        return *this;
    }

    // --------------------------------------------------------------------------------
    // Enums
    template <typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    inline MessageBuffer& operator<<(T t)
    {
        return operator<<(static_cast<std::underlying_type_t<T>>(t));
    }
    template <typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    inline MessageBuffer& operator>>(T& t)
    {
        return operator>>(reinterpret_cast<std::underlying_type_t<T>&>(t));
    }

    // --------------------------------------------------------------------------------
    // std::string
    inline MessageBuffer& operator<<(const std::string& str);
    inline MessageBuffer& operator>>(std::string& str);
    // Prohibit C-strings to avoid improper serialization of blobs, which are not null terminated
    MessageBuffer& operator<<(const char*) = delete;
    MessageBuffer& operator<<(char*) = delete;
    MessageBuffer& operator>>(const char*) = delete;
    MessageBuffer& operator>>(char*) = delete;
    // --------------------------------------------------------------------------------
    // std::vector<uint8_t>
    inline MessageBuffer& operator<<(const std::vector<uint8_t>& vector);
    inline MessageBuffer& operator>>(std::vector<uint8_t>& vector);
    // --------------------------------------------------------------------------------
    // std::vector<T>
    template <typename ValueT>
    inline MessageBuffer& operator<<(const std::vector<ValueT>& vector);
    template <typename ValueT>
    inline MessageBuffer& operator>>(std::vector<ValueT>& vector);
    // --------------------------------------------------------------------------------
    // Util::SharedSpan<T>
    template <typename ValueT>
    inline MessageBuffer& operator<<(const Util::SharedSpan<ValueT>& sharedData);
    template <typename ValueT>
    inline MessageBuffer& operator>>(Util::SharedSpan<ValueT>& sharedData);
    // NB: only for uint8_t. A byte payload is serialized as a length prefix plus the bytes
    //     verbatim, so it can be aliased. Other element types are serialized element-wise and
    //     have alignment requirements, so they must be copied out.
    inline MessageBuffer& operator>>(Util::SharedSpan<uint8_t>& sharedData);
    // --------------------------------------------------------------------------------
    // Util::Span<T>
    inline MessageBuffer& operator<<(const Util::Span<const uint8_t>& sharedData);
    inline MessageBuffer& operator<<(const Util::Span<uint8_t>& sharedData);
    template <typename ValueT>
    inline MessageBuffer& operator<<(const Util::Span<ValueT>& sharedData);
    // --------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------
    // std::array<uint8_t, SIZE>
    template <size_t SIZE>
    inline MessageBuffer& operator<<(const std::array<uint8_t, SIZE>& array);
    template <size_t SIZE>
    inline MessageBuffer& operator>>(std::array<uint8_t, SIZE>& array);
    // --------------------------------------------------------------------------------
    // std::array<T, SIZE>
    template <typename ValueT, size_t SIZE>
    inline MessageBuffer& operator<<(const std::array<ValueT, SIZE>& array);
    template <typename ValueT, size_t SIZE>
    inline MessageBuffer& operator>>(std::array<ValueT, SIZE>& array);
    // --------------------------------------------------------------------------------
    // std::pair<T, U>
    template <typename T, typename U>
    inline MessageBuffer& operator<<(const std::pair<T, U>& pair);
    template <typename T, typename U>
    inline MessageBuffer& operator>>(std::pair<T, U>& pair);
    // --------------------------------------------------------------------------------
    // std::chrono::duration<Rep, Period> and system_clock::time_point
    template <class Rep, class Period>
    inline MessageBuffer& operator<<(std::chrono::duration<Rep, Period> duration);
    template <class Rep, class Period>
    inline MessageBuffer& operator>>(std::chrono::duration<Rep, Period>& duration);
    inline MessageBuffer& operator<<(std::chrono::system_clock::time_point time);
    inline MessageBuffer& operator>>(std::chrono::system_clock::time_point& time);

    // --------------------------------------------------------------------------------
    // void* is used as UserContext pointer. this needs to be stable across 32bit/64bit systems

    inline MessageBuffer& operator<<(const void* t);
    inline MessageBuffer& operator>>(void*& t);

    // --------------------------------------------------------------------------------
    // std::map<string,string>
    inline MessageBuffer& operator<<(const std::map<std::string, std::string>& msg);
    inline MessageBuffer& operator>>(std::map<std::string, std::string>& updatedMsg);

    // std::unordered_map<string,string>
    inline MessageBuffer& operator<<(const std::unordered_map<std::string, std::string>& msg);
    inline MessageBuffer& operator>>(std::unordered_map<std::string, std::string>& updatedMsg);
    // --------------------------------------------------------------------------------
    // Uuid
    inline MessageBuffer& operator<<(const Util::Uuid& uuid);
    inline MessageBuffer& operator>>(Util::Uuid& uuid);

public:
    void IncreaseCapacity(size_t capacity)
    {
        _storage.reserve(_storage.size() + capacity);
    }

private:
    //! \brief Start of the readable bytes, whether owned locally or shared.
    inline auto Data() const -> const uint8_t*
    {
        return _readsFromSharedStorage ? _sharedStorage.AsSpan().data() : _storage.data();
    }

    //! \brief Number of readable bytes, whether owned locally or shared.
    inline auto Size() const -> size_t
    {
        return _readsFromSharedStorage ? _sharedStorage.size() : _storage.size();
    }

    //! \brief A buffer constructed from a shared blob is read-only.
    inline void AssertWritable() const
    {
        if (_readsFromSharedStorage)
        {
            throw SilKitError{"MessageBuffer: attempt to write to a buffer backed by a shared blob"};
        }
    }

    //! \brief Append count raw bytes at the current write position.
    //
    // Uses a single insert for the common append case, which avoids the zero-fill that resize()
    // performs on bytes that are immediately overwritten anyway.
    inline void AppendBytes(const uint8_t* first, size_t count)
    {
        AssertWritable();

        if (count == 0)
        {
            return;
        }

        if (_wPos == _storage.size())
        {
            _storage.insert(_storage.end(), first, first + count);
        }
        else
        {
            if (_wPos + count > _storage.size())
            {
                _storage.resize(_wPos + count);
            }
            std::memcpy(_storage.data() + _wPos, first, count);
        }

        _wPos += count;
    }

    // ----------------------------------------
    // private members
    ProtocolVersion _protocolVersion{CurrentProtocolVersion()};
    std::vector<uint8_t> _storage;
    // Set instead of _storage when the buffer reads from a blob owned elsewhere. Deserialized
    // byte payloads then alias this blob rather than being copied out of it.
    Util::SharedSpan<uint8_t> _sharedStorage;
    bool _readsFromSharedStorage{false};
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

inline MessageBuffer::MessageBuffer(Util::SharedSpan<uint8_t> blob)
    : _sharedStorage{std::move(blob)}
    , _readsFromSharedStorage{true}
{
}

auto MessageBuffer::ReleaseStorage() -> std::vector<uint8_t>
{
    AssertWritable();
    _wPos = 0u;
    _rPos = 0u;
    return std::move(_storage);
}

inline auto MessageBuffer::RemainingBytesLeft() const noexcept -> size_t
{
    return (_rPos > Size()) ? 0 : (Size() - _rPos);
}

// --------------------------------------------------------------------------------
// std::string
MessageBuffer& MessageBuffer::operator<<(const std::string& str)
{
    if (str.size() > std::numeric_limits<uint32_t>::max())
        throw end_of_buffer{};

    *this << static_cast<uint32_t>(str.length());

    AppendBytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());

    return *this;
}
MessageBuffer& MessageBuffer::operator>>(std::string& str)
{
    uint32_t strLength{0u};
    *this >> strLength;

    if (_rPos + strLength > Size())
        throw end_of_buffer{};

    str = std::string(reinterpret_cast<const char*>(Data() + _rPos), strLength);
    _rPos += strLength;

    return *this;
}

// --------------------------------------------------------------------------------
// std::vector<uint8_t>
MessageBuffer& MessageBuffer::operator<<(const std::vector<uint8_t>& vector)
{
    const auto span = Util::ToSpan(vector);
    return *this << span;
}
MessageBuffer& MessageBuffer::operator>>(std::vector<uint8_t>& vector)
{
    uint32_t vectorSize{0u};
    *this >> vectorSize;

    if (_rPos + vectorSize > Size())
        throw end_of_buffer{};

    vector = std::vector<uint8_t>(Data() + _rPos, Data() + _rPos + vectorSize);
    _rPos += vectorSize;

    return *this;
}

// --------------------------------------------------------------------------------
// std::vector<T>
template <typename ValueT>
MessageBuffer& MessageBuffer::operator<<(const std::vector<ValueT>& vector)
{
    if (vector.size() > std::numeric_limits<uint32_t>::max())
        throw end_of_buffer{};

    IncreaseCapacity(sizeof(uint32_t) + vector.size());

    *this << static_cast<uint32_t>(vector.size());

    for (auto&& value : vector)
    {
        *this << value;
    }

    return *this;
}
template <typename ValueT>
MessageBuffer& MessageBuffer::operator>>(std::vector<ValueT>& vector)
{
    uint32_t vectorSize{0u};
    *this >> vectorSize;

    if (_rPos + vectorSize > Size())
        throw end_of_buffer{};

    vector.resize(vectorSize);
    for (uint32_t i = 0; i < vectorSize; i++)
    {
        *this >> vector[i];
    }

    return *this;
}


// --------------------------------------------------------------------------------
// Util::Span<T>
inline MessageBuffer& MessageBuffer::operator<<(const Util::Span<uint8_t>& span)
{
    const auto cspan = Util::Span<const uint8_t>{span};
    return *this << cspan;
}

inline MessageBuffer& MessageBuffer::operator<<(const Util::Span<const uint8_t>& span)
{
    if (span.size() > std::numeric_limits<uint32_t>::max())
        throw end_of_buffer{};

    *this << static_cast<uint32_t>(span.size());

    AppendBytes(span.data(), span.size());

    return *this;
}

template <typename ValueT>
inline MessageBuffer& MessageBuffer::operator<<(const Util::Span<ValueT>& span)
{
    if (span.size() > std::numeric_limits<uint32_t>::max())
    {
        throw end_of_buffer{};
    }
    IncreaseCapacity(sizeof(uint32_t) + span.size());

    *this << static_cast<uint32_t>(span.size());

    for (auto&& value : span)
    {
        *this << value;
    }

    return *this;
}
// --------------------------------------------------------------------------------
// Util::SharedSpan<T>
template <typename ValueT>
inline MessageBuffer& MessageBuffer::operator<<(const Util::SharedSpan<ValueT>& sharedData)
{
    const auto span = sharedData.AsSpan();
    return *this << span;
}

template <typename ValueT>
inline MessageBuffer& MessageBuffer::operator>>(Util::SharedSpan<ValueT>& sharedData)
{
    std::vector<ValueT> vector;
    *this >> vector;

    sharedData = Util::SharedSpan<ValueT>{std::move(vector)};

    return *this;
}

inline MessageBuffer& MessageBuffer::operator>>(Util::SharedSpan<uint8_t>& sharedData)
{
    uint32_t payloadSize{0u};
    *this >> payloadSize;

    if (_rPos + payloadSize > Size())
        throw end_of_buffer{};

    if (_readsFromSharedStorage)
    {
        // Alias the shared blob instead of copying the payload out of it. Subspan is range
        // checked and keeps the blob alive for as long as the payload is referenced.
        sharedData = _sharedStorage.Subspan(_rPos, payloadSize);
    }
    else
    {
        sharedData = Util::SharedSpan<uint8_t>{std::vector<uint8_t>(Data() + _rPos, Data() + _rPos + payloadSize)};
    }

    _rPos += payloadSize;

    return *this;
}

// --------------------------------------------------------------------------------
// std::array<uint8_t, SIZE>
template <size_t SIZE>
MessageBuffer& MessageBuffer::operator<<(const std::array<uint8_t, SIZE>& array)
{
    if (array.size() > std::numeric_limits<uint32_t>::max())
        throw end_of_buffer{};

    AssertWritable();
    if (_wPos + array.size() > _storage.size())
    {
        _storage.resize(_wPos + array.size());
    }

    std::copy(array.begin(), array.end(), _storage.begin() + _wPos);
    _wPos += array.size();

    return *this;
}
template <size_t SIZE>
MessageBuffer& MessageBuffer::operator>>(std::array<uint8_t, SIZE>& array)
{
    if (_rPos + array.size() > Size())
        throw end_of_buffer{};

    std::copy(Data() + _rPos, Data() + _rPos + array.size(), array.begin());
    _rPos += array.size();

    return *this;
}

// --------------------------------------------------------------------------------
// std::array<T, SIZE>
template <typename ValueT, size_t SIZE>
MessageBuffer& MessageBuffer::operator<<(const std::array<ValueT, SIZE>& array)
{
    if (array.size() > std::numeric_limits<uint32_t>::max())
        throw end_of_buffer{};

    for (auto&& value : array)
        *this << value;

    return *this;
}
template <typename ValueT, size_t SIZE>
MessageBuffer& MessageBuffer::operator>>(std::array<ValueT, SIZE>& array)
{
    if (_rPos + array.size() > Size())
        throw end_of_buffer{};

    for (auto&& value : array)
    {
        *this >> value;
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
    Rep count{};
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

// --------------------------------------------------------------------------------
// void ptrs
inline MessageBuffer& MessageBuffer::operator<<(const void* t)
{
    *this << reinterpret_cast<uint64_t>(t);
    return *this;
}

inline MessageBuffer& MessageBuffer::operator>>(void*& t)
{
    uint64_t value{0};
    *this >> value;
    t = reinterpret_cast<void*>(value);
    return *this;
}


// --------------------------------------------------------------------------------
//Special case for std::map<std::string, std::string> as used in ServiceDescription::supplementalData
// We encode it as follows:
// 0    NUM_ELEMENTS N
// 1    STRING key1
// 2    STRING value1
// ...
// 2N    STRING keyN
// 2N+1  STRING valueN
//
// NB: for a generic approach we would have to encode the key type and the element type somehow

inline MessageBuffer& MessageBuffer::operator<<(const std::map<std::string, std::string>& msg)
{
    *this << static_cast<uint32_t>(msg.size());
    for (auto&& kv : msg)
    {
        *this << kv.first << kv.second;
    }
    return *this;
}

inline MessageBuffer& MessageBuffer::operator>>(std::map<std::string, std::string>& updatedMsg)
{
    std::map<std::string, std::string> tmp; // do not modify updatedMsg until we validated the input
    uint32_t numElements{0};
    *this >> numElements;

    for (auto i = 0u; i < numElements; i++)
    {
        std::string key;
        std::string value;
        *this >> key >> value;
        tmp[key] = std::move(value);
    }
    if (numElements != tmp.size())
    {
        throw SilKitError("MessageBuffer unable to deserialize std::map<std::string, std::string>");
    }
    updatedMsg = std::move(tmp);
    return *this;
}


inline MessageBuffer& MessageBuffer::operator<<(const std::unordered_map<std::string, std::string>& msg)
{
    *this << static_cast<uint32_t>(msg.size());
    for (auto&& kv : msg)
    {
        *this << kv.first << kv.second;
    }
    return *this;
}

inline MessageBuffer& MessageBuffer::operator>>(std::unordered_map<std::string, std::string>& updatedMsg)
{
    std::unordered_map<std::string, std::string> tmp; // do not modify updatedMsg until we validated the input
    uint32_t numElements{0};
    *this >> numElements;

    for (auto i = 0u; i < numElements; i++)
    {
        std::string key;
        std::string value;
        *this >> key >> value;
        tmp[key] = std::move(value);
    }
    if (numElements != tmp.size())
    {
        throw SilKitError("MessageBuffer unable to deserialize std::unordered_map<std::string, std::string>");
    }
    updatedMsg = std::move(tmp);
    return *this;
}

template <typename T, typename U>
inline MessageBuffer& MessageBuffer::operator<<(const std::pair<T, U>& pair)
{
    *this << pair.first << pair.second;
    return *this;
}

template <typename T, typename U>
inline MessageBuffer& MessageBuffer::operator>>(std::pair<T, U>& pair)
{
    *this >> pair.first >> pair.second;
    return *this;
}


// --------------------------------------------------------------------------------
// Uuid

inline MessageBuffer& MessageBuffer::operator<<(const Util::Uuid& uuid)
{
    *this << uuid.ab << uuid.cd;
    return *this;
}

inline MessageBuffer& MessageBuffer::operator>>(Util::Uuid& uuid)
{
    *this >> uuid.ab >> uuid.cd;
    return *this;
}

// --------------------------------------------------------------------------------
// Public methods for backward compatibility.

inline void MessageBuffer::SetProtocolVersion(ProtocolVersion version)
{
    _protocolVersion = version;
}
inline auto MessageBuffer::GetProtocolVersion() -> ProtocolVersion
{
    return _protocolVersion;
}


inline auto MessageBuffer::PeekData() const -> SilKit::Util::Span<const uint8_t>
{
    return _readsFromSharedStorage ? _sharedStorage.AsSpan() : SilKit::Util::ToSpan(_storage);
}
inline auto MessageBuffer::ReadPos() const -> size_t
{
    return _rPos;
}

inline auto MessageBuffer::WritePos() const -> size_t
{
    return _wPos;
}

inline void MessageBuffer::SetReadPos(size_t newReadPos)
{
    _rPos = newReadPos;
}


MessageBufferPeeker::MessageBufferPeeker(MessageBuffer& messageBuffer)
    : _messageBuffer{messageBuffer}
    , _readPos{_messageBuffer.ReadPos()}
{
}

MessageBufferPeeker::~MessageBufferPeeker()
{
    _messageBuffer.SetReadPos(_readPos);
}

} // namespace Core
} // namespace SilKit
