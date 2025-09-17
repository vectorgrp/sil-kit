// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <istream>
#include <fstream>

#include "IReplay.hpp"

namespace SilKit {
namespace Tracing {

class PcapReader final : public SilKit::IReplayChannelReader
{
public:
    // Constructors
    PcapReader(const std::string& filePath, SilKit::Services::Logging::ILogger* logger);
    //This CTor is for testing purposes only:
    PcapReader(std::istream* stream, SilKit::Services::Logging::ILogger* logger);
    PcapReader(PcapReader& other);

public:
    // Methods
    auto StartTime() const -> std::chrono::nanoseconds;
    auto EndTime() const -> std::chrono::nanoseconds;
    auto NumberOfMessages() const -> uint64_t;

    // Interface IReplayChannelReader
    bool Seek(size_t messageNumber) override;
    auto Read() -> std::shared_ptr<SilKit::IReplayMessage> override;

    auto GetMetaInfos() const -> const std::map<std::string, std::string>&;

private:
    //Methods
    void Reset();
    void ReadGlobalHeader();

private:
    std::string _filePath;
    std::ifstream _file;
    std::istream* _stream{nullptr};
    std::map<std::string, std::string> _metaInfos;
    std::shared_ptr<IReplayMessage> _currentMessage;
    uint64_t _numMessages{0};
    SilKit::Services::Logging::ILogger* _log{nullptr};
    std::chrono::nanoseconds _startTime{0};
    std::chrono::nanoseconds _endTime{0};
};

} // namespace Tracing
} // namespace SilKit
