// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/capi/SilKit.h"
#include "silkit/experimental/management/ManagementClient.hpp"

#include "capi/CapiImpl.hpp"
#include "config/ParticipantConfiguration.hpp"
#include "CreateParticipantImpl.hpp"

#include <chrono>
#include <cstring>
#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace {

namespace Mgmt = SilKit::Experimental::Management;

//! A SilKit_StringList referring to the strings of a vector, which must outlive it.
class CStringList
{
public:
    explicit CStringList(const std::vector<std::string>& strings)
    {
        _pointers.reserve(strings.size());
        for (const auto& string : strings)
        {
            _pointers.push_back(const_cast<char*>(string.c_str()));
        }
        _list.numStrings = _pointers.size();
        _list.strings = _pointers.data();
    }

    CStringList(const CStringList&) = delete;
    CStringList& operator=(const CStringList&) = delete;

    auto Get() const -> const SilKit_StringList*
    {
        return &_list;
    }

private:
    std::vector<char*> _pointers;
    SilKit_StringList _list{};
};

auto ToCpp(SilKit_Experimental_ManagementClient* client) -> Mgmt::ManagementClient*
{
    return reinterpret_cast<Mgmt::ManagementClient*>(client);
}

} // namespace


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ManagementClient_Create(
    SilKit_Experimental_ManagementClient** outClient, SilKit_ParticipantConfiguration* participantConfiguration,
    const char* registryUri)
try
{
    ASSERT_VALID_OUT_PARAMETER(outClient);
    ASSERT_VALID_POINTER_PARAMETER(participantConfiguration);
    ASSERT_VALID_POINTER_PARAMETER(registryUri);

    auto cppParticipantConfiguration = std::make_shared<SilKit::Config::ParticipantConfiguration>(
        *reinterpret_cast<SilKit::Config::ParticipantConfiguration*>(participantConfiguration));

    auto participant =
        SilKit::CreateParticipantImpl(std::move(cppParticipantConfiguration), Mgmt::MakeClientParticipantName(),
                                      Mgmt::MakeManagementUri(registryUri));

    *outClient =
        reinterpret_cast<SilKit_Experimental_ManagementClient*>(new Mgmt::ManagementClient(std::move(participant)));
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ManagementClient_Destroy(SilKit_Experimental_ManagementClient* client)
try
{
    ASSERT_VALID_POINTER_PARAMETER(client);

    delete ToCpp(client);
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL
SilKit_Experimental_ManagementClient_SetRegistryInfoHandler(SilKit_Experimental_ManagementClient* client, void* context,
                                                            SilKit_Experimental_ManagementRegistryInfoHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(client);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    ToCpp(client)->SetRegistryInfoHandler([client, context, handler](const Mgmt::RegistryInfo& registryInfo) {
        const CStringList features{registryInfo.features};

        SilKit_Experimental_ManagementRegistryInfo cRegistryInfo;
        SilKit_Struct_Init(SilKit_Experimental_ManagementRegistryInfo, cRegistryInfo);
        cRegistryInfo.apiVersionMajor = registryInfo.apiVersionMajor;
        cRegistryInfo.apiVersionMinor = registryInfo.apiVersionMinor;
        cRegistryInfo.silKitVersion = registryInfo.silKitVersion.c_str();
        cRegistryInfo.features = features.Get();

        handler(context, client, &cRegistryInfo);
    });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL
SilKit_Experimental_ManagementClient_SetSimulationsHandler(SilKit_Experimental_ManagementClient* client, void* context,
                                                           SilKit_Experimental_ManagementSimulationsHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(client);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    ToCpp(client)->SetSimulationsHandler(
        [client, context, handler](const Mgmt::Simulations& simulations, const Mgmt::SimulationsChange& change) {
        std::deque<CStringList> participantLists;
        std::vector<SilKit_Experimental_ManagementSimulation> cSimulations(simulations.simulations.size());
        for (size_t index = 0; index < simulations.simulations.size(); ++index)
        {
            const auto& simulation = simulations.simulations[index];
            auto& cSimulation = cSimulations[index];
            SilKit_Struct_Init(SilKit_Experimental_ManagementSimulation, cSimulation);
            cSimulation.name = simulation.name.c_str();
            cSimulation.participantNames = participantLists.emplace_back(simulation.participantNames).Get();
            cSimulation.requiredParticipantNames =
                participantLists.emplace_back(simulation.requiredParticipantNames).Get();
        }

        const CStringList created{change.created};
        const CStringList removed{change.removed};
        const CStringList changed{change.changed};

        SilKit_Experimental_ManagementSimulationsEvent cEvent;
        SilKit_Struct_Init(SilKit_Experimental_ManagementSimulationsEvent, cEvent);
        cEvent.sequence = simulations.sequence;
        cEvent.numSimulations = cSimulations.size();
        cEvent.simulations = cSimulations.data();
        cEvent.createdSimulationNames = created.Get();
        cEvent.removedSimulationNames = removed.Get();
        cEvent.changedSimulationNames = changed.Get();

        handler(context, client, &cEvent);
    });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS


SilKit_ReturnCode SilKitCALL SilKit_Experimental_ManagementClient_GetParticipantConfiguration(
    SilKit_Experimental_ManagementClient* client, const char* simulationName, const char* participantName,
    SilKit_NanosecondsTime timeout, void* context,
    SilKit_Experimental_ManagementParticipantConfigurationHandler_t handler)
try
{
    ASSERT_VALID_POINTER_PARAMETER(client);
    ASSERT_VALID_POINTER_PARAMETER(simulationName);
    ASSERT_VALID_POINTER_PARAMETER(participantName);
    ASSERT_VALID_HANDLER_PARAMETER(handler);

    ToCpp(client)->GetParticipantConfiguration(
        simulationName, participantName, std::chrono::nanoseconds{static_cast<std::chrono::nanoseconds::rep>(timeout)},
        [client, context, handler](SilKit::Services::Rpc::RpcCallStatus callStatus,
                                   const Mgmt::ParticipantConfigurationReply& reply) {
        const auto cCallStatus = static_cast<SilKit_RpcCallStatus>(callStatus);
        if (callStatus != SilKit::Services::Rpc::RpcCallStatus::Success)
        {
            handler(context, client, cCallStatus, nullptr);
            return;
        }

        SilKit_Experimental_ManagementParticipantConfigurationReply cReply;
        SilKit_Struct_Init(SilKit_Experimental_ManagementParticipantConfigurationReply, cReply);
        cReply.status = static_cast<SilKit_Experimental_ManagementParticipantConfigurationStatus>(reply.status);
        cReply.configurationJson = reply.configurationJson.c_str();

        handler(context, client, cCallStatus, &cReply);
    });
    return SilKit_ReturnCode_SUCCESS;
}
CAPI_CATCH_EXCEPTIONS
