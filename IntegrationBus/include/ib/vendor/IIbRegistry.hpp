// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.
#pragma once

#include <cstdint>
#include <functional>

#include "ib/mw/logging/fwd_decl.hpp"

namespace ib {
namespace vendor {
inline namespace vector {

//! \brief Dedicated IB registry for the VAsio middleware.
//         This is a loadable runtime extension that is non-redistributable.
class IIbRegistry
{
public:
    virtual ~IIbRegistry() = default;
    //! \brief Provide the VAsio domain with the given domain ID
    virtual void ProvideDomain(uint32_t domainId) = 0;
    //! \brief Register the handler that is called when all participants are connected
    virtual void SetAllConnectedHandler(std::function<void()> handler) = 0;
    //! \brief Register the handler that is called when all participants are disconnected
    virtual void SetAllDisconnectedHandler(std::function<void()> handler) = 0;
    //! \brief Returns the logger that is used by the IB registry.
    virtual auto GetLogger() -> mw::logging::ILogger* = 0;
    //! \brief Provide the VAsio domain with the given listening URI with scheme vib://. e.g. vib://localhost:8500
    virtual void ProvideDomain(std::string listenUri) = 0;
};


}//end namespace vector
}//end namespace vendor
}//end namespace ib
