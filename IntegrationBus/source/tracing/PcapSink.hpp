// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <mutex>
#include <fstream>
#include <memory>

#include "EthFrame.hpp"
#include "ib/mw/EndpointAddress.hpp"
#include "detail/NamedPipe.hpp"
#include "Tracing.hpp"

namespace ib {
namespace tracing {


class PcapSink 
    : public ib::tracing::ITraceMessageSink
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    PcapSink() = delete;
    PcapSink(const PcapSink&) = delete;
    PcapSink(mw::logging::ILogger* logger, std::string name);
    PcapSink(PcapSink&&) = default;
    ~PcapSink() = default;

    // ----------------------------------------
    // Public methods

    void Open(
        tracing::SinkType outputType,
        const std::string& outputPath
    ) override;
    void Close() override;

    void Trace(tracing::Direction txRx,
        const mw::EndpointAddress& id,
        std::chrono::nanoseconds timestamp,
        const TraceMessage& msg
    ) override;
   
    auto GetLogger() const -> mw::logging::ILogger* override
    {
        return _logger;
    }

    auto Name() const -> const std::string& override
    {
        return _name;
    }

private:
    // ----------------------------------------
    // Private members
    bool _headerWritten{false};
    std::ofstream _file;
    std::unique_ptr<NamedPipe> _pipe;
    std::mutex _lock;
    std::string _name;
    std::string _busName;
    std::string _outputPath;
    mw::logging::ILogger* _logger{nullptr};
};

} // namespace tracing
} // namespace ib
