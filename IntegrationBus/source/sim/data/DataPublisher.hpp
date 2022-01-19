// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <future>

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/data/IDataPublisher.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToDataPublisher.hpp"
#include "IComAdapterInternal.hpp"

namespace ib {
namespace sim {
namespace data {

class DataPublisher
    : public IDataPublisher
    , public IIbToDataPublisher
    , public mw::sync::ITimeConsumer
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    DataPublisher() = delete;
    DataPublisher(const DataPublisher&) = default;
    DataPublisher(DataPublisher&&) = default;

    DataPublisher(mw::IComAdapterInternal* comAdapter, cfg::DataPort config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    DataPublisher& operator=(DataPublisher& other) = default;
    DataPublisher& operator=(DataPublisher&& other) = default;

public:
    void SendPublisherAnnouncement() const;
    
    void Publish(std::vector<uint8_t> data) override;
    void Publish(const uint8_t* data, std::size_t size) override;

    auto Config() const -> const cfg::DataPort& override;

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    //ib::mw::sync::ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* provider) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;


private:
    cfg::DataPort _config{};
    mw::IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
    mw::sync::ITimeProvider* _timeProvider{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataPublisher::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataPublisher::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace data
} // namespace sim
} // namespace ib
