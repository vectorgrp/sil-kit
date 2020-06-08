// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/generic/IGenericPublisher.hpp"
#include "ib/sim/generic/IIbToGenericPublisher.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "Tracing.hpp"

#include <vector>

namespace ib {
namespace sim {
namespace generic {

class GenericPublisher
    : public IGenericPublisher
    , public IIbToGenericPublisher
    , public mw::sync::ITimeConsumer
    , public tracing::IControllerToTraceSink
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    GenericPublisher() = delete;
    GenericPublisher(const GenericPublisher&) = default;
    GenericPublisher(GenericPublisher&&) = default;

    GenericPublisher(mw::IComAdapter* comAdapter, mw::sync::ITimeProvider* timeProvider);
    GenericPublisher(mw::IComAdapter* comAdapter, cfg::GenericPort config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    GenericPublisher& operator=(GenericPublisher& other) = default;
    GenericPublisher& operator=(GenericPublisher&& other) = default;

public:
    void Publish(std::vector<uint8_t> data) override;
    void Publish(const uint8_t* data, std::size_t size) override;

    auto Config() const -> const cfg::GenericPort& override;

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // tracing::IControllerToTraceSink
    inline void AddSink(tracing::ITraceMessageSink* sink) override;

private:
    //private Members
    cfg::GenericPort _config{};
    mw::IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddr{};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    tracing::Tracer<GenericMessage> _tracer;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void GenericPublisher::AddSink(tracing::ITraceMessageSink* sink)
{
    _tracer.AddSink(EndpointAddress(), *sink);
}

} // namespace generic
} // namespace sim
} // namespace ib
