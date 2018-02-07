// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "EthFrame.hpp"

#include <cassert>

namespace ib {
namespace sim {
namespace eth {

namespace {

constexpr uint16_t TPID = 0x8100;

template <size_t offset, size_t size>
struct FrameElement
{
    constexpr static size_t Start = offset;
    constexpr static size_t Size = size;
    constexpr static size_t End = offset + size;

    template <size_t next_size>
    using NextElement = FrameElement<End, next_size>;
};

using DestinationMac = FrameElement<0, 6>;
using SourceMac      = DestinationMac::NextElement<6>;
using VlanTag        = SourceMac::NextElement<4>;
using EtherType      = VlanTag::NextElement<2>;


enum Constants
{
    CrcSize        = 4,
    MinFrameSize   = 68
};

uint16_t reverse_byte_order(uint16_t value)
{
    return (value << 8) | (value >> 8);
}

template <typename T>
using EnableIfIntegral = typename std::enable_if<std::is_integral<T>::value>::type;


template <typename T, typename = EnableIfIntegral<T>>
inline T get_value(const std::vector<uint8_t>& storage, size_t offset)
{
    assert(storage.size() >= offset + sizeof(T));
    return reverse_byte_order(*reinterpret_cast<const T*>(storage.data() + offset));
}

template <typename T, typename = EnableIfIntegral<T>>
inline void set_value(T t, std::vector<uint8_t>& storage, size_t offset)
{
    assert(storage.size() >= offset + sizeof(T));
    T* tPtr = reinterpret_cast<T*>(storage.data() + offset);
    *tPtr = reverse_byte_order(t);
}

template <size_t N>
inline void get_value(std::array<uint8_t, N>& outArray, const std::vector<uint8_t>& storage, size_t offset)
{
    assert(storage.size() >= offset + N);

    std::copy(storage.begin() + offset, storage.begin() + offset + N, outArray.begin());
}
template <size_t N>
inline void set_value(const std::array<uint8_t, N>& array, std::vector<uint8_t>& storage, size_t offset)
{
    assert(storage.size() >= offset + N);

    std::copy(array.begin(), array.end(), storage.begin() + offset);
}

inline void get_value(EthTagControlInformation& tci, const std::vector<uint8_t>& storage, size_t offset)
{
    uint16_t rawTci = get_value<uint16_t>(storage, offset);
    
    tci.pcp = (rawTci >> 13) & 0x0003;
    tci.dei = (rawTci >> 12) & 0x0001;
    tci.vid = rawTci & 0x0fff;
}
inline void set_value(const EthTagControlInformation& tci, std::vector<uint8_t>& storage, size_t offset)
{
    uint16_t rawTci = (tci.pcp & 0x03) << 13
                    | (tci.dei & 0x01) << 12
                    | (tci.vid & 0x0fff);

    set_value(rawTci, storage, offset);
}

} // anonymous namespace for local helpers


EthFrame::EthFrame(const std::vector<uint8_t>& rawFrame)
: _rawFrame(rawFrame)
{
}

EthFrame::EthFrame(std::vector<uint8_t>&& rawFrame)
: _rawFrame(std::move(rawFrame))
{
}

EthFrame::EthFrame(const uint8_t* rawFrame, size_t size_t)
: _rawFrame(rawFrame, rawFrame + size_t)
{
}

auto EthFrame::GetDestinationMac() const -> EthMac
{
    if (_rawFrame.empty())
    {
        throw std::exception();
        return EthMac{};
    }

    EthMac dest;
    get_value(dest, _rawFrame, DestinationMac::Start);
    return dest;
}

void EthFrame::SetDestinationMac(const EthMac& dest)
{
    if (_rawFrame.empty())
    {
        _rawFrame.resize(MinFrameSize);
    }

    set_value(dest, _rawFrame, DestinationMac::Start);
}

auto EthFrame::GetSourceMac() const -> EthMac
{
    if (_rawFrame.empty())
    {
        throw std::exception();
        return EthMac{};
    }

    EthMac source;
    get_value(source, _rawFrame, SourceMac::Start);
    return source;
}

void EthFrame::SetSourceMac(const EthMac& source)
{
    if (_rawFrame.empty())
    {
        _rawFrame.resize(MinFrameSize);
    }

    set_value(source, _rawFrame, SourceMac::Start);
}

auto EthFrame::GetVlanTag() const -> EthTagControlInformation
{
    if (_rawFrame.empty())
    {
        throw std::exception();
        return EthTagControlInformation();
    }

    EthTagControlInformation tci;
    get_value(tci, _rawFrame, VlanTag::Start + 2);
    return tci;
}

void EthFrame::SetVlanTag(const EthTagControlInformation& tci)
{
    if (_rawFrame.empty())
    {
        _rawFrame.resize(MinFrameSize);
    }

    set_value(TPID, _rawFrame, VlanTag::Start);
    set_value(tci, _rawFrame, VlanTag::Start + 2);
}

auto EthFrame::GetEtherType() const -> uint16_t
{
    if (_rawFrame.empty())
    {
        throw std::exception();
        return 0;
    }

    return get_value<uint16_t>(_rawFrame, EtherType::Start);
}

void EthFrame::SetEtherType(uint16_t etherType)
{
    if (_rawFrame.empty())
    {
        _rawFrame.resize(MinFrameSize);
    }

    set_value(etherType, _rawFrame, EtherType::Start);
}

auto EthFrame::GetFrameSize() const -> size_t
{
    return _rawFrame.size();
}

auto EthFrame::GetHeaderSize() const -> size_t
{
    return EtherType::End;
}

auto EthFrame::GetPayloadSize() const -> size_t
{
    // the payload is the entire frame without header and checksum
    return GetFrameSize() - GetHeaderSize() - CrcSize;
}

auto EthFrame::GetPayload() -> util::vector_view<uint8_t>
{
    auto view = ::ib::util::make_vector_view(_rawFrame);
    view.trim_front(GetHeaderSize());
    view.trim_back(CrcSize);
    return view;
}

auto EthFrame::GetPayload() const -> util::vector_view<const uint8_t>
{
    auto view = ::ib::util::make_vector_view(_rawFrame);
    view.trim_front(GetHeaderSize());
    view.trim_back(CrcSize);
    return view;
}

void EthFrame::SetPayload(const std::vector<uint8_t>& payload)
{
    auto payloadOffset = GetHeaderSize();
    _rawFrame.resize(payloadOffset + payload.size() + CrcSize);

    std::copy(payload.begin(), payload.end(), _rawFrame.begin() + payloadOffset);

    // FIXME: calculate a checksum?
}

void EthFrame::SetPayload(const uint8_t* payload, size_t size)
{
    auto payloadOffset = GetHeaderSize();
    _rawFrame.resize(payloadOffset + size + CrcSize);

    std::copy(payload, payload + size, _rawFrame.begin() + payloadOffset);
}

auto EthFrame::RawFrame() const -> const std::vector<uint8_t>&
{
    return _rawFrame;
}

} // namespace eth
} // namespace sim
} // namespace ib