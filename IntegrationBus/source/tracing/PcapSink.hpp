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
        const sim::eth::EthFrame& msg
    ) override;
   
    void SetLogger(mw::logging::ILogger* logger) override
    {
        _logger = logger;
    }

    auto Name() const -> const std::string& override
    {
        return _name;
    }

    void Trace(tracing::Direction /*txRx*/,
        const mw::EndpointAddress& /*id*/,
        std::chrono::nanoseconds /*timestamp*/,
        const sim::can::CanMessage& /*msg*/
    ) override {
        throw std::runtime_error("PcapSink: CanMessage type not supported");
    }

    void Trace(tracing::Direction /*txRx*/,
        const mw::EndpointAddress& /*id*/,
        std::chrono::nanoseconds /*timestamp*/,
        const sim::generic::GenericMessage& /*msg*/
    ) override {
        throw std::runtime_error("PcapSink: GenericMessage type not supported");
    }
       
private:
    // ----------------------------------------
    // Private members
    std::ofstream _file;
    std::unique_ptr<NamedPipe> _pipe;
    std::mutex _lock;
    std::string _name;
    std::string _busName;
    mw::logging::ILogger* _logger{nullptr};
};

} // namespace tracing
} // namespace ib
