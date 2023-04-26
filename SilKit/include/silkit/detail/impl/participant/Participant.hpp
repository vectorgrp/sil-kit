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

#include "silkit/detail/impl/services/can/CanController.hpp"

#include "silkit/detail/impl/services/ethernet/EthernetController.hpp"

#include "silkit/detail/impl/services/flexray/FlexrayController.hpp"

#include "silkit/detail/impl/services/lin/LinController.hpp"

#include "silkit/detail/impl/services/logging/Logger.hpp"

#include "silkit/detail/impl/services/orchestration/LifecycleService.hpp"
#include "silkit/detail/impl/services/orchestration/SystemMonitor.hpp"

#include "silkit/detail/impl/services/pubsub/DataPublisher.hpp"
#include "silkit/detail/impl/services/pubsub/DataSubscriber.hpp"

#include "silkit/detail/impl/services/rpc/RpcClient.hpp"
#include "silkit/detail/impl/services/rpc/RpcServer.hpp"

#include "silkit/detail/impl/experimental/services/orchestration/SystemController.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {

class Participant : public SilKit::IParticipant
{
public:
    inline explicit Participant(SilKit_Participant* participant);

    inline ~Participant() override;

    inline auto CreateCanController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Can::ICanController* override;

    inline auto CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Ethernet::IEthernetController* override;

    inline auto CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Flexray::IFlexrayController* override;

    inline auto CreateLinController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Lin::ILinController* override;

    inline auto CreateDataPublisher(const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                             size_t history) -> SilKit::Services::PubSub::IDataPublisher* override;

    inline auto CreateDataSubscriber(const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                              SilKit::Services::PubSub::DataMessageHandler dataMessageHandler)
        -> SilKit::Services::PubSub::IDataSubscriber* override;

    inline auto CreateRpcClient(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                         SilKit::Services::Rpc::RpcCallResultHandler handler)
        -> SilKit::Services::Rpc::IRpcClient* override;

    inline auto CreateRpcServer(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                         SilKit::Services::Rpc::RpcCallHandler handler) -> SilKit::Services::Rpc::IRpcServer* override;

    inline auto CreateLifecycleService(SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration)
        -> SilKit::Services::Orchestration::ILifecycleService* override;

    inline auto CreateSystemMonitor() -> SilKit::Services::Orchestration::ISystemMonitor* override;

    inline auto GetLogger() -> SilKit::Services::Logging::ILogger* override;

public:
    inline auto ExperimentalCreateSystemController()
        -> SilKit::Experimental::Services::Orchestration::ISystemController*;

public:
    inline auto Get() const -> SilKit_Participant*;

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

    ControllerStorage<Services::Can::CanController> _canControllers;
    ControllerStorage<Services::Ethernet::EthernetController> _ethernetControllers;
    ControllerStorage<Services::Flexray::FlexrayController> _flexrayControllers;
    ControllerStorage<Services::Lin::LinController> _linControllers;
    ControllerStorage<Services::PubSub::DataPublisher> _dataPublishers;
    ControllerStorage<Services::PubSub::DataSubscriber> _dataSubscribers;
    ControllerStorage<Services::Rpc::RpcClient> _rpcClients;
    ControllerStorage<Services::Rpc::RpcServer> _rpcServers;

    std::unique_ptr<Impl::Services::Orchestration::LifecycleService> _lifecycleService;
    std::unique_ptr<Services::Orchestration::SystemMonitor> _systemMonitor;

    std::unique_ptr<Impl::Experimental::Services::Orchestration::SystemController> _systemController;

    std::unique_ptr<Impl::Services::Logging::Logger> _logger;
};

} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {

Participant::Participant(SilKit_Participant* participant)
    : _participant{participant}
{
    _logger = std::make_unique<Services::Logging::Logger>(_participant);
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
    _lifecycleService = std::make_unique<Services::Orchestration::LifecycleService>(_participant, startConfiguration);

    return _lifecycleService.get();
}

auto Participant::CreateSystemMonitor() -> SilKit::Services::Orchestration::ISystemMonitor*
{
    _systemMonitor = std::make_unique<Services::Orchestration::SystemMonitor>(_participant);

    return _systemMonitor.get();
}

auto Participant::GetLogger() -> SilKit::Services::Logging::ILogger*
{
    return _logger.get();
}

auto Participant::ExperimentalCreateSystemController()
    -> SilKit::Experimental::Services::Orchestration::ISystemController*
{
    _systemController = std::make_unique<Impl::Experimental::Services::Orchestration::SystemController>(_participant);

    return _systemController.get();
}

auto Participant::Get() const -> SilKit_Participant*
{
    return _participant;
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
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
