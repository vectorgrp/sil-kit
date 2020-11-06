// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <fstream>

#include "ib/extensions/IReplay.hpp"
#include "ib/sim/eth/EthDatatypes.hpp"
#include "ib/mw/EndpointAddress.hpp"

namespace ib {
namespace tracing {

class PcapMessage
    : public ib::extensions::IReplayMessage
    , public ib::sim::eth::EthFrame
{
public:

    auto Timestamp() const -> std::chrono::nanoseconds override;

    auto GetDirection() const -> ib::extensions::Direction override;

    auto EndpointAddress() const -> ib::mw::EndpointAddress override;

    auto Type() const -> ib::extensions::TraceMessageType override;
    
private:
    std::chrono::nanoseconds _timeStamp{0};
    ib::extensions::Direction _direction{ib::extensions::Direction::Send};
    ib::mw::EndpointAddress  _endpointAddress{};

    friend class PcapReader;
};


class PcapReader
    : public ib::extensions::IReplayChannelReader
{
public:

    PcapReader(const std::string& filePath, ib::mw::logging::ILogger* logger);
    PcapReader(PcapReader& other);

    auto StartTime() const -> std::chrono::nanoseconds;
    auto EndTime() const -> std::chrono::nanoseconds;
    auto NumberOfMessages() const -> uint64_t ;

    // Interface IReplayChannelReader
    bool Seek(size_t messageNumber) override;
    std::shared_ptr<ib::extensions::IReplayMessage> Read() override;

    // 
    auto GetMetaInfos() const -> const std::map<std::string, std::string>&;
private:
    //Methods
    void Reset();
    void ReadGlobalHeader();
private:
    std::string _filePath;
    std::ifstream _file;
    std::map<std::string, std::string> _metaInfos;
    std::shared_ptr<PcapMessage> _currentMessage;
    uint64_t _numMessages{0};
    ib::mw::logging::ILogger* _log{nullptr};
    std::chrono::nanoseconds _startTime{0};
    std::chrono::nanoseconds _endTime{0};
};


} // namespace tracing
} // namespace ib
