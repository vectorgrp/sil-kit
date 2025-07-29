// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <functional>

#include "silkit/services/fwd_decl.hpp"
#include "silkit/services/orchestration/fwd_decl.hpp"
#include "silkit/experimental/netsim/fwd_decl.hpp"

#include "silkit/services/pubsub/PubSubDatatypes.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/services/rpc/RpcSpec.hpp"
#include "silkit/services/rpc/RpcDatatypes.hpp"

namespace SilKit {

/*! \brief Communication interface to be used by SIL Kit participants
 *
 */
class IParticipant
{
public:
    virtual ~IParticipant() = default;

    /* Methods Create*Controller() create controllers at this SIL Kit participant.
     *
     * Controllers provide an easy interface to interact with a simulated bus. They
     * act as a proxy to the controller implementation if a network simulator is connected
     * to the SIL Kit.
     *
     * Each Create*Controller() method creates a proxy instance, sets up all
     * necessary data structures and establishes the connection according to the
     * underlying middleware.
     */

    //! \brief Create a CAN controller at this SIL Kit participant.
    virtual auto CreateCanController(const std::string& canonicalName,
                                     const std::string& networkName) -> Services::Can::ICanController* = 0;

    //! \brief Create an Ethernet controller at this SIL Kit participant.
    virtual auto CreateEthernetController(const std::string& canonicalName, const std::string& networkName)
        -> Services::Ethernet::IEthernetController* = 0;

    //! \brief Create an FlexRay controller at this SIL Kit participant.
    virtual auto CreateFlexrayController(const std::string& canonicalName,
                                         const std::string& networkName) -> Services::Flexray::IFlexrayController* = 0;

    //! \brief Create a LIN controller at this SIL Kit participant.
    virtual auto CreateLinController(const std::string& canonicalName,
                                     const std::string& networkName) -> Services::Lin::ILinController* = 0;

    //! \brief Create a data publisher at this SIL Kit participant.
    virtual auto CreateDataPublisher(const std::string& canonicalName,
                                     const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                                     size_t history = 0) -> Services::PubSub::IDataPublisher* = 0;

    //! \brief Create a data subscriber at this SIL Kit participant.
    virtual auto CreateDataSubscriber(
        const std::string& canonicalName, const SilKit::Services::PubSub::PubSubSpec& dataSpec,
        Services::PubSub::DataMessageHandler dataMessageHandler) -> Services::PubSub::IDataSubscriber* = 0;

    //! \brief Create a Rpc client at this SIL Kit participant.
    virtual auto CreateRpcClient(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                                 Services::Rpc::RpcCallResultHandler handler) -> Services::Rpc::IRpcClient* = 0;

    //! \brief Create a Rpc server at this SIL Kit participant.
    virtual auto CreateRpcServer(const std::string& canonicalName, const SilKit::Services::Rpc::RpcSpec& dataSpec,
                                 Services::Rpc::RpcCallHandler handler) -> Services::Rpc::IRpcServer* = 0;

    //! \brief Return the ILifecycleService at this SIL Kit participant.
    virtual auto CreateLifecycleService(Services::Orchestration::LifecycleConfiguration startConfiguration)
        -> Services::Orchestration::ILifecycleService* = 0;

    //! \brief Return the ISystemMonitor at this SIL Kit participant.
    virtual auto CreateSystemMonitor() -> Services::Orchestration::ISystemMonitor* = 0;

    //! \brief Return the ILogger at this SIL Kit participant.
    virtual auto GetLogger() -> Services::Logging::ILogger* = 0;
};

} // namespace SilKit
