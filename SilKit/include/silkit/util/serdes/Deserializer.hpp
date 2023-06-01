/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Util {
namespace SerDes {

inline namespace v1 {

class Deserializer
{
public:
    // ----------------------------------------
    // CTOR, DTOR, Copy Operators
    Deserializer() = default;
    Deserializer(std::vector<uint8_t> buffer)
        : mBuffer(std::move(buffer))
    {
    }
    Deserializer(const Deserializer& other) = default;
    Deserializer(Deserializer&& other) = default;
    ~Deserializer() = default;

    auto operator=(const Deserializer& other) -> Deserializer& = default;
    auto operator=(Deserializer&& other) -> Deserializer& = default;

    /*! \brief Deserializes uint8_t through uint64_t, int8_t through int64_t.
     *  \param bitSize The number of bits which shall be deserialized.
     *  \returns The deserialized value
     */
    template <typename T,
              typename std::enable_if_t<
                  std::is_integral<T>::value && !std::is_same<bool, typename std::decay_t<T>>::value, int> = 0>
    auto Deserialize(std::size_t bitSize) -> T
    {
        auto byteSize = bitSize / 8;
        auto remainingBits = bitSize % 8;

        if (remainingBits)
            return DeserializeUnaligned<T>(bitSize);
        else
            return DeserializeAligned<T>(byteSize);
    }

    /*! \brief Deserializes a boolean value.
     *  \returns The deserialized value
     */
    template <typename T, typename std::enable_if_t<std::is_same<bool, typename std::decay_t<T>>::value, int> = 0>
    auto Deserialize() -> T
    {
        return DeserializeAligned<uint8_t>(1) ? true : false;
    }

    /*! \brief Deserializes a float or double value.
     *  \returns The deserialized value
     */
    template <typename T, typename std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    auto Deserialize() -> T
    {
        static_assert(std::numeric_limits<T>::is_iec559,
                      "This compiler does not support IEEE 754 standard for floating points.");
        static_assert(!std::is_same<long double, typename std::decay_t<T>>::value,
                      "Serialization of long doubles is not supported. Cast to double at loss of precision.");
        Align();
        AssertCapacity(sizeof(T));

        T result;
        std::memcpy(&result, &mBuffer[mReadPos], sizeof(T));
        mReadPos += sizeof(T);
        return result;
    }

    /*! \brief Deserializes a string value.
     *  \returns The deserialized value
     */
    template <typename T, typename std::enable_if_t<std::is_same<std::string, T>::value, int> = 0>
    auto Deserialize() -> T
    {
        auto size = DeserializeAligned<uint32_t>(4);
        AssertCapacity(size);
        std::string result{mBuffer.begin() + mReadPos, mBuffer.begin() + mReadPos + size};
        mReadPos += size;
        return result;
    }

    /*! \brief Deserializes a byte array.
     *  \returns The deserialized value
     */
    template <typename T, typename std::enable_if_t<std::is_same<std::vector<uint8_t>, T>::value, int> = 0>
    auto Deserialize() -> T
    {
        auto size = DeserializeAligned<uint32_t>(4);
        AssertCapacity(size);
        std::vector<uint8_t> result{mBuffer.begin() + mReadPos, mBuffer.begin() + mReadPos + size};
        mReadPos += size;
        return result;
    }

    /*! \brief Deserializes the start of a struct. */
    void BeginStruct() { Align(); }

    /*! \brief Deserializes the end of a struct. */
    void EndStruct() {}

    /*! \brief Deserializes the start of an array or list.
     *  Note: Because the array size is serialized as well, dynamic arrays aka. lists are also supported.
     *  \returns The size of the array (in elements).
     */
    auto BeginArray() -> std::size_t { return DeserializeAligned<uint32_t>(sizeof(uint32_t)); }

    /*! \brief Deserializes the end of an array or list.
     */
    void EndArray() {}

    /*! \brief Deserializes the start of an optional value.
     *  \returns `true` if the value is set, otherwise `false`.
     */
    auto BeginOptional() -> bool { return Deserialize<bool>(); }

    /*! \brief Deserializes the end of an optional value. */
    void EndOptional() {}

    auto BeginUnion() -> int { throw SilKitError("Unions are currently not supported."); }
    void EndUnion() { throw SilKitError("Unions are currently not supported."); }

    /*! \brief Resets the buffer and replaces it with another one.
     *  \param buffer The new data buffer.
     */
    void Reset(std::vector<uint8_t> buffer)
    {
        mBuffer = std::move(buffer);
        mReadPos = 0;

        mUnalignedData = 0;
        mUnalignedBits = 0;
    }

private:
    // ----------------------------------------
    // private methods
    template <typename T, typename std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, int> = 0>
    auto DeserializeUnaligned(std::size_t bitSize) -> T
    {
        if (mUnalignedBits >= 8)
            throw SilKitError{"DeserializedUnaligned(): current mUnalignedBits >= 8"};
        if (bitSize > sizeof(T) * 8)
            throw SilKitError{"DeserializedUnaligned(): current bitSize > 8 * sizeof(T)"};

        uint64_t readData = 0;
        std::size_t readBits = 0;

        if (mUnalignedBits < bitSize)
        {
            auto missingBits = bitSize - mUnalignedBits;
            auto readBytes = missingBits / 8;
            auto unalignedBits = missingBits % 8;
            if (unalignedBits > 0)
                readBytes += 1;
            readBits = readBytes * 8;

            AssertCapacity(readBytes);
            std::memcpy(&readData, &mBuffer[mReadPos], readBytes);
            mReadPos += readBytes;

            mUnalignedData |= (readData << mUnalignedBits);

            if (mUnalignedBits + readBits > 64u)
            {
                readData >>= (64u - mUnalignedBits);
                readBits = (mUnalignedBits + readBits - 64u);
                mUnalignedBits = 64u;
            }
            else
            {
                mUnalignedBits += readBits;
                readBits = 0;
            }
        }

        T result = static_cast<T>(mUnalignedData);
        if (bitSize < sizeof(T) * 8)
        {
            T mask = (1ull << bitSize) - 1;
            result &= mask;
        }

        mUnalignedData >>= bitSize;
        mUnalignedBits -= bitSize;

        if (readBits)
        {
            mUnalignedData |= (readData << mUnalignedBits);
            mUnalignedBits += readBits;
        }
        return result;
    }
    template <typename T, typename std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, int> = 0>
    auto DeserializeUnaligned(std::size_t bitSize) -> T
    {
        using uint_t = typename std::make_unsigned_t<T>;
        auto udata = DeserializeUnaligned<uint_t>(bitSize);

        // Make sure that >> on signed integers is an arithmetic shift. (right shift on signed integers is
        // implementation defined)
        static_assert(static_cast<int64_t>((std::numeric_limits<uint64_t>::max)()) >> 63
                          == static_cast<int64_t>((std::numeric_limits<uint64_t>::max)()),
                      "Right shift on signed integers must be an arithmetic shift!");

        // logic shift everything to MSB
        udata <<= (sizeof(udata) * 8) - bitSize;
        // cast to signed integer
        auto data = static_cast<T>(udata);
        // arithmetic shift down to LSB for sign extension.
        data >>= (sizeof(udata) * 8) - bitSize;
        return data;
    }

    template <typename T, typename std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto DeserializeAligned(std::size_t numBytes) -> T
    {
        Align();
        AssertCapacity(numBytes);

        T result;
        // we copy the "raw" value to the MSB and then shift it down for sign extension
        std::memcpy(reinterpret_cast<unsigned char*>(&result) + sizeof(T) - numBytes, &mBuffer[mReadPos], numBytes);
        result >>= (sizeof(T) - numBytes) * 8;
        mReadPos += numBytes;
        return result;
    }

    void Align()
    {
        mUnalignedData = 0;
        mUnalignedBits = 0;
    }

    void AssertCapacity(std::size_t requiredSize)
    {
        if (mBuffer.size() - mReadPos < requiredSize)
            throw SilKit::SilKitError{"SilKit::Util::Serdes::Deserializer::AssertCapacity: end of buffer"};
    }

private:
    // ----------------------------------------
    // private members
    std::vector<uint8_t> mBuffer;
    std::size_t mReadPos = 0;
    uint64_t mUnalignedData = 0;
    std::size_t mUnalignedBits = 0;
};

} // namespace v1
} // namespace SerDes
} // namespace Util
} // namespace SilKit
