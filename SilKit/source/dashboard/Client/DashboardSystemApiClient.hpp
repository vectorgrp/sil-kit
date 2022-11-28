/* Copyright (c) 2022 Vector Informatik GmbH
 
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "oatpp/web/client/ApiClient.hpp"

#include "SimulationCreationRequestDto.hpp"
#include "SystemStatusDto.hpp"
#include "ParticipantStatusDto.hpp"
#include "ServiceDto.hpp"
#include "DataPublisherDto.hpp"
#include "RpcClientDto.hpp"
#include "SimulationEndDto.hpp"

#include OATPP_CODEGEN_BEGIN(ApiClient)

namespace SilKit {
namespace Dashboard {

class DashboardSystemApiClient : public oatpp::web::client::ApiClient
{
    API_CLIENT_INIT(DashboardSystemApiClient)

    // notify a simulation has been started
    // get a simulationId in return that can be used to send additional data
    API_CALL_ASYNC("POST", "system-service/v1.0/simulations", createSimulation,
             BODY_DTO(Object<SimulationCreationRequestDto>, simulation))

    // notify a participant has been created for a given simulation
    API_CALL_ASYNC("PUT", "system-service/v1.0/simulations/{simulationId}/participants/{participantName}",
             addParticipantToSimulation, PATH(UInt32, simulationId), PATH(String, participantName))

    // notify a participant has entered a new state for a given simulation
    API_CALL_ASYNC("POST", "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/statuses",
             addParticipantStatusForSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             BODY_DTO(Object<ParticipantStatusDto>, participantStatus))

    // notify a participant has created a CAN controller for a given simulation
    API_CALL_ASYNC(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/cancontrollers/{canonicalName}",
        addCanControllerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
        PATH(String, canonicalName), BODY_DTO(Object<ServiceDto>, canController))

    // notify a participant has created an Ethernet controller for a given simulation
    API_CALL_ASYNC("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/ethernetcontrollers/"
             "{canonicalName}",
             addEthernetControllerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             PATH(String, canonicalName), BODY_DTO(Object<ServiceDto>, ethernetController))

    // notify a participant has created a FlexRay controller for a given simulation
    API_CALL_ASYNC("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/flexraycontrollers/"
             "{canonicalName}",
             addFlexrayControllerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             PATH(String, canonicalName), BODY_DTO(Object<ServiceDto>, flexrayController))

    // notify a participant has created a LIN controller for a given simulation
    API_CALL_ASYNC(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/lincontrollers/{canonicalName}",
        addLinControllerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
        PATH(String, canonicalName), BODY_DTO(Object<ServiceDto>, linController))

    // notify a participant has created a data publisher for a given simulation
    API_CALL_ASYNC(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/datapublishers/{canonicalName}",
        addDataPublisherForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
        PATH(String, canonicalName), BODY_DTO(Object<DataPublisherDto>, dataPublisher))

    // notify a participant has created a data subscriber for a given simulation
    API_CALL_ASYNC(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/datasubscribers/{canonicalName}",
        addDataSubscriberForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
        PATH(String, canonicalName), BODY_DTO(Object<ServiceDto>, dataSubscriber))

    // notify a participant has created a Rpc client for a given simulation
    API_CALL_ASYNC(
        "PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/rpcclients/{canonicalName}",
             addRpcClientForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             PATH(String, canonicalName), BODY_DTO(Object<RpcClientDto>, rpcClient))

    // notify a participant has created a Rpc Server for a given simulation
    API_CALL_ASYNC(
        "PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/rpcservers/{canonicalName}",
             addRpcServerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             PATH(String, canonicalName), BODY_DTO(Object<ServiceDto>, rpcServer))

    // notify a simulated CAN network has been created for a given simulation
    API_CALL_ASYNC("PUT", "system-service/v1.0/simulations/{simulationId}/cannetworks/{networkName}",
             addCanNetworkToSimulation, PATH(UInt32, simulationId), PATH(String, networkName))

    // notify a simulated Ethernet network has been created for a given simulation
    API_CALL_ASYNC("PUT", "system-service/v1.0/simulations/{simulationId}/ethernetnetworks/{networkName}",
             addEthernetNetworkToSimulation, PATH(UInt32, simulationId), PATH(String, networkName))

    // notify a simulated FlexRay network has been created for a given simulation
    API_CALL_ASYNC("PUT", "system-service/v1.0/simulations/{simulationId}/flexraynetworks/{networkName}",
             addFlexrayNetworkToSimulation, PATH(UInt32, simulationId), PATH(String, networkName))

    // notify a simulated LIN network has been created for a given simulation
    API_CALL_ASYNC("PUT", "system-service/v1.0/simulations/{simulationId}/linnetworks/{networkName}",
             addLinNetworkToSimulation, PATH(UInt32, simulationId), PATH(String, networkName))

    // notify the system has entered a new state for a given simulation
    API_CALL_ASYNC("PUT", "system-service/v1.0/simulations/{simulationId}/system/status",
                   updateSystemStatusForSimulation,
             PATH(UInt32, simulationId), BODY_DTO(Object<SystemStatusDto>, systemStatus))

    // notify the end of a simulation
    API_CALL_ASYNC("POST", "system-service/v1.0/simulations/{simulationId}", setSimulationEnd,
                   PATH(UInt32, simulationId),
             BODY_DTO(Object<SimulationEndDto>, simulation))
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(ApiClient)
