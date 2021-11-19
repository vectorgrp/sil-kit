// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/generic/IGenericPublisher.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"

#include "IIbToGenericPublisher.hpp"
#include "IComAdapterInternal.hpp"

namespace ib {
namespace sim {
namespace generic {

class GenericPublisher
    : public IGenericPublisher
    , public IIbToGenericPublisher
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public mw::IServiceId
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    GenericPublisher() = delete;
    GenericPublisher(const GenericPublisher&) = default;
    GenericPublisher(GenericPublisher&&) = default;

    GenericPublisher(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider);
    GenericPublisher(mw::IComAdapterInternal* comAdapter, cfg::GenericPort config, mw::sync::ITimeProvider* timeProvider);

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

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IServiceId
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;

private:
    //private Members
    cfg::GenericPort _config{};
    mw::IComAdapterInternal* _comAdapter{nullptr};
    ::ib::mw::ServiceId _serviceId;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    extensions::Tracer _tracer;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void GenericPublisher::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(EndpointAddress(), *sink);
}

void GenericPublisher::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}
auto GenericPublisher::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}
} // namespace generic
} // namespace sim
} // namespace ib
