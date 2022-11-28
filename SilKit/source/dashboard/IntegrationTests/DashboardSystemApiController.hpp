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

#include <atomic>
#include <cstdint>
#include <chrono>
#include <map>
#include <set>
#include <thread>
#include <vector>
#include <utility>

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "DataPublisherDto.hpp"
#include "ParticipantStatusDto.hpp"
#include "RpcClientDto.hpp"
#include "ServiceDto.hpp"
#include "SimulationCreationRequestDto.hpp"
#include "SimulationCreationResponseDto.hpp"
#include "SimulationEndDto.hpp"
#include "SystemStatusDto.hpp"

#include "TestResult.hpp"

namespace SilKit {
namespace Dashboard {

#include OATPP_CODEGEN_BEGIN(ApiController)

class DashboardSystemApiController : public oatpp::web::server::api::ApiController
{
public:
    DashboardSystemApiController(std::chrono::duration<long long> creationTimeout,
                                 std::chrono::duration<long long> updateTimeout,
                                 const std::shared_ptr<ObjectMapper>& objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
        , _creationTimeout(creationTimeout)
        , _updateTimeout(updateTimeout)
    {
    }

private:
    std::chrono::duration<long long> _creationTimeout;
    std::chrono::duration<long long> _updateTimeout;
    std::atomic<uint32_t> _simulationId{0};
    std::mutex _mutex;
    std::map<uint32_t, SimulationData> _data;

public:
    static std::shared_ptr<DashboardSystemApiController> createShared(
        std::chrono::duration<long long> creationTimeout,
        std::chrono::duration<long long> updateTimeout,
        const std::shared_ptr<ObjectMapper>& objectMapper = OATPP_GET_COMPONENT(std::shared_ptr<ObjectMapper>))
    {
        return std::make_shared<DashboardSystemApiController>(creationTimeout, updateTimeout, objectMapper);
    }

    std::map<uint32_t, SimulationData> GetData() { return _data; }

    ENDPOINT("POST", "system-service/v1.0/simulations", createSimulation,
             BODY_DTO(Object<SilKit::Dashboard::SimulationCreationRequestDto>, simulation))
    {
        std::this_thread::sleep_for(_creationTimeout);
        auto body = SilKit::Dashboard::SimulationCreationResponseDto::createShared();
        body->id = ++_simulationId;
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[body->id] = {};
        return createDtoResponse(Status::CODE_201, body);
    }

    ENDPOINT("PUT", "system-service/v1.0/simulations/{simulationId}/participants/{participantName}",
             addParticipantToSimulation, PATH(UInt32, simulationId), PATH(String, participantName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].participants.insert(participantName);
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("POST", "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/statuses",
             addParticipantStatusForSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             BODY_DTO(Object<SilKit::Dashboard::ParticipantStatusDto>, participantStatus))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(participantStatus, Status::CODE_400, "participantStatus not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].statesByParticipant[participantName].insert(
            oatpp::Enum<ParticipantState>::getEntryByValue(participantStatus->state).name.toString());
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/cancontrollers/{canonicalName}",
        addCanControllerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
        PATH(String, canonicalName), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, canController))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(canController, Status::CODE_400, "canController not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].servicesByParticipant[participantName].insert(
            {"cancontroller", canonicalName, canController->networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/ethernetcontrollers/"
             "{canonicalName}",
             addEthernetControllerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             PATH(String, canonicalName), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, ethernetController))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(ethernetController, Status::CODE_400, "ethernetController not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].servicesByParticipant[participantName].insert(
            {"ethernetcontroller", canonicalName, ethernetController->networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/flexraycontrollers/"
             "{canonicalName}",
             addFlexrayControllerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             PATH(String, canonicalName), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, flexrayController))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(flexrayController, Status::CODE_400, "flexrayController not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].servicesByParticipant[participantName].insert(
            {"flexraycontroller", canonicalName, flexrayController->networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/lincontrollers/{canonicalName}",
        addLinControllerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
        PATH(String, canonicalName), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, linController))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(linController, Status::CODE_400, "linController not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].servicesByParticipant[participantName].insert(
            {"lincontroller", canonicalName, linController->networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/datapublishers/{canonicalName}",
        addDataPublisherForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
        PATH(String, canonicalName), BODY_DTO(Object<SilKit::Dashboard::DataPublisherDto>, dataPublisher))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(dataPublisher, Status::CODE_400, "dataPublisher not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        std::vector<SilKit::Services::MatchingLabel> labels;
        for (auto& label : *dataPublisher->labels)
        {
            labels.push_back({label->key, label->value,
                              label->kind == LabelKind::Mandatory ? SilKit::Services::MatchingLabel::Kind::Mandatory
                                                                  : SilKit::Services::MatchingLabel::Kind::Optional});
        }
        _data[_simulationId].servicesByParticipant[participantName].insert(
            {"datapublisher", canonicalName, dataPublisher->networkName, dataPublisher->topic, "", labels});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/datasubscribers/{canonicalName}",
        addDataSubscriberForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
        PATH(String, canonicalName), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, dataSubscriber))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(dataSubscriber, Status::CODE_400, "dataSubscriber not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].servicesByParticipant[participantName].insert(
            {"datasubscriber", canonicalName, dataSubscriber->networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/rpcclients/{canonicalName}",
             addRpcClientForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             PATH(String, canonicalName), BODY_DTO(Object<SilKit::Dashboard::RpcClientDto>, rpcClient))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(rpcClient, Status::CODE_400, "rpcClient not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        std::vector<SilKit::Services::MatchingLabel> labels;
        for (auto& label : *rpcClient->labels)
        {
            labels.push_back({label->key, label->value,
                              label->kind == LabelKind::Mandatory ? SilKit::Services::MatchingLabel::Kind::Mandatory
                                                                  : SilKit::Services::MatchingLabel::Kind::Optional});
        }
        _data[_simulationId].servicesByParticipant[participantName].insert(
            {"rpcclient", canonicalName, rpcClient->networkName, "", rpcClient->functionName, labels});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/rpcservers/{canonicalName}",
             addRpcServerForParticipantOfSimulation, PATH(UInt32, simulationId), PATH(String, participantName),
             PATH(String, canonicalName), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, rpcServer))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(rpcServer, Status::CODE_400, "rpcServer not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].servicesByParticipant[participantName].insert(
            {"rpcserver", canonicalName, rpcServer->networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT", "system-service/v1.0/simulations/{simulationId}/cannetworks/{networkName}",
             addCanNetworkToSimulation, PATH(UInt32, simulationId), PATH(String, networkName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].links.insert({"can", networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT", "system-service/v1.0/simulations/{simulationId}/ethernetnetworks/{networkName}",
             addEthernetNetworkToSimulation, PATH(UInt32, simulationId), PATH(String, networkName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].links.insert({"ethernet", networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT", "system-service/v1.0/simulations/{simulationId}/flexraynetworks/{networkName}",
             addFlexrayNetworkToSimulation, PATH(UInt32, simulationId), PATH(String, networkName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].links.insert({"flexray", networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT", "system-service/v1.0/simulations/{simulationId}/linnetworks/{networkName}",
             addLinNetworkToSimulation, PATH(UInt32, simulationId), PATH(String, networkName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].links.insert({"lin", networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT", "system-service/v1.0/simulations/{simulationId}/system/status", updateSystemStatusForSimulation,
             PATH(UInt32, simulationId), BODY_DTO(Object<SilKit::Dashboard::SystemStatusDto>, systemStatus))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(systemStatus, Status::CODE_400, "systemStatus not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].systemStates.insert(
            oatpp::Enum<SystemState>::getEntryByValue(systemStatus->state).name.toString());
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("POST", "system-service/v1.0/simulations/{simulationId}", setSimulationEnd, PATH(UInt32, simulationId),
             BODY_DTO(Object<SilKit::Dashboard::SimulationEndDto>, simulation))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(simulation, Status::CODE_400, "simulation not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[_simulationId].stopped = true;
        return createResponse(Status::CODE_204, "");
    }
};

#include OATPP_CODEGEN_END(ApiController)

} // namespace Dashboard
} // namespace SilKit