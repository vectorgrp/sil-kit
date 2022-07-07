// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace SilKit {
namespace Util {
namespace SerDes {

inline namespace v1 {

class Serializer
{
public:
    // ----------------------------------------
    // CTOR, DTOR, Copy Operators
    Serializer() = default;
    Serializer(const Serializer& other) = default;
    Serializer(Serializer&& other) = default;
    ~Serializer() = default;

    auto operator=(const Serializer& other) -> Serializer& = default;
    auto operator=(Serializer&& other) -> Serializer& = default;

    /*! \brief Serializes an uint8_t to uint64_t, int8_t to int64_t
     *  \param bitSize The number of bits which shall be serialized.
     */
    template <typename T, typename std::enable_if<std::is_integral<T>::value
                                                      && !std::is_same<bool, typename std::decay<T>::type>::value,
                                                  int>::type = 0>
    void Serialize(T data, std::size_t bitSize)
    {
        auto byteSize = bitSize / 8;
        auto remainingBits = bitSize % 8;

        if (remainingBits)
            SerializeUnaligned(data, bitSize);
        else
            SerializeAligned(data, byteSize);
    }

    //! \brief Serializes a boolean value.
    template <typename T, typename std::enable_if_t<std::is_same<bool, typename std::decay_t<T>>::value, int> = 0>
    void Serialize(T data)
    {
        SerializeAligned(static_cast<uint8_t>(data ? 1 : 0), 1);
    }

    /*! \brief Serializes a float or double value.
     *
     *  NB: long double is disabled via static assert as they are treated
     *  differently by MSVC and GCC.
     */
    template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    void Serialize(T data)
    {
        static_assert(std::numeric_limits<T>::is_iec559,
                      "This compiler does not support IEEE 754 standard for floating points.");
        static_assert(!std::is_same<long double, typename std::decay_t<T>>::value,
                      "Serialization of long doubles is not supported. Cast to double at loss of precision.");
        Align();
        auto oldSize = mBuffer.size();
        mBuffer.resize(oldSize + sizeof(T));
        std::memcpy(&mBuffer[oldSize], &data, sizeof(T));
    }

    //! \brief Serializes a string.
    void Serialize(std::string string)
    {
        SerializeAligned(static_cast<uint32_t>(string.size()), 4);
        auto oldSize = mBuffer.size();
        mBuffer.resize(oldSize + string.size());
        std::copy(string.begin(), string.end(), mBuffer.begin() + oldSize);
    }

    //! \brief Serializes a dynamic byte array.
    void Serialize(const std::vector<uint8_t>& bytes)
    {
        SerializeAligned(static_cast<uint32_t>(bytes.size()), 4);
        auto oldSize = mBuffer.size();
        mBuffer.resize(oldSize + bytes.size());
        std::copy(bytes.begin(), bytes.end(), mBuffer.begin() + oldSize);
    }

    /*! \brief Serializes the start of an struct. */
    void BeginStruct() { Align(); }

    /*! \brief Serializes the end of an struct.*/
    void EndStruct() {}

    /*!  \brief Serializes the start of an array.
     *   NB: Dynamic array, i.e. also usable for lists.
     */
    void BeginArray(std::size_t size)
    {
#ifndef NDEBUG
        if (size > (std::numeric_limits<uint32_t>::max)()) // Additional bracers to prevent max-Macro collision
            throw std::length_error{"Array is too big"};
#endif // NDEBUG
        SerializeAligned(static_cast<uint32_t>(size), sizeof(uint32_t));
    }

    /*!  \brief Serializes the end of an array.
     *   NB: Dynamic array, i.e. also usable for lists.
     */
    void EndArray() {}

    /*! \brief Serializes the start of an optional value.
     *  \param isAvailable `true` if the optional value is set, otherwise `false`.
     *                     If `true`, the optional value must be serialized afterwards.
     */
    void BeginOptional(bool isAvailable) { Serialize(isAvailable); }

    /*! \brief Serializes the end of an optional value. */
    void EndOptional() {}

    void BeginUnion(int) { throw std::runtime_error("Unions are currently not supported."); }
    void EndUnion() { throw std::runtime_error("Unions are currently not supported."); }

    /*! \brief Resets the buffer. */
    void Reset()
    {
        mBuffer.resize(0);
        mUnalignedData = 0;
        mUnalignedBits = 0;
    }

    /*! \brief Releases and resets serializer buffer.
     *  \returns the serialized data.
     */
    auto ReleaseBuffer() -> std::vector<uint8_t>
    {
        Align();
        std::vector<uint8_t> buffer;
        buffer.reserve(mBuffer.size());
        mBuffer.swap(buffer);
        Reset();
        return buffer;
    }

private:
    // ----------------------------------------
    // private methods
    template <typename T,
              typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, int>::type = 0>
    void SerializeUnaligned(T data, std::size_t bitSize)
    {
        if (mUnalignedBits >= 8)
            throw std::runtime_error{"SerializedUnaligned(): current mUnalignedBits >= 8"};
        if (bitSize > sizeof(T) * 8)
            throw std::runtime_error{"SerializedUnaligned(): current bitSize > 8 * sizeof(T)"};

        mUnalignedData |= (static_cast<uint64_t>(data) << mUnalignedBits);

        auto flushBytes = (mUnalignedBits + bitSize) / 8;
        auto remainingBits = (mUnalignedBits + bitSize) % 8;

        if (flushBytes)
        {
            auto oldSize = mBuffer.size();
            mBuffer.resize(oldSize + flushBytes);
            memcpy(&mBuffer[oldSize], &mUnalignedData, flushBytes);

            assert(flushBytes <= 8);
            if (flushBytes == 8)
            {
                mUnalignedData = (data >> (64u - mUnalignedBits));
            }
            else
            {
                mUnalignedData >>= (flushBytes * 8);
            }
        }
        assert(remainingBits < 8);
        mUnalignedBits = static_cast<uint8_t>(remainingBits);
    }

    template <typename T,
              typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0>
    void SerializeUnaligned(T data, std::size_t bitSize)
    {
        auto udata = static_cast<typename std::make_unsigned<T>::type>(data);
        if (bitSize < sizeof(T) * 8)
        {
            T mask = (1ull << bitSize) - 1;
            udata &= mask;
        }
        SerializeUnaligned(udata, bitSize);
    }

    template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    void SerializeAligned(T data, std::size_t numBytes)
    {
        Align();
        auto oldSize = mBuffer.size();
        mBuffer.resize(oldSize + numBytes);
        std::memcpy(&mBuffer[oldSize], &data, numBytes);
    }

    void Align()
    {
        if (mUnalignedBits != 0)
        {
            mBuffer.push_back(static_cast<uint8_t>(mUnalignedData));
            mUnalignedData = 0;
            mUnalignedBits = 0;
        }
    }

private:
    // ----------------------------------------
    // private members

    std::vector<uint8_t> mBuffer;
    uint64_t mUnalignedData = 0;
    uint8_t mUnalignedBits = 0;
};

} // namespace v1
} // namespace SerDes
} // namespace Util
} // namespace SilKit
