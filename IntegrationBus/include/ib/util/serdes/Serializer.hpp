// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>

#include "SerializationHeader.hpp"

namespace ib {
namespace util {
namespace serdes {

inline namespace V1 {

class Serializer
{
public:
  /*! \brief The version of this serializer. */
  static constexpr uint16_t Version() { return SerDesVersion(); }

  // ----------------------------------------
  // CTOR, DTOR, Copy Operators
  Serializer() = default;
  Serializer(const Serializer& other) = default;
  Serializer(Serializer&& other) = default;
  ~Serializer() = default;

  auto operator=(const Serializer& other) -> Serializer& = default;
  auto operator=(Serializer&& other) -> Serializer& = default;

  /*! \brief cf. ISerializer::SerializeInt64()
   * Serialize method usable for:
   *  - ISerializer::SerializeInt8() through
   *  - ISerializer::SerializeInt64() through
   *  - ISerializer::SerializeUint8() through
   *  - ISerializer::SerializeUint64() through
   */
  template<typename T,
           typename std::enable_if<std::is_integral<T>::value && !std::is_same<bool, typename std::decay<T>::type>::value, int>::type = 0>
  void Serialize(T data, std::size_t bitSize)
  {
    auto byteSize = bitSize / 8;
    auto remainingBits = bitSize % 8;

    if (remainingBits) 
      SerializeUnaligned(data, bitSize);
    else
      SerializeAligned(data, byteSize);
  }

  //! \brief cf. ISerializer::SerializeBool()
  template<typename T,
           typename std::enable_if<std::is_same<bool, typename std::decay<T>::type>::value, int>::type = 0>
  void Serialize(T data)
  {
    SerializeAligned(static_cast<uint8_t>(data ? 1 : 0), 1);
  }

  /*! \brief cf. ISerializer::SerializeFloat()
   * also: ISerializer::SerializeDouble()
   *
   * NB: long double is disabled via static assert as they are treated
   * differently by MSVC and GCC.
   */
  template <typename T,
            typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
  void Serialize(T data)
  {
    static_assert(std::numeric_limits<T>::is_iec559,
                  "This compiler does not support IEEE 754 standard for floating points.");
    static_assert(!std::is_same<long double, typename std::decay<T>::type>::value,
                  "Serialization of long doubles is not supported. Cast to double at loss of precision.");
    Align();
    auto oldSize = mBuffer.size();
    mBuffer.resize(oldSize + sizeof(T));
    std::memcpy(&mBuffer[oldSize], &data, sizeof(T));
  }

  //! \brief cf. ISerializer::SerializeString()
  void Serialize(std::string string)
  {
    SerializeAligned(static_cast<uint32_t>(string.size()), 4);
    auto oldSize = mBuffer.size();
    mBuffer.resize(oldSize + string.size());
    std::copy(string.begin(), string.end(), mBuffer.begin() + oldSize);
  }

  //! \brief cf. ISerializer::SerializeBytes()
  void Serialize(const std::vector<uint8_t>& bytes)
  {
    SerializeAligned(static_cast<uint32_t>(bytes.size()), 4);
    auto oldSize = mBuffer.size();
    mBuffer.resize(oldSize + bytes.size());
    std::copy(bytes.begin(), bytes.end(), mBuffer.begin() + oldSize);
  }

  void BeginStruct()
  {
    Align();
  }
  void EndStruct() {}

  /*!  \brief cf. ISerializer::BeginArray()
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

  /*!  \brief cf. ISerializer::EndArray()
   *   NB: Dynamic array, i.e. also usable for lists.
   */
  void EndArray() {}

  void BeginOptional(bool isAvailable)
  {
    Serialize(isAvailable);
  }
  void EndOptional() {}

  void BeginUnion(int) { throw std::runtime_error("Unions are currently not supported."); }
  void EndUnion() { throw std::runtime_error("Unions are currently not supported."); }

  // --------------------------------------
  // Transport layer serialization only
  // --------------------------------------

  void Serialize(const DataMemberUpdateHeader& header)
  {
    Serialize<uint16_t>(header.GetVersion(), 16);
  }

  void Serialize(const EventMemberUpdateHeader& header)
  {
    Serialize<uint16_t>(header.GetVersion(), 16);
  }

  void Serialize(const MethodCallHeader& header)
  {
    Serialize<uint16_t>(header.GetVersion(), 16);
    Serialize<int64_t>(header.GetRequestId(), 64);
    Serialize<uint8_t>(static_cast<uint8_t>(header.GetMessageType()), 8);
    Serialize<uint8_t>(static_cast<uint8_t>(header.GetReturnCode()), 8);
  }

  /*! \brief Appends a buffer which is already in a serialized format.
   *  \param buffer The buffer which shall be appended. */
  void AppendBuffer(const std::vector<uint8_t>& buffer)
  {
    auto oldSize = mBuffer.size();
    mBuffer.resize(oldSize + buffer.size());
    std::copy(buffer.begin(), buffer.end(), mBuffer.begin() + oldSize);
  }

  void Reset()
  {
    mBuffer.resize(0);
    mUnalignedData = 0;
    mUnalignedBits = 0;
  }

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
  template<typename T, typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, int>::type = 0>
  void SerializeUnaligned(T data, std::size_t bitSize)
  {
    if (mUnalignedBits >= 8) throw std::runtime_error{"SerializedUnaligned(): current mUnalignedBits >= 8"};
    if (bitSize > sizeof(T) * 8) throw std::runtime_error{"SerializedUnaligned(): current bitSize > 8 * sizeof(T)"};

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

  template<typename T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0>
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

  template<typename T,
           typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
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

} // namespace V1
} // namespace serdes
} // namespace util
} // namespace ib
