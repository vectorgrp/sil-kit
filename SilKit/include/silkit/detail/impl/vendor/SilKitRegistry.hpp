// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "silkit/capi/Participant.h"

#include "silkit/participant/exception.hpp"

#include "silkit/vendor/ISilKitRegistry.hpp"

#include "silkit/detail/impl/services/logging/Logger.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Vendor {
namespace Vector {

class SilKitRegistry : public SilKit::Vendor::Vector::ISilKitRegistry
{
public:
    inline explicit SilKitRegistry(SilKit_Vendor_Vector_SilKitRegistry* silKitRegistry);

    inline ~SilKitRegistry() override;

    inline void SetAllConnectedHandler(std::function<void()> handler) override;

    inline void SetAllDisconnectedHandler(std::function<void()> handler) override;

    inline auto GetLogger() -> SilKit::Services::Logging::ILogger* override;

    inline auto StartListening(const std::string& listenUri) -> std::string override;

private:
    SilKit_Vendor_Vector_SilKitRegistry* _silKitRegistry{nullptr};

    std::unique_ptr<std::function<void()>> _allDisconnectedHandler;

    std::mutex _loggerMx;
    std::unique_ptr<Impl::Services::Logging::Logger> _logger;
};

} // namespace Vector
} // namespace Vendor
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Vendor {
namespace Vector {

SilKitRegistry::SilKitRegistry(SilKit_Vendor_Vector_SilKitRegistry* silKitRegistry)
    : _silKitRegistry{silKitRegistry}
{
}

SilKitRegistry::~SilKitRegistry()
{
    if (_silKitRegistry)
    {
        SilKit_Vendor_Vector_SilKitRegistry_Destroy(_silKitRegistry);
    }
}

void SilKitRegistry::SetAllConnectedHandler(std::function<void()> handler)
{
    SILKIT_UNUSED_ARG(handler);
    throw SilKit::SilKitError{
        "SilKitRegistry::SetAllConnectedHandler: non functional, please use the system monitor instead"};
}

void SilKitRegistry::SetAllDisconnectedHandler(std::function<void()> handler)
{
    auto ownedHandlerPtr = std::make_unique<std::function<void()>>(handler);

    const auto cHandler = [](void* context, SilKit_Vendor_Vector_SilKitRegistry* registry) {
        SILKIT_UNUSED_ARG(registry);

        const auto handlerPtr = static_cast<std::function<void()>*>(context);
        (*handlerPtr)();
    };

    const auto returnCode =
        SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler(_silKitRegistry, ownedHandlerPtr.get(), cHandler);
    ThrowOnError(returnCode);

    _allDisconnectedHandler = std::move(ownedHandlerPtr);
}

auto SilKitRegistry::GetLogger() -> SilKit::Services::Logging::ILogger*
{
    std::unique_lock<decltype(_loggerMx)> lock{_loggerMx};
    if (!_logger)
    {
        _logger = std::make_unique<Services::Logging::Logger>(_silKitRegistry);
    }
    lock.unlock();

    return _logger.get();
}

auto SilKitRegistry::StartListening(const std::string& listenUri) -> std::string
{
    const char* cRegistryUri{nullptr};

    const auto returnCode =
        SilKit_Vendor_Vector_SilKitRegistry_StartListening(_silKitRegistry, listenUri.c_str(), &cRegistryUri);
    ThrowOnError(returnCode);

    return std::string{cRegistryUri};
}

} // namespace Vector
} // namespace Vendor
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
