// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <string>

#include "silkit/services/logging/fwd_decl.hpp"

namespace SilKit {
namespace Vendor {
namespace Vector {

//! \brief Dedicated SIL Kit registry for the Vector SIL Kit middleware.
class ISilKitRegistry
{
public:
    virtual ~ISilKitRegistry() = default;

    //! \brief Register the handler that is called when all participants are connected
    virtual void SetAllConnectedHandler(std::function<void()> handler) = 0;

    //! \brief Register the handler that is called when all participants are disconnected
    virtual void SetAllDisconnectedHandler(std::function<void()> handler) = 0;

    //! \brief Returns the logger that is used by the SIL Kit registry.
    virtual auto GetLogger() -> Services::Logging::ILogger* = 0;

    //! \brief Start to listen on the URI with scheme silkit://. e.g. silkit://localhost:8500
    virtual auto StartListening(const std::string& listenUri) -> std::string = 0;
};


} //end namespace Vector
} //end namespace Vendor
} //end namespace SilKit
