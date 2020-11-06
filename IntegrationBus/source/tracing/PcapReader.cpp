// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "PcapReader.hpp"

#include "Pcap.hpp"

namespace ib {
namespace tracing {
using namespace ib::extensions;
using namespace ib::mw::logging;
//////////////////////////////////////////////////////////////////////
// PcapMessage
//////////////////////////////////////////////////////////////////////

auto PcapMessage::Timestamp() const -> std::chrono::nanoseconds
{
    return _timeStamp;
}

auto PcapMessage::GetDirection() const -> Direction
{
    return _direction;
}

auto PcapMessage::EndpointAddress() const -> ib::mw::EndpointAddress
{
    return _endpointAddress;
}

auto PcapMessage::Type() const -> TraceMessageType
{
    return TraceMessageType::EthFrame;
}

//////////////////////////////////////////////////////////////////////
// PcapReader
//////////////////////////////////////////////////////////////////////

PcapReader::PcapReader(const std::string& filePath, ILogger* logger)
    : _filePath{filePath}
    , _log{logger}
{
    Reset();
}

// Copies meta infos and resets file stream to initial seek state
PcapReader::PcapReader(PcapReader& other)
{
    _filePath = other._filePath;
    _metaInfos = other._metaInfos;
    _numMessages = other._numMessages;
    _log = other._log;
    _startTime = other._startTime;
    _endTime = other._endTime;
    Reset();
}

void PcapReader::Reset()
{
    if (!_file.is_open())
    {
        _file.open(_filePath, std::ios::binary);
    }

    if (!_file.good())
    {
        _log->Error("Cannot open file " + _filePath);
        throw std::runtime_error("Cannot open file " + _filePath);
    }
    //seek stream to first packet and cache first message
    ReadGlobalHeader();
    Seek(1);

}
void PcapReader::ReadGlobalHeader()
{
    std::array<char, sizeof(Pcap::GlobalHeader)> buf{};
    _file.seekg(0);
    _file.read(buf.data(), buf.size());
    if (!_file.good())
    {
        throw std::runtime_error("PCAP file cannot be opened: global header short read");
    }
    auto* hdr = reinterpret_cast<Pcap::GlobalHeader*>(buf.data());
    if (hdr->magic_number != Pcap::NativeMagic)
    {
        throw std::runtime_error("PCAP file cannot be opened: invalid PCAP valid magic number");
    }
    if ((hdr->version_major != Pcap::MajorVersion)
        && (hdr->version_minor != Pcap::MinorVersion)
        )
    {
        throw std::runtime_error("PCAP file cannot be opened: invalid PCAP version "
            + std::to_string(hdr->version_major) + "." + std::to_string(hdr->version_minor)
        );
    }
    _metaInfos["pcap/version"] = std::to_string(hdr->version_major) 
        + "." 
        + std::to_string(hdr->version_minor);
    _metaInfos["pcap/gmt_to_local"] = std::to_string(hdr->thiszone);
}


auto PcapReader::StartTime() const -> std::chrono::nanoseconds
{
    return _startTime;
}
auto PcapReader::EndTime() const -> std::chrono::nanoseconds
{
    throw std::runtime_error("PcapReader::EndTime(): Not Implemented");
    return _endTime;
}

auto PcapReader::NumberOfMessages() const -> uint64_t 
{
    return _numMessages;
}

bool PcapReader::Seek(size_t messageNumber)
{
    //seek number of messages relative to current position
    for (auto i = 0u; i < messageNumber; i++)
    {
        //extract PCAP packet from stream

        std::array<char, sizeof(Pcap::PacketHeader)> buf{};
        _file.read(buf.data(), buf.size());
        if (!_file.good())
        {
            _log->Warn("PCAP file: " + _filePath +": short read on packet header.");
            return false;
        }
        auto msg = std::make_shared<PcapMessage>();
        auto* hdr = reinterpret_cast<Pcap::PacketHeader*>(buf.data());
        std::chrono::nanoseconds timeStamp{((uint64_t)hdr->ts_sec * 1000000000u)
            + ((uint64_t)hdr->ts_usec * 1000u)};

        std::vector<uint8_t> msgBuf{};
        msgBuf.resize(hdr->incl_len);
        _file.read(reinterpret_cast<char*>(msgBuf.data()), hdr->incl_len);
        if (!_file.good())
        {
            _log->Warn("PCAP file: " + _filePath 
                + ": Cannot read packet at offset " + std::to_string(_file.tellg()));
            return false;
        }
        msg->SetRawFrame(msgBuf);
        msg->_timeStamp = timeStamp;

        _currentMessage = std::move(msg);
        _numMessages++; //HACK we can't know the number of messages without seeking through the whole file
    }
    return true;
}

std::shared_ptr<IReplayMessage> PcapReader::Read()
{
    //return cached value
    return _currentMessage;
};

auto PcapReader::GetMetaInfos() const -> const std::map<std::string, std::string>&
{
    return _metaInfos;
}


} // namespace tracing
} // namespace ib
