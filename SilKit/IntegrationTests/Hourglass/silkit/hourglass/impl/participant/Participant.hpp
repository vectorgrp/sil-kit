#pragma once

#include <forward_list>

#include "silkit/capi/Participant.h"

#include "silkit/participant/IParticipant.hpp"
#include "silkit/participant/exception.hpp"

#include "silkit/hourglass/impl/Macros.hpp"

#include "silkit/hourglass/impl/services/can/CanController.hpp"

#include "silkit/hourglass/impl/services/ethernet/EthernetController.hpp"

#include "silkit/hourglass/impl/services/lin/LinController.hpp"

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
    explicit Participant(SilKit_Participant* participant)
        : _participant{participant}
    {
    }

    ~Participant() override
    {
        if (_participant)
        {
            SilKit_Participant_Destroy(_participant);
        }
    }

    auto CreateCanController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Can::ICanController* override
    {
        _canControllers.emplace_front(_participant, canonicalName, networkName);
        return &_canControllers.front();
    }

    auto CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Ethernet::IEthernetController* override
    {
        _ethernetControllers.emplace_front(_participant, canonicalName, networkName);
        return &_ethernetControllers.front();
    }

#define SILKIT_THROW_NOT_IMPLEMENTED_(FUNCTION_NAME) \
    SILKIT_HOURGLASS_IMPL_THROW_NOT_IMPLEMENTED("Participant", FUNCTION_NAME)

    auto CreateFlexrayController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Flexray::IFlexrayController* override
    {
        SILKIT_UNUSED_ARG(canonicalName);
        SILKIT_UNUSED_ARG(networkName);
        SILKIT_THROW_NOT_IMPLEMENTED_("CreateFlexrayController");
    }

    auto CreateLinController(const std::string& canonicalName, const std::string& networkName)
        -> SilKit::Services::Lin::ILinController* override
    {
        _linControllers.emplace_front(_participant, canonicalName, networkName);
        return &_linControllers.front();
    }

    auto CreateDataPublisher(const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                             size_t history) -> SilKit::Services::PubSub::IDataPublisher* override
    {
        _dataPublishers.emplace_front(_participant, canonicalName, dataSpec, history);
        return &_dataPublishers.front();
    }

    auto CreateDataSubscriber(const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                              SilKit::Services::PubSub::DataMessageHandler dataMessageHandler)
        -> SilKit::Services::PubSub::IDataSubscriber* override
    {
        _dataSubscribers.emplace_front(_participant, canonicalName, dataSpec, std::move(dataMessageHandler));
        return &_dataSubscribers.front();
    }

    auto CreateRpcClient(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                         SilKit::Services::Rpc::RpcCallResultHandler handler)
        -> SilKit::Services::Rpc::IRpcClient* override
    {
        _rpcClients.emplace_front(_participant, canonicalName, dataSpec, std::move(handler));
        return &_rpcClients.front();
    }

    auto CreateRpcServer(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                         SilKit::Services::Rpc::RpcCallHandler handler) -> SilKit::Services::Rpc::IRpcServer* override
    {
        _rpcServers.emplace_front(_participant, canonicalName, dataSpec, std::move(handler));
        return &_rpcServers.front();
    }

    auto CreateLifecycleService(SilKit::Services::Orchestration::LifecycleConfiguration startConfiguration)
        -> SilKit::Services::Orchestration::ILifecycleService* override
    {
        _lifecycleService = std::make_unique<SilKit::Hourglass::Impl::Services::Orchestration::LifecycleService>(
            _participant, startConfiguration);

        return _lifecycleService.get();
    }

    auto CreateSystemMonitor() -> SilKit::Services::Orchestration::ISystemMonitor* override
    {
        _systemMonitor =
            std::make_unique<SilKit::Hourglass::Impl::Services::Orchestration::SystemMonitor>(_participant);

        return _systemMonitor.get();
    }

    auto GetLogger() -> SilKit::Services::Logging::ILogger* override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("GetLogger");
    }

#undef SILKIT_THROW_NOT_IMPLEMENTED_

private:
    SilKit_Participant* _participant{nullptr};

    std::forward_list<SilKit::Hourglass::Impl::Services::Can::CanController> _canControllers;
    std::forward_list<SilKit::Hourglass::Impl::Services::Ethernet::EthernetController> _ethernetControllers;
    std::forward_list<SilKit::Hourglass::Impl::Services::Lin::LinController> _linControllers;
    std::forward_list<SilKit::Hourglass::Impl::Services::PubSub::DataPublisher> _dataPublishers;
    std::forward_list<SilKit::Hourglass::Impl::Services::PubSub::DataSubscriber> _dataSubscribers;
    std::forward_list<SilKit::Hourglass::Impl::Services::Rpc::RpcClient> _rpcClients;
    std::forward_list<SilKit::Hourglass::Impl::Services::Rpc::RpcServer> _rpcServers;

    std::unique_ptr<SilKit::Hourglass::Impl::Services::Orchestration::LifecycleService> _lifecycleService;
    std::unique_ptr<SilKit::Hourglass::Impl::Services::Orchestration::SystemMonitor> _systemMonitor;
};

} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
