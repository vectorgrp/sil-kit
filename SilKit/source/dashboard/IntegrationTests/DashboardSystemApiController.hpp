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
#include <future>
#include <map>
#include <set>
#include <thread>
#include <vector>
#include <utility>

#include "OatppHeaders.hpp"

#include "DataPublisherDto.hpp"
#include "DataSubscriberDto.hpp"
#include "ParticipantStatusDto.hpp"
#include "RpcClientDto.hpp"
#include "RpcServerDto.hpp"
#include "ServiceDto.hpp"
#include "SimulationCreationRequestDto.hpp"
#include "SimulationCreationResponseDto.hpp"
#include "SimulationEndDto.hpp"
#include "SystemStatusDto.hpp"

#include "TestResult.hpp"

using namespace std::chrono_literals;

namespace SilKit {
namespace Dashboard {

#include OATPP_CODEGEN_BEGIN(ApiController)

class DashboardSystemApiController : public oatpp::web::server::api::ApiController
{
public:
    DashboardSystemApiController(uint64_t expectedSimulationsCount, std::chrono::seconds creationTimeout,
                                 std::chrono::seconds updateTimeout, const std::shared_ptr<ObjectMapper>& objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
        , _expectedSimulationsCount(expectedSimulationsCount)
        , _creationTimeout(creationTimeout)
        , _updateTimeout(updateTimeout)
    {
        _allSimulationsFinishedFuture = _allSimulationsFinishedPromise.get_future();
        if (_expectedSimulationsCount == 0)
        {
            _allSimulationsFinishedPromise.set_value();
        }
    }

private:
    uint64_t _expectedSimulationsCount;
    std::chrono::seconds _creationTimeout;
    std::chrono::seconds _updateTimeout;
    std::atomic<uint64_t> _simulationId{0};
    std::mutex _mutex;
    std::map<uint64_t, SimulationData> _data;
    std::promise<void> _allSimulationsFinishedPromise;
    std::future<void> _allSimulationsFinishedFuture;
    std::future_status _allSimulationsFinishedStatus;

public:
    static std::shared_ptr<DashboardSystemApiController> createShared(
        uint64_t expectedFinishedSimulationsCount, std::chrono::seconds creationTimeout,
        std::chrono::seconds updateTimeout,
        const std::shared_ptr<ObjectMapper>& objectMapper = OATPP_GET_COMPONENT(std::shared_ptr<ObjectMapper>))
    {
        return std::make_shared<DashboardSystemApiController>(expectedFinishedSimulationsCount, creationTimeout,
                                                              updateTimeout, objectMapper);
    }

    std::map<uint64_t, SimulationData> GetData()
    {
        return _data;
    }

    ENDPOINT("POST", "system-service/v1.0/simulations", createSimulation,
             BODY_DTO(Object<SilKit::Dashboard::SimulationCreationRequestDto>, simulation))
    {
        SILKIT_UNUSED_ARG(simulation);
        std::this_thread::sleep_for(_creationTimeout);
        auto body = SilKit::Dashboard::SimulationCreationResponseDto::createShared();
        body->id = ++_simulationId;
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[body->id] = {};
        return createDtoResponse(Status::CODE_201, body);
    }

    ENDPOINT("PUT", "system-service/v1.0/simulations/{simulationId}/participants/{participantName}",
             addParticipantToSimulation, PATH(UInt64, simulationId), PATH(String, participantName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].participants.insert(participantName);
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("POST", "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/statuses",
             addParticipantStatusForSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             BODY_DTO(Object<SilKit::Dashboard::ParticipantStatusDto>, participantStatus))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(participantStatus, Status::CODE_400, "participantStatus not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].statesByParticipant[participantName].insert(
            oatpp::Enum<ParticipantState>::getEntryByValue(participantStatus->state).name.toString());
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/cancontrollers/{serviceId}",
             addCanControllerForParticipantOfSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(UInt64, serviceId), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, canController))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(canController, Status::CODE_400, "canController not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(std::pair<uint64_t, Service>(
            serviceId, {"", "cancontroller", canController->name, canController->networkName, {}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/ethernetcontrollers/"
             "{serviceId}",
             addEthernetControllerForParticipantOfSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(UInt64, serviceId), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, ethernetController))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(ethernetController, Status::CODE_400, "ethernetController not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(std::pair<uint64_t, Service>(
            serviceId, {"", "ethernetcontroller", ethernetController->name, ethernetController->networkName, {}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/flexraycontrollers/"
             "{serviceId}",
             addFlexrayControllerForParticipantOfSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(UInt64, serviceId), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, flexrayController))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(flexrayController, Status::CODE_400, "flexrayController not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(std::pair<uint64_t, Service>(
            serviceId, {"", "flexraycontroller", flexrayController->name, flexrayController->networkName, {}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/lincontrollers/{serviceId}",
             addLinControllerForParticipantOfSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(UInt64, serviceId), BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, linController))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(linController, Status::CODE_400, "linController not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(std::pair<uint64_t, Service>(
            serviceId, {"", "lincontroller", linController->name, linController->networkName, {}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/datapublishers/{serviceId}",
             addDataPublisherForParticipantOfSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(UInt64, serviceId), BODY_DTO(Object<SilKit::Dashboard::DataPublisherDto>, dataPublisher))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(dataPublisher, Status::CODE_400, "dataPublisher not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(
            std::pair<uint64_t, Service>(serviceId, {"",
                                                     "datapublisher",
                                                     dataPublisher->name,
                                                     dataPublisher->networkName,
                                                     {dataPublisher->spec->topic, "", dataPublisher->spec->mediaType,
                                                      GetLabels(dataPublisher->spec->labels)}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/datasubscribers/{serviceId}",
        addDataSubscriberForParticipantOfSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
        PATH(UInt64, serviceId), BODY_DTO(Object<SilKit::Dashboard::DataSubscriberDto>, dataSubscriber))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(dataSubscriber, Status::CODE_400, "dataSubscriber not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(
            std::pair<uint64_t, Service>(serviceId, {"",
                                                     "datasubscriber",
                                                     dataSubscriber->name,
                                                     "",
                                                     {dataSubscriber->spec->topic, "", dataSubscriber->spec->mediaType,
                                                      GetLabels(dataSubscriber->spec->labels)}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/datasubscribers/"
             "{parentServiceId}/internals/{serviceId}",
             addDataSubscriberInternalForParticipantOfSimulation, PATH(UInt64, simulationId),
             PATH(String, participantName), PATH(String, parentServiceId), PATH(UInt64, serviceId),
             BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, dataSubscriberInternal))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(dataSubscriberInternal, Status::CODE_400, "dataSubscriberInternal not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(
            std::pair<uint64_t, Service>(serviceId, {parentServiceId,
                                                     "datasubscriberinternal",
                                                     dataSubscriberInternal->name,
                                                     dataSubscriberInternal->networkName,
                                                     {}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/rpcclients/{serviceId}",
             addRpcClientForParticipantOfSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(UInt64, serviceId), BODY_DTO(Object<SilKit::Dashboard::RpcClientDto>, rpcClient))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(rpcClient, Status::CODE_400, "rpcClient not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(std::pair<uint64_t, Service>(
            serviceId,
            {"",
             "rpcclient",
             rpcClient->name,
             rpcClient->networkName,
             {"", rpcClient->spec->functionName, rpcClient->spec->mediaType, GetLabels(rpcClient->spec->labels)}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/rpcservers/{serviceId}",
             addRpcServerForParticipantOfSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(UInt64, serviceId), BODY_DTO(Object<SilKit::Dashboard::RpcServerDto>, rpcServer))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(rpcServer, Status::CODE_400, "rpcServer not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(std::pair<uint64_t, Service>(
            serviceId,
            {"",
             "rpcserver",
             rpcServer->name,
             "",
             {"", rpcServer->spec->functionName, rpcServer->spec->mediaType, GetLabels(rpcServer->spec->labels)}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/rpcservers/"
             "{parentServiceId}/internals/{serviceId}",
             addRpcServerInternalForParticipantOfSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(String, parentServiceId), PATH(UInt64, serviceId),
             BODY_DTO(Object<SilKit::Dashboard::ServiceDto>, rpcServerInternal))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(rpcServerInternal, Status::CODE_400, "rpcServerInternal not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].servicesByParticipant[participantName].insert(std::pair<uint64_t, Service>(
            serviceId,
            {parentServiceId, "rpcserverinternal", rpcServerInternal->name, rpcServerInternal->networkName, {}}));
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/cannetworks/{networkName}",
             addCanNetworkToSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(String, networkName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].linksByParticipant[participantName].insert({"can", networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/ethernetnetworks/{networkName}",
        addEthernetNetworkToSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
        PATH(String, networkName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].linksByParticipant[participantName].insert({"ethernet", networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT(
        "PUT",
        "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/flexraynetworks/{networkName}",
        addFlexrayNetworkToSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
        PATH(String, networkName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].linksByParticipant[participantName].insert({"flexray", networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT",
             "system-service/v1.0/simulations/{simulationId}/participants/{participantName}/linnetworks/{networkName}",
             addLinNetworkToSimulation, PATH(UInt64, simulationId), PATH(String, participantName),
             PATH(String, networkName))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].linksByParticipant[participantName].insert({"lin", networkName});
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("PUT", "system-service/v1.0/simulations/{simulationId}/system/status", updateSystemStatusForSimulation,
             PATH(UInt64, simulationId), BODY_DTO(Object<SilKit::Dashboard::SystemStatusDto>, systemStatus))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(systemStatus, Status::CODE_400, "systemStatus not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].systemStates.insert(
            oatpp::Enum<SystemState>::getEntryByValue(systemStatus->state).name.toString());
        return createResponse(Status::CODE_204, "");
    }

    ENDPOINT("POST", "system-service/v1.0/simulations/{simulationId}", setSimulationEnd, PATH(UInt64, simulationId),
             BODY_DTO(Object<SilKit::Dashboard::SimulationEndDto>, simulation))
    {
        std::this_thread::sleep_for(_updateTimeout);
        OATPP_ASSERT_HTTP(simulationId <= _simulationId, Status::CODE_404, "simulationId not found");
        OATPP_ASSERT_HTTP(simulation, Status::CODE_400, "simulation not set");
        std::unique_lock<decltype(_mutex)> lock(_mutex);
        _data[simulationId].stopped = true;
        if (static_cast<uint64_t>(CountFinishedSimulations()) == _expectedSimulationsCount)
        {
            _allSimulationsFinishedPromise.set_value();
        }
        return createResponse(Status::CODE_204, "");
    }

    int CountFinishedSimulations()
    {
        auto finishedSimulationsCount = 0;
        for (auto const& d : _data)
        {
            if (d.second.stopped)
            {
                finishedSimulationsCount++;
            }
        }
        return finishedSimulationsCount;
    }

    void WaitSimulationsFinished()
    {
        _allSimulationsFinishedStatus = _allSimulationsFinishedFuture.wait_for(5s);
    }

    bool AllSimulationsFinished()
    {
        return _allSimulationsFinishedStatus == std::future_status::ready;
    }

private:
    std::vector<SilKit::Services::MatchingLabel> GetLabels(oatpp::Vector<Object<MatchingLabelDto>> labels)
    {
        std::vector<SilKit::Services::MatchingLabel> res;
        for (auto& label : *labels)
        {
            res.push_back({label->key, label->value,
                           label->kind == LabelKind::Mandatory ? SilKit::Services::MatchingLabel::Kind::Mandatory
                                                               : SilKit::Services::MatchingLabel::Kind::Optional});
        }
        return res;
    }
};

#include OATPP_CODEGEN_END(ApiController)

} // namespace Dashboard
} // namespace SilKit