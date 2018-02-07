// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"
#include "ib/sim/generic/IGenericSubscriber.hpp"
#include "ib/sim/generic/IIbToGenericSubscriber.hpp"

namespace ib {
namespace sim {
namespace generic {

class GenericSubscriber
    : public IGenericSubscriber
    , public IIbToGenericSubscriber
{
public:
    // ----------------------------------------
    // Constructors and Destructor
    GenericSubscriber() = delete;
    GenericSubscriber(const GenericSubscriber&) = default;
    GenericSubscriber(GenericSubscriber&&) = default;

    GenericSubscriber(mw::IComAdapter* comAdapter);
    GenericSubscriber(mw::IComAdapter* comAdapter, cfg::GenericPort config);

public:
    // ----------------------------------------
    // Operator Implementations
    GenericSubscriber& operator=(GenericSubscriber& other) = default;
    GenericSubscriber& operator=(GenericSubscriber&& other) = default;

public:
    void SetReceiveMessageHandler(CallbackT callback)  override;

    auto Config() const -> const cfg::GenericPort& override;

    void ReceiveIbMessage(mw::EndpointAddress from, const GenericMessage& msg) override;
    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

private:
    //private Members
    cfg::GenericPort _config{};
    mw::IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddr{};
    CallbackT _callback;
};

} // namespace generic
} // namespace sim
} // namespace ib
