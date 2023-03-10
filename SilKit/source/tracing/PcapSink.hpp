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

#include <mutex>
#include <fstream>
#include <memory>

#include "ITraceMessageSink.hpp"

#include "EndpointAddress.hpp"
#include "detail/NamedPipe.hpp"

namespace SilKit {
namespace Tracing {

class PcapSink : public ITraceMessageSink
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    PcapSink() = delete;
    PcapSink(const PcapSink&) = delete;
    PcapSink(Services::Logging::ILogger* logger, std::string name);
    ~PcapSink() = default;

    // ----------------------------------------
    // Public methods

    void Open(SinkType outputType, const std::string& outputPath) override;
    void Close() override;

    void Trace(SilKit::Services::TransmitDirection txRx, const Core::ServiceDescriptor& id,
               std::chrono::nanoseconds timestamp, const TraceMessage& msg) override;

    auto GetLogger() const -> Services::Logging::ILogger* override;

    auto Name() const -> const std::string& override;

private:
    // ----------------------------------------
    // Private members
    bool _headerWritten{false};
    std::ofstream _file;
    std::unique_ptr<Detail::NamedPipe> _pipe;
    std::mutex _lock;
    std::string _name;
    std::string _busName;
    std::string _outputPath;
    Services::Logging::ILogger* _logger{nullptr};
};

} // namespace Tracing
} // namespace SilKit
