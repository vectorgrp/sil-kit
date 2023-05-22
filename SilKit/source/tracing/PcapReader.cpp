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
#include "PcapReader.hpp"

#include "silkit/services/ethernet/EthernetDatatypes.hpp"

#include "WireEthernetMessages.hpp"
#include "Pcap.hpp"
#include "Assert.hpp"

namespace SilKit {
namespace Tracing {

using namespace SilKit::Services::Logging;

//////////////////////////////////////////////////////////////////////
// PcapMessage -- internal only
//////////////////////////////////////////////////////////////////////

class PcapMessage
    : public SilKit::IReplayMessage
    , public SilKit::Services::Ethernet::WireEthernetFrame
{
public:
    auto Timestamp() const -> std::chrono::nanoseconds override;
    void SetTimestamp(std::chrono::nanoseconds timeStamp);
    auto GetDirection() const -> SilKit::Services::TransmitDirection override;
    auto ServiceDescriptorStr() const -> std::string override;
    auto EndpointAddress() const -> SilKit::Core::EndpointAddress override;
    auto Type() const -> SilKit::TraceMessageType override;

private:
    std::chrono::nanoseconds _timeStamp{0};
    SilKit::Services::TransmitDirection _direction{SilKit::Services::TransmitDirection::TX};
    std::string _serviceDescriptorStr;
};

void PcapMessage::SetTimestamp(std::chrono::nanoseconds timeStamp)
{
    _timeStamp = timeStamp;
}

auto PcapMessage::Timestamp() const -> std::chrono::nanoseconds
{
    return _timeStamp;
}

auto PcapMessage::GetDirection() const -> SilKit::Services::TransmitDirection
{
    return _direction;
}

auto PcapMessage::ServiceDescriptorStr() const -> std::string
{
    return _serviceDescriptorStr;
}

auto PcapMessage::EndpointAddress() const -> SilKit::Core::EndpointAddress
{
    return {};
}

auto PcapMessage::Type() const -> TraceMessageType
{
    return TraceMessageType::EthernetFrame;
}

//////////////////////////////////////////////////////////////////////
// PcapReader
//////////////////////////////////////////////////////////////////////

PcapReader::PcapReader(std::istream* stream, SilKit::Services::Logging::ILogger* logger)
    : _stream{stream}
    , _log{logger}
{
    Reset();
}
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
    _stream = other._stream;
    Reset();
}

void PcapReader::Reset()
{
    if (_filePath.empty() && _stream == nullptr)
    {
        _log->Error("PcapReader::Reset(): no input file or stream pointer given!");
        throw SilKitError("PcapReader::Reset(): no input file or stream pointer given!");
    }

    if (!_filePath.empty())
    {
        if (!_file.is_open())
        {
            _file.open(_filePath, std::ios::binary | std::ios::in);
            _stream = &_file;
        }
        if (!_file.good())
        {
            _log->Error("Cannot open file " + _filePath);
            throw SilKitError("Cannot open file " + _filePath);
        }
    }

    SILKIT_ASSERT(_stream != nullptr);

    //seek stream to first packet and cache first message
    ReadGlobalHeader();
    Seek(1);
}

void PcapReader::ReadGlobalHeader()
{
    std::array<char, sizeof(Pcap::GlobalHeader)> buf{};
    _stream->seekg(0);
    _stream->read(buf.data(), buf.size());
    if (!_stream->good())
    {
        throw SilKitError("PCAP file cannot be opened: global header short read");
    }
    auto* hdr = reinterpret_cast<Pcap::GlobalHeader*>(buf.data());
    if (hdr->magic_number != Pcap::NativeMagic)
    {
        throw SilKitError("PCAP file cannot be opened: invalid PCAP valid magic number");
    }
    if ((hdr->version_major != Pcap::MajorVersion) && (hdr->version_minor != Pcap::MinorVersion))
    {
        throw SilKitError("PCAP file cannot be opened: invalid PCAP version " + std::to_string(hdr->version_major) + "."
                          + std::to_string(hdr->version_minor));
    }
    _metaInfos["pcap/version"] = std::to_string(hdr->version_major) + "." + std::to_string(hdr->version_minor);
    _metaInfos["pcap/gmt_to_local"] = std::to_string(hdr->thiszone);
}

auto PcapReader::StartTime() const -> std::chrono::nanoseconds
{
    return _startTime;
}

auto PcapReader::EndTime() const -> std::chrono::nanoseconds
{
    throw SilKitError("PcapReader::EndTime(): Not Implemented");
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
        _stream->read(buf.data(), buf.size());
        if (!_stream->good())
        {
            _log->Warn("PCAP file: " + _filePath + ": short read on packet header.");
            return false;
        }
        auto msg = std::make_shared<PcapMessage>();
        auto* hdr = reinterpret_cast<Pcap::PacketHeader*>(buf.data());
        std::chrono::nanoseconds timeStamp{((uint64_t)hdr->ts_sec * 1000000000u) + ((uint64_t)hdr->ts_usec * 1000u)};

        std::vector<uint8_t> msgBuf{};
        msgBuf.resize(hdr->incl_len);
        _stream->read(reinterpret_cast<char*>(msgBuf.data()), hdr->incl_len);
        if (!_stream->good())
        {
            _log->Warn("PCAP file: " + _filePath + ": Cannot read packet at offset "
                       + std::to_string(_stream->tellg()));
            return false;
        }
        msg->raw = std::move(msgBuf);
        msg->SetTimestamp(timeStamp);

        _currentMessage = std::move(msg);
        //NB we can't know the number of messages without seeking through the whole file,
        //      which we're not going to do for performance reasons.
        _numMessages++;
    }
    return true;
}

std::shared_ptr<IReplayMessage> PcapReader::Read()
{
    //return cached value
    return _currentMessage;
}

auto PcapReader::GetMetaInfos() const -> const std::map<std::string, std::string>&
{
    return _metaInfos;
}

} // namespace Tracing
} // namespace SilKit
