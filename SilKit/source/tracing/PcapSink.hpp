// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
