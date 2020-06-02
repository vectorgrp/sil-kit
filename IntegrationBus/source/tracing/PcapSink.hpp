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
   
    auto GetLogger() const -> mw::logging::ILogger* override
    {
        return _logger;
    }

    auto Name() const -> const std::string& override
    {
        return _name;
    }

    void Trace(tracing::Direction,
        const mw::EndpointAddress&,
        std::chrono::nanoseconds,
        const sim::can::CanMessage& 
    ) override {
        throw std::runtime_error("PcapSink: CanMessage type not supported");
    }

    void Trace(tracing::Direction,
        const mw::EndpointAddress&,
        std::chrono::nanoseconds,
        const sim::lin::Frame& 
    ) override {
        throw std::runtime_error("PcapSink: lin::Frame type not supported");
    }

    void Trace(tracing::Direction,
        const mw::EndpointAddress&,
        std::chrono::nanoseconds,
        const sim::generic::GenericMessage& 
    ) override {
        throw std::runtime_error("PcapSink: GenericMessage type not supported");
    }

    void Trace(tracing::Direction,
        const mw::EndpointAddress&,
        std::chrono::nanoseconds,
        const sim::io::AnalogIoMessage& 
    ) override {
        throw std::runtime_error("PcapSink: AnalogIoMessage type not supported");
    }

    void Trace(tracing::Direction,
        const mw::EndpointAddress&,
        std::chrono::nanoseconds,
        const sim::io::DigitalIoMessage& 
    ) override {
        throw std::runtime_error("PcapSink: DigitalIoMessage type not supported");
    }

    void Trace(tracing::Direction,
        const mw::EndpointAddress&,
        std::chrono::nanoseconds,
        const sim::io::PatternIoMessage& 
    ) override {
        throw std::runtime_error("PcapSink: PatternIoMessage type not supported");
    }

    void Trace(tracing::Direction,
        const mw::EndpointAddress&,
        std::chrono::nanoseconds,
        const sim::io::PwmIoMessage& 
    ) override {
        throw std::runtime_error("PcapSink: PwmIoMessage type not supported");
    }

    void Trace(tracing::Direction,
        const mw::EndpointAddress&,
        std::chrono::nanoseconds,
        const sim::fr::FrMessage& 
    ) override {
        throw std::runtime_error("PcapSink: FrMessage type not supported");
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
