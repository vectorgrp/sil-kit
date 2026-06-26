// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "PcapReplay.hpp"

#include <memory>

#include "silkit/services/ethernet/EthernetDatatypes.hpp"

#include "IReplay.hpp"
#include "PcapReader.hpp"

namespace {

using namespace SilKit::Services::Logging;
using namespace SilKit::Tracing;

//////////////////////////////////////////////////////////////////////
// IReplay: Boilerplate to satisfy interfaces follows.
//          Actual implementation is in PcapReader.
//////////////////////////////////////////////////////////////////////

class ReplayPcapChannel : public SilKit::IReplayChannel
{
public:
    ReplayPcapChannel(const std::string& filePath, ILogger* logger)
        : _reader{filePath, logger}
    {
    }

    // Interface IReplayChannel
    auto Type() const -> SilKit::TraceMessageType override
    {
        //our version supports only ethernet
        return SilKit::TraceMessageType::EthernetFrame;
    }

    auto StartTime() const -> std::chrono::nanoseconds override
    {
        return _reader.StartTime();
    }
    auto EndTime() const -> std::chrono::nanoseconds override
    {
        return _reader.EndTime();
    }
    auto NumberOfMessages() const -> uint64_t override
    {
        return _reader.NumberOfMessages();
    }
    auto Name() const -> const std::string& override
    {
        // Pcap has no concept of channel, return hard coded.
        return _channelName;
    }
    auto GetMetaInfos() const -> const std::map<std::string, std::string>& override
    {
        return _reader.GetMetaInfos();
    }
    auto GetReader() -> std::shared_ptr<SilKit::IReplayChannelReader> override
    {
        // return a copy, which allows caching the internal data structures
        // for seeking. It is reset to start reading at the beginning.
        return std::make_shared<PcapReader>(_reader);
    }

private:
    PcapReader _reader;
    const std::string _channelName{"PcapChannel0"};
};

class ReplayPcapFile : public SilKit::IReplayFile
{
public:
    ReplayPcapFile(std::string filePath, SilKit::Services::Logging::ILogger* logger)
        : _filePath{std::move(filePath)}
    {
        auto channel = std::make_shared<ReplayPcapChannel>(_filePath, logger);
        _channels.emplace_back(std::move(channel));
    }

    auto FilePath() const -> const std::string& override
    {
        return _filePath;
    }
    auto SilKitConfig() const -> std::string override
    {
        return {};
    }

    FileType Type() const override
    {
        return IReplayFile::FileType::PcapFile;
    }

    std::vector<std::shared_ptr<SilKit::IReplayChannel>>::iterator begin() override
    {
        return _channels.begin();
    }
    std::vector<std::shared_ptr<SilKit::IReplayChannel>>::iterator end() override
    {
        return _channels.end();
    }

private:
    std::string _filePath;
    std::vector<std::shared_ptr<SilKit::IReplayChannel>> _channels;
};

} // namespace

namespace SilKit {
namespace Tracing {

auto PcapReplay::OpenFile(const SilKit::Config::ParticipantConfiguration&, const std::string& filePath,
                          SilKit::Services::Logging::ILogger* logger) -> std::shared_ptr<IReplayFile>
{
    return std::make_shared<ReplayPcapFile>(filePath, logger);
}

} // namespace Tracing
} // namespace SilKit
