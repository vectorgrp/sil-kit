// Copyright (c) 2023 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "silkit/capi/Participant.h"

#include "silkit/participant/IParticipant.hpp"
#include "silkit/participant/exception.hpp"

#include "silkit/hourglass/impl/services/can/CanController.hpp"

#include "silkit/hourglass/impl/services/ethernet/EthernetController.hpp"

#include "silkit/hourglass/impl/services/flexray/FlexrayController.hpp"

#include "silkit/hourglass/impl/services/lin/LinController.hpp"

#include "silkit/hourglass/impl/services/logging/Logger.hpp"

#include "silkit/hourglass/impl/services/orchestration/LifecycleService.hpp"
#include "silkit/hourglass/impl/services/orchestration/SystemMonitor.hpp"

#include "silkit/hourglass/impl/services/pubsub/DataPublisher.hpp"
#include "silkit/hourglass/impl/services/pubsub/DataSubscriber.hpp"

#include "silkit/hourglass/impl/services/rpc/RpcClient.hpp"
#include "silkit/hourglass/impl/services/rpc/RpcServer.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {

class Participant : public SilKit::IParticipant
{
public:
    explicit Participant(SilKit_Participant* participant);

    ~Participant() override;

    auto CreateCanController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Can::ICanController* override;

    auto CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Ethernet::IEthernetController* override;

    auto CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Flexray::IFlexrayController* override;

    auto CreateLinController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Lin::ILinController* override;

    auto CreateDataPublisher(const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                             size_t history) -> SilKit::Services::PubSub::IDataPublisher* override;

    auto CreateDataSubscriber(const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                              SilKit::Services::PubSub::DataMessageHandler dataMessageHandler)
        -> SilKit::Services::PubSub::IDataSubscriber* override;

    auto CreateRpcClient(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                         SilKit::Services::Rpc::RpcCallResultHandler handler)
        -> SilKit::Services::Rpc::IRpcClient* override;

    auto CreateRpcServer(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                         SilKit::Services::Rpc::RpcCallHandler handler) -> SilKit::Services::Rpc::IRpcServer* override;

    auto CreateLifecycleService(SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration)
        -> SilKit::Services::Orchestration::ILifecycleService* override;

    auto CreateSystemMonitor() -> SilKit::Services::Orchestration::ISystemMonitor* override;

    auto GetLogger() -> SilKit::Services::Logging::ILogger* override;

private:
    template <typename ControllerT>
    class ControllerStorage
    {
    public:
        template <typename... ArgT>
        auto Create(ArgT&&... arg) -> ControllerT*;

    private:
        std::mutex _mutex;
        std::vector<std::unique_ptr<ControllerT>> _controllers;
    };

private:
    SilKit_Participant* _participant{nullptr};

    ControllerStorage<SilKit::Hourglass::Impl::Services::Can::CanController> _canControllers;
    ControllerStorage<SilKit::Hourglass::Impl::Services::Ethernet::EthernetController> _ethernetControllers;
    ControllerStorage<SilKit::Hourglass::Impl::Services::Flexray::FlexrayController> _flexrayControllers;
    ControllerStorage<SilKit::Hourglass::Impl::Services::Lin::LinController> _linControllers;
    ControllerStorage<SilKit::Hourglass::Impl::Services::PubSub::DataPublisher> _dataPublishers;
    ControllerStorage<SilKit::Hourglass::Impl::Services::PubSub::DataSubscriber> _dataSubscribers;
    ControllerStorage<SilKit::Hourglass::Impl::Services::Rpc::RpcClient> _rpcClients;
    ControllerStorage<SilKit::Hourglass::Impl::Services::Rpc::RpcServer> _rpcServers;

    std::unique_ptr<SilKit::Hourglass::Impl::Services::Orchestration::LifecycleService> _lifecycleService;
    std::unique_ptr<SilKit::Hourglass::Impl::Services::Orchestration::SystemMonitor> _systemMonitor;

    std::mutex _loggerMx;
    std::unique_ptr<SilKit::Hourglass::Impl::Services::Logging::Logger> _logger;
};

} // namespace Impl
} // namespace Hourglass
} // namespace SilKit

// ================================================================================
//  Inline Implementations
// ================================================================================

namespace SilKit {
namespace Hourglass {
namespace Impl {

Participant::Participant(SilKit_Participant* participant)
    : _participant{participant}
{
}

Participant::~Participant()
{
    if (_participant)
    {
        SilKit_Participant_Destroy(_participant);
    }
}

auto Participant::CreateCanController(const std::string& canonicalName, const std::string& networkName)
    -> SilKit::Services::Can::ICanController*
{
    return _canControllers.Create(_participant, canonicalName, networkName);
}

auto Participant::CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
    -> SilKit::Services::Ethernet::IEthernetController*
{
    return _ethernetControllers.Create(_participant, canonicalName, networkName);
}

auto Participant::CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
    -> SilKit::Services::Flexray::IFlexrayController*
{
    return _flexrayControllers.Create(_participant, canonicalName, networkName);
}

auto Participant::CreateLinController(const std::string& canonicalName, const std::string& networkName)
    -> SilKit::Services::Lin::ILinController*
{
    return _linControllers.Create(_participant, canonicalName, networkName);
}

auto Participant::CreateDataPublisher(const std::string& canonicalName,
                                      const SilKit::Services::PubSub::PubSubSpec& dataSpec, size_t history)
    -> SilKit::Services::PubSub::IDataPublisher*
{
    return _dataPublishers.Create(_participant, canonicalName, dataSpec, static_cast<uint8_t>(history & 0xff));
}

auto Participant::CreateDataSubscriber(const std::string& canonicalName,
                                       const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                                       SilKit::Services::PubSub::DataMessageHandler dataMessageHandler)
    -> SilKit::Services::PubSub::IDataSubscriber*
{
    return _dataSubscribers.Create(_participant, canonicalName, dataSpec, std::move(dataMessageHandler));
}

auto Participant::CreateRpcClient(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                                  SilKit::Services::Rpc::RpcCallResultHandler handler)
    -> SilKit::Services::Rpc::IRpcClient*
{
    return _rpcClients.Create(_participant, canonicalName, dataSpec, std::move(handler));
}

auto Participant::CreateRpcServer(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                                  SilKit::Services::Rpc::RpcCallHandler handler) -> SilKit::Services::Rpc::IRpcServer*
{
    return _rpcServers.Create(_participant, canonicalName, dataSpec, std::move(handler));
}

auto Participant::CreateLifecycleService(SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration)
    -> SilKit::Services::Orchestration::ILifecycleService*
{
    _lifecycleService = std::make_unique<SilKit::Hourglass::Impl::Services::Orchestration::LifecycleService>(
        _participant, startConfiguration);

    return _lifecycleService.get();
}

auto Participant::CreateSystemMonitor() -> SilKit::Services::Orchestration::ISystemMonitor*
{
    _systemMonitor = std::make_unique<SilKit::Hourglass::Impl::Services::Orchestration::SystemMonitor>(_participant);

    return _systemMonitor.get();
}

auto Participant::GetLogger() -> SilKit::Services::Logging::ILogger*
{
    std::unique_lock<decltype(_loggerMx)> lock{_loggerMx};
    if (!_logger)
    {
        _logger = std::make_unique<SilKit::Hourglass::Impl::Services::Logging::Logger>(_participant);
    }
    lock.unlock();

    return _logger.get();
}

template <typename ControllerT>
template <typename... ArgT>
auto Participant::ControllerStorage<ControllerT>::Create(ArgT&&... arg) -> ControllerT*
{
    auto ownedControllerPtr = std::make_unique<ControllerT>(std::forward<ArgT>(arg)...);
    ControllerT* controllerPtr = ownedControllerPtr.get();

    std::unique_lock<decltype(_mutex)> lock{_mutex};
    _controllers.emplace_back(std::move(ownedControllerPtr));

    return controllerPtr;
}

} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
