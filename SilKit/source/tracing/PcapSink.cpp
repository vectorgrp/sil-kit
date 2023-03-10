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

#include "PcapSink.hpp"

#include <string>
#include <ctime>
#include <sstream>

#include "TraceMessage.hpp"
#include "string_utils.hpp"

#include "Pcap.hpp"
#include "detail/NamedPipe.hpp"

#include "ILogger.hpp"

namespace SilKit {
namespace Tracing {

using namespace SilKit::Services::Logging;

namespace {
constexpr Pcap::GlobalHeader g_pcapGlobalHeader{};
} // namespace

PcapSink::PcapSink(Services::Logging::ILogger* logger, std::string name)
    : _name{std::move(name)}
    , _logger{logger}
{
}

void PcapSink::Open(SinkType outputType, const std::string& outputPath)
{
    if (outputPath.empty())
    {
        throw SilKitError("PcapSink::Open: outputPath must not be empty!");
    }

    switch (outputType)
    {
    case SilKit::SinkType::PcapFile:
        if (_file.is_open())
        {
            _file.close();
        }
        _file.open(outputPath, std::ios::out | std::ios::binary);
        _file.write(reinterpret_cast<const char*>(&g_pcapGlobalHeader), sizeof(g_pcapGlobalHeader));
        break;

    case SilKit::SinkType::PcapNamedPipe:
        _pipe = Detail::NamedPipe::Create(outputPath);
        _headerWritten = false;
        _outputPath = outputPath;
        break;
    default: throw SilKitError("PcapSink::Open: specified SinkType not implemented");
    }
}

auto PcapSink::GetLogger() const -> Services::Logging::ILogger*
{
    return _logger;
}

auto PcapSink::Name() const -> const std::string&
{
    return _name;
}

void PcapSink::Close()
{
    if (_file)
    {
        _file.flush();
        _file.close();
    }

    if (_pipe)
    {
        try
        {
            _pipe->Close();
        }
        catch (const SilKitError& err)
        {
            Services::Logging::Warn(_logger, "Failed to close PCAP sink: {}", err.what());
        }
        _pipe.reset();
    }
}

void PcapSink::Trace(SilKit::Services::TransmitDirection /*unused*/,
                     const Core::ServiceDescriptor& /* unused endpoint address */, std::chrono::nanoseconds timestamp,
                     const TraceMessage& traceMessage)
{
    if (traceMessage.Type() != TraceMessageType::EthernetFrame)
    {
        std::stringstream ss;
        ss << "Error: unsupported message type: " << traceMessage;
        throw SilKitError(ss.str());
    }
    const auto& message = traceMessage.Get<Services::Ethernet::EthernetFrame>();

    std::unique_lock<decltype(_lock)> lock;

    const auto tosec = 1000'000ull;
    const auto usec = std::chrono::duration_cast<std::chrono::microseconds>(timestamp);

    Pcap::PacketHeader pcapPacketHeader;
    pcapPacketHeader.orig_len = static_cast<uint32_t>(message.raw.size());
    pcapPacketHeader.incl_len = pcapPacketHeader.orig_len;
    pcapPacketHeader.ts_sec = static_cast<uint32_t>(usec.count() / tosec);
    pcapPacketHeader.ts_usec = static_cast<uint32_t>(usec.count() % tosec);

    bool ok = true;
    if (_file.is_open())
    {
        _file.write(reinterpret_cast<const char*>(&pcapPacketHeader), sizeof(pcapPacketHeader));
        _file.write(reinterpret_cast<const char*>(&message.raw.at(0)), message.raw.size());
        ok &= _file.good();
    }

    if (_pipe)
    {
        if (!_headerWritten)
        {
            Services::Logging::Info(_logger, "Sink {}: Waiting for a reader to connect to PCAP pipe {} ... ", _name,
                                    _outputPath);

            ok &= _pipe->Write(reinterpret_cast<const char*>(&g_pcapGlobalHeader), sizeof(g_pcapGlobalHeader));
            Services::Logging::Debug(_logger, "Sink {}: PCAP pipe: {} is connected successfully", _name, _outputPath);

            _headerWritten = true;
        }

        ok &= _pipe->Write(reinterpret_cast<const char*>(&pcapPacketHeader), sizeof(pcapPacketHeader));
        ok &= _pipe->Write(reinterpret_cast<const char*>(&message.raw.at(0)), message.raw.size());
    }

    if (!ok)
    {
        throw SilKitError("Failed to write trace message to PCAP sink");
    }
}

} // namespace Tracing
} // namespace SilKit
