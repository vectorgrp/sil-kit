// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "PcapSink.hpp"

#include <string>
#include <ctime>
#include <sstream>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/extensions/TraceMessage.hpp"
#include "ib/extensions/string_utils.hpp"

#include "Pcap.hpp"
#include "detail/NamedPipe.hpp"

namespace ib {
namespace tracing {

namespace {
Pcap::GlobalHeader g_pcapGlobalHeader{};
} //anonymous namespace

PcapSink::PcapSink(mw::logging::ILogger* logger, std::string name)
    : _name{std::move(name)}
    , _logger{logger}
{

}

void PcapSink::Open(extensions::SinkType outputType, const std::string& outputPath)
{
    if (outputPath.empty())
    {
        throw std::runtime_error("PcapSink::Open: outputPath must not be empty!");
    }

    switch (outputType)
    {
    case ib::extensions::SinkType::PcapFile:
        if (_file.is_open())
        {
            _file.close();
        }
        _file.open(outputPath, std::ios::out | std::ios::binary | std::ios::app);
        _file.write(reinterpret_cast<char*>(&g_pcapGlobalHeader),
                sizeof(g_pcapGlobalHeader));
        break;

    case ib::extensions::SinkType::PcapNamedPipe:
        _pipe = detail::NamedPipe::Create(outputPath);
        _headerWritten = false;
        _outputPath = outputPath;
        break;
    default:
        throw std::runtime_error("PcapSink::Open: specified SinkType not implemented");
    }
}

auto PcapSink::GetLogger() const -> mw::logging::ILogger*
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
        _pipe.reset();
    }
}

void PcapSink::Trace(extensions::Direction /*unused*/,
        const mw::EndpointAddress& /* unused endpoint address */,
        std::chrono::nanoseconds timestamp,
        const extensions::TraceMessage& traceMessage)
{
    if (traceMessage.Type() != extensions::TraceMessageType::EthFrame)
    {
        std::stringstream ss;
        ss << "Error: unsupported message type: " << traceMessage;
        throw std::runtime_error(ss.str());
    }
    const auto& message = traceMessage.Get<sim::eth::EthFrame>();

    std::unique_lock<decltype(_lock)> lock;
    

    const auto tosec = 1000'000ull;
    const auto usec = std::chrono::duration_cast<std::chrono::microseconds>(timestamp);

    Pcap::PacketHeader pcapPacketHeader;
    pcapPacketHeader.orig_len = static_cast<uint32_t>(message.GetFrameSize());
    pcapPacketHeader.incl_len = pcapPacketHeader.orig_len;
    pcapPacketHeader.ts_sec = usec.count() / tosec;
    pcapPacketHeader.ts_usec = usec.count() % tosec;

    bool ok = true;
    if (_file.is_open())
    {
        _file.write(reinterpret_cast<char*>(&pcapPacketHeader),
                sizeof(pcapPacketHeader));
        _file.write(reinterpret_cast<const char*>(&message.RawFrame().at(0)),
                message.GetFrameSize());
        ok = _file.good();
    }

    if (_pipe)
    {
        if (!_headerWritten)
        {
            _logger->Info("Sink {}: Waiting for a reader to connect to PCAP pipe {} ... ",
                _name, _outputPath);

            ok &= _pipe->Write(reinterpret_cast<char*>(&g_pcapGlobalHeader),
                sizeof(g_pcapGlobalHeader));
            _logger->Debug("Sink {}: PCAP pipe: {} is connected successfully",
                _name, _outputPath);

            _headerWritten = true;
        }

        ok &=_pipe->Write(reinterpret_cast<char*>(&pcapPacketHeader), sizeof(pcapPacketHeader));
        ok &=_pipe->Write(reinterpret_cast<const char*>(&message.RawFrame().at(0)), message.GetFrameSize());
    }
}


} // tracing
} // ib
