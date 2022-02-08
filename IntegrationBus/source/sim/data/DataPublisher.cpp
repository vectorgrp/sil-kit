// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataPublisher.hpp"
#include "IComAdapterInternal.hpp"
#include "DataMessageDatatypeUtils.hpp"

namespace ib {
namespace sim {
namespace data {

DataPublisher::DataPublisher(mw::IComAdapterInternal* comAdapter, cfg::DataPort config, mw::sync::ITimeProvider* timeProvider)
  : _comAdapter{comAdapter}, _timeProvider{timeProvider}
{
    _config = std::move(config);
}

auto DataPublisher::Config() const -> const cfg::DataPort&
{
    return _config;
}

void DataPublisher::Publish(std::vector<uint8_t> data)
{
    DataMessage msg{std::move(data)};
    _comAdapter->SendIbMessage(this, std::move(msg));
}

void DataPublisher::Publish(const uint8_t* data, std::size_t size)
{
    Publish({data, data + size});
}

void DataPublisher::SetTimeProvider(mw::sync::ITimeProvider* provider)
{
    _timeProvider = provider;
}

} // namespace data
} // namespace sim
} // namespace ib
