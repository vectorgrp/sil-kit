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

#pragma once

#include <istream>
#include <fstream>

#include "IReplay.hpp"

namespace SilKit {
namespace Tracing {

class PcapReader : public SilKit::IReplayChannelReader
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
