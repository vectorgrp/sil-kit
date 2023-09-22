#pragma once


#include <cstdint>


namespace VSilKit {
namespace LittleEndian {


auto GetU16(const void *data) -> uint16_t;
void PutU16(void *data, uint16_t value);


auto GetU32(const void *data) -> uint32_t;
void PutU32(void *data, uint32_t value);


auto GetU64(const void *data) -> uint64_t;
void PutU64(void *data, uint64_t value);


} // namespace LittleEndian
} // namespace VSilKit


namespace VSilKit {
namespace LittleEndian {


inline auto GetU16(const void *data) -> uint16_t
{
    const auto bytes = static_cast<const unsigned char *>(data);

    uint16_t value{0};
    for (size_t index = 0; index != 2; ++index)
    {
        value |= (static_cast<uint16_t>(bytes[index]) << (8u * index));
    }
    return value;
}


inline void PutU16(void *data, uint16_t value)
{
    const auto bytes = static_cast<unsigned char *>(data);

    for (size_t index = 0; index != 2; ++index)
    {
        bytes[index] = static_cast<unsigned char>((value >> (8u * index)) & static_cast<uint16_t>(0xFF));
    }
}


inline auto GetU32(const void *data) -> uint32_t
{
    const auto bytes = static_cast<const unsigned char *>(data);

    uint32_t value{0};
    for (size_t index = 0; index != 4; ++index)
    {
        value |= (static_cast<uint32_t>(bytes[index]) << (8u * index));
    }
    return value;
}


inline void PutU32(void *data, uint32_t value)
{
    const auto bytes = static_cast<unsigned char *>(data);

    for (size_t index = 0; index != 4; ++index)
    {
        bytes[index] = static_cast<unsigned char>((value >> (8u * index)) & static_cast<uint32_t>(0xFF));
    }
}


inline auto GetU64(const void *data) -> uint64_t
{
    const auto bytes = static_cast<const unsigned char *>(data);

    uint64_t value{0};
    for (size_t index = 0; index != 8; ++index)
    {
        value |= (static_cast<uint64_t>(bytes[index]) << (8u * index));
    }
    return value;
}


inline void PutU64(void *data, uint64_t value)
{
    const auto bytes = static_cast<unsigned char *>(data);

    for (size_t index = 0; index != 8; ++index)
    {
        bytes[index] = static_cast<unsigned char>((value >> (8u * index)) & static_cast<uint64_t>(0xFF));
    }
}


} // namespace LittleEndian
} // namespace VSilKit


namespace SilKit {
namespace Core {
namespace LittleEndian {
using namespace VSilKit::LittleEndian;
} // namespace LittleEndian
} // namespace Core
} // namespace SilKit
