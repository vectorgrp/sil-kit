// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "SerializationHeader.hpp"

namespace ib {
namespace util {
namespace serdes {

inline namespace V1 {

class Deserializer
{
  public:
    /*! \brief The version of this deserializer. Corresponds to the namespace name. */
    static constexpr uint16_t Version() { return SerDesVersion(); }

    // ----------------------------------------
    // CTOR, DTOR, Copy Operators
    Deserializer() = default;
    Deserializer(std::vector<uint8_t> buffer) : mBuffer(std::move(buffer)) {}
    Deserializer(const Deserializer& other) = default;
    Deserializer(Deserializer&& other) = default;
    ~Deserializer() = default;

    auto operator=(const Deserializer& other) -> Deserializer& = default;
    auto operator=(Deserializer&& other) -> Deserializer& = default;

    /*! \brief cf. IDeserializer::DeserializeInt64()
     * Deserialize method usable for:
     *  - IDeserializer::DeserializeInt8() through
     *  - IDeserializer::DeserializeInt64() through
     *  - IDeserializer::DeserializeUint8() through
     *  - IDeserializer::DeserializeUint64() through
     */
    template <typename T, typename std::enable_if<std::is_integral<T>::value &&
                                                      !std::is_same<bool, typename std::decay<T>::type>::value,
                                                  int>::type = 0>
    auto Deserialize(std::size_t bitSize) -> T
    {
        auto byteSize = bitSize / 8;
        auto remainingBits = bitSize % 8;

        if (remainingBits)
            return DeserializeUnaligned<T>(bitSize);
        else
            return DeserializeAligned<T>(byteSize);
    }

    //! \brief cf. IDeserializer::DeserializeBool()
    template <typename T,
              typename std::enable_if<std::is_same<bool, typename std::decay<T>::type>::value, int>::type = 0>
    auto Deserialize() -> T
    {
        return DeserializeAligned<uint8_t>(1) ? true : false;
    }

    /*! \brief cf. IDeserializer::DeserializeFloat()
     * also: IDeserializer::DeserializeDouble()
     */
    template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    auto Deserialize() -> T
    {
        static_assert(std::numeric_limits<T>::is_iec559,
                      "This compiler does not support IEEE 754 standard for floating points.");
        static_assert(!std::is_same<long double, typename std::decay<T>::type>::value,
                      "Serialization of long doubles is not supported. Cast to double at loss of precision.");
        Align();
        AssertCapacity(sizeof(T));

        T result;
        std::memcpy(&result, &mBuffer[mReadPos], sizeof(T));
        mReadPos += sizeof(T);
        return result;
    }

    //! \brief cf. IDeserializer::DeserializeString()
    template <typename T, typename std::enable_if<std::is_same<std::string, T>::value, int>::type = 0>
    auto Deserialize() -> T
    {
        auto size = DeserializeAligned<uint32_t>(4);
        AssertCapacity(size);
        std::string result{mBuffer.begin() + mReadPos, mBuffer.begin() + mReadPos + size};
        mReadPos += size;
        return result;
    }

    //! \brief cf. IDeserializer::DeserializeBytes()
    template <typename T, typename std::enable_if<std::is_same<std::vector<uint8_t>, T>::value, int>::type = 0>
    auto Deserialize() -> T
    {
        auto size = DeserializeAligned<uint32_t>(4);
        AssertCapacity(size);
        std::vector<uint8_t> result{mBuffer.begin() + mReadPos, mBuffer.begin() + mReadPos + size};
        mReadPos += size;
        return result;
    }

    void BeginStruct() { Align(); }
    void EndStruct() {}

    /*! \brief cf. IDeserializer::BeginArray
     *  NB: Dynamic arrays, i.e. also usable for lists.
     */
    auto BeginArray() -> std::size_t { return DeserializeAligned<uint32_t>(sizeof(uint32_t)); }

    /*! \brief cf. IDeserializer::EndArray
     *  NB: Dynamic arrays, i.e. also usable for lists.
     */
    void EndArray() {}

    auto BeginOptional() -> bool { return Deserialize<bool>(); }
    void EndOptional() {}

    auto BeginUnion() -> int { throw std::runtime_error("Unions are currently not supported."); }
    void EndUnion() { throw std::runtime_error("Unions are currently not supported."); }

    // --------------------------------------
    // Transport layer deserialization only
    // --------------------------------------

    template <typename T, typename std::enable_if<
                              std::is_same<DataMemberUpdateHeader, typename std::decay<T>::type>::value, int>::type = 0>
    auto Deserialize() -> T
    {
        const auto version = Deserialize<uint16_t>(16);
        CheckVersionInformation(version);
        return DataMemberUpdateHeader();
    }

    template <typename T,
              typename std::enable_if<std::is_same<EventMemberUpdateHeader, typename std::decay<T>::type>::value,
                                      int>::type = 0>
    auto Deserialize() -> T
    {
        const auto version = Deserialize<uint16_t>(16);
        CheckVersionInformation(version);
        return EventMemberUpdateHeader();
    }

    template <typename T, typename std::enable_if<std::is_same<MethodCallHeader, typename std::decay<T>::type>::value,
                                                  int>::type = 0>
    auto Deserialize() -> T
    {
        const auto version = Deserialize<uint16_t>(16);
        CheckVersionInformation(version);

        const auto requestId = Deserialize<int64_t>(64);
        const auto messageType = static_cast<MethodCallMessageType>(Deserialize<uint8_t>(8));
        const auto returnCode = static_cast<MethodCallReturnCode>(Deserialize<uint8_t>(8));
        return MethodCallHeader(requestId, messageType, returnCode);
    }

    auto GetRemainingBuffer() -> std::vector<uint8_t>
    {
        return std::vector<uint8_t>(mBuffer.begin() + mReadPos, mBuffer.end());
    }

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
    template <typename T,
              typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, int>::type = 0>
    auto DeserializeUnaligned(std::size_t bitSize) -> T
    {
        if (mUnalignedBits >= 8)
            throw std::runtime_error{"DeserializedUnaligned(): current mUnalignedBits >= 8"};
        if (bitSize > sizeof(T) * 8)
            throw std::runtime_error{"DeserializedUnaligned(): current bitSize > 8 * sizeof(T)"};

        uint64_t    readData = 0;
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
    template <typename T,
              typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0>
    auto DeserializeUnaligned(std::size_t bitSize) -> T
    {
        using uint_t = typename std::make_unsigned<T>::type;
        auto udata = DeserializeUnaligned<uint_t>(bitSize);

        // Make sure that >> on signed integers is an arithmetic shift. (right shift on signed integers is
        // implementation defined)
        static_assert(static_cast<int64_t>((std::numeric_limits<uint64_t>::max)()) >> 63 ==
                          static_cast<int64_t>((std::numeric_limits<uint64_t>::max)()),
                      "Right shift on signed integers must be an arithmetic shift!");

        // logic shift everything to MSB
        udata <<= (sizeof(udata) * 8) - bitSize;
        // cast to signed integer
        auto data = static_cast<T>(udata);
        // arithmetic shift down to LSB for sign extension.
        data >>= (sizeof(udata) * 8) - bitSize;
        return data;
    }

    template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
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
            throw std::runtime_error{"end of buffer"}; // FIXME: introduce dedicated exception?
    }

    void CheckVersionInformation(uint16_t version)
    {
        if (version != Version())
        {
            std::string errorMsg{"Received data with unsupported serialization version="};
            errorMsg += std::to_string(version);
            throw std::runtime_error{errorMsg};
        }
    }

  private:
    // ----------------------------------------
    // private members
    std::vector<uint8_t> mBuffer;
    std::size_t          mReadPos = 0;
    uint64_t             mUnalignedData = 0;
    std::size_t          mUnalignedBits = 0;
};

} // namespace V1
} // namespace serdes
} // namespace util 
} // namespace ib
