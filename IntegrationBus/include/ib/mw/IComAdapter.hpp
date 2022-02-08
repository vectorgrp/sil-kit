// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <functional>

#include "ib/mw/fwd_decl.hpp"
#include "ib/cfg/fwd_decl.hpp"
#include "ib/sim/fwd_decl.hpp"

#include "ib/sim/data/DataMessageDatatypes.hpp"
#include "ib/sim/rpc/RpcDatatypes.hpp"

namespace ib {
namespace mw {

/*! \brief Communication interface to be used by IB participants
 *
 */
class IComAdapter
{
public:
    virtual ~IComAdapter() = default;

    /* Methods Create*Controller() create controllers at this IB participant.
     *
     * Controllers provide an easy interface to interact with a simulated bus. They
     * act as a proxy to the controller implementation in a Network Simulator connected
     * to the Integration Bus.
     *
     * Each Create*Controller() method creates a proxy instance, sets up all
     * necessary data structures and establishes the connection according to the
     * underlying middleware.
     */

     //! \brief Create a CAN controller at this IB participant.
    virtual auto CreateCanController(const std::string& canonicalName) -> sim::can::ICanController* = 0;
    //! \brief Create an Ethernet controller at this IB participant.
    virtual auto CreateEthController(const std::string& canonicalName) -> sim::eth::IEthController* = 0;
    //! \brief Create an Ethernet controller at this IB participant.
    virtual auto CreateFlexrayController(const std::string& canonicalName) -> sim::fr::IFrController* = 0;
    //! \brief Create a LIN controller at this IB participant.
    virtual auto CreateLinController(const std::string& canonicalName) -> sim::lin::ILinController* = 0;

    //! \brief Create a generic message publisher at this IB participant.
    virtual auto CreateGenericPublisher(const std::string& canonicalName) -> sim::generic::IGenericPublisher* = 0;
    //! \brief Create a generic message subscriber at this IB participant.
    virtual auto CreateGenericSubscriber(const std::string& canonicalName) -> sim::generic::IGenericSubscriber* = 0;
    
    //! \brief Create a data publisher at this IB participant.
    virtual auto CreateDataPublisher(const std::string& topic, const sim::data::DataExchangeFormat& dataExchangeFormat,
                                     const std::map<std::string, std::string>& labels, size_t history = 0)
        -> sim::data::IDataPublisher* = 0;
    //! \brief Create a data subscriber at this IB participant.
    virtual auto CreateDataSubscriber(const std::string& topic, const sim::data::DataExchangeFormat& dataExchangeFormat,
                                      const std::map<std::string, std::string>& labels,
                                      sim::data::DataHandlerT defaultDataHandler,
                                      sim::data::NewDataSourceHandlerT newDataSourceHandler = nullptr)
        -> sim::data::IDataSubscriber* = 0;

    
    //! \brief Create a Rpc client at this IB participant.
    virtual auto CreateRpcClient(const std::string& functionName, const sim::rpc::RpcExchangeFormat exchangeFormat,
                                 sim::rpc::CallReturnHandler handler) -> sim::rpc::IRpcClient* = 0;
    //! \brief Create a Rpc server at this IB participant.
    virtual auto CreateRpcServer(const std::string& functionName, const sim::rpc::RpcExchangeFormat exchangeFormat,
                                 sim::rpc::CallProcessor handler) -> sim::rpc::IRpcServer* = 0;
    
    //! \brief Return the  IParticipantController for the current participant.
    virtual auto GetParticipantController() -> sync::IParticipantController* = 0;
    virtual auto GetSystemMonitor() -> sync::ISystemMonitor* = 0;
    virtual auto GetSystemController() -> sync::ISystemController* = 0;
    virtual auto GetLogger() -> logging::ILogger* = 0;
};

} // mw
} // namespace ib
