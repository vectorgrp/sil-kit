// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "RpcSpecDto.hpp"
#include "DataSpecDto.hpp"
#include "ParticipantStatusDto.hpp"
#include "SystemStatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

class BulkSystemDto : public oatpp::DTO
{
    DTO_INIT(BulkSystemDto, DTO)

    DTO_FIELD(Vector<Object<SystemStatusDto>>, statuses);

    static auto CreateEmpty() -> Wrapper
    {
        auto dto = createShared();
        dto->statuses = Vector<Object<SystemStatusDto>>::createShared();
        return dto;
    }
};

class BulkControllerDto : public oatpp::DTO
{
    DTO_INIT(BulkControllerDto, DTO)

    DTO_FIELD(UInt64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, networkName);
};

class BulkDataServiceDto : public oatpp::DTO
{
    DTO_INIT(BulkDataServiceDto, DTO)

    DTO_FIELD(UInt64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, networkName);
    DTO_FIELD(Object<DataSpecDto>, spec);
};

class BulkRpcServiceDto : public oatpp::DTO
{
    DTO_INIT(BulkRpcServiceDto, DTO)

    DTO_FIELD(UInt64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, networkName);
    DTO_FIELD(Object<RpcSpecDto>, spec);
};

class BulkServiceInternalDto : public oatpp::DTO
{
    DTO_INIT(BulkServiceInternalDto, DTO)

    DTO_FIELD(UInt64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, networkName);
    DTO_FIELD(UInt64, parentId);
};

class BulkParticipantDto : public oatpp::DTO
{
    DTO_INIT(BulkParticipantDto, DTO)

    DTO_FIELD(String, name);
    DTO_FIELD(Vector<Object<ParticipantStatusDto>>, statuses);
    DTO_FIELD(Vector<Object<BulkControllerDto>>, canControllers);
    DTO_FIELD(Vector<Object<BulkControllerDto>>, ethernetControllers);
    DTO_FIELD(Vector<Object<BulkControllerDto>>, flexrayControllers);
    DTO_FIELD(Vector<Object<BulkControllerDto>>, linControllers);
    DTO_FIELD(Vector<Object<BulkDataServiceDto>>, dataPublishers);
    DTO_FIELD(Vector<Object<BulkDataServiceDto>>, dataSubscribers);
    DTO_FIELD(Vector<Object<BulkServiceInternalDto>>, dataSubscriberInternals);
    DTO_FIELD(Vector<Object<BulkRpcServiceDto>>, rpcClients);
    DTO_FIELD(Vector<Object<BulkRpcServiceDto>>, rpcServers);
    DTO_FIELD(Vector<Object<BulkServiceInternalDto>>, rpcServerInternals);
    DTO_FIELD(Vector<String>, canNetworks);
    DTO_FIELD(Vector<String>, ethernetNetworks);
    DTO_FIELD(Vector<String>, flexrayNetworks);
    DTO_FIELD(Vector<String>, linNetworks);

    static auto CreateEmpty() -> Wrapper
    {
        auto dto = createShared();
        dto->statuses = Vector<Object<ParticipantStatusDto>>::createShared();
        dto->canControllers = Vector<Object<BulkControllerDto>>::createShared();
        dto->ethernetControllers = Vector<Object<BulkControllerDto>>::createShared();
        dto->flexrayControllers = Vector<Object<BulkControllerDto>>::createShared();
        dto->linControllers = Vector<Object<BulkControllerDto>>::createShared();
        dto->dataPublishers = Vector<Object<BulkDataServiceDto>>::createShared();
        dto->dataSubscribers = Vector<Object<BulkDataServiceDto>>::createShared();
        dto->dataSubscriberInternals = Vector<Object<BulkServiceInternalDto>>::createShared();
        dto->rpcClients = Vector<Object<BulkRpcServiceDto>>::createShared();
        dto->rpcServers = Vector<Object<BulkRpcServiceDto>>::createShared();
        dto->rpcServerInternals = Vector<Object<BulkServiceInternalDto>>::createShared();
        dto->canNetworks = Vector<String>::createShared();
        dto->ethernetNetworks = Vector<String>::createShared();
        dto->flexrayNetworks = Vector<String>::createShared();
        dto->linNetworks = Vector<String>::createShared();
        return dto;
    }
};

class BulkSimulationDto : public oatpp::DTO
{
    DTO_INIT(BulkSimulationDto, DTO)

    DTO_FIELD(Int64, stopped);
    DTO_FIELD(Object<BulkSystemDto>, system);
    DTO_FIELD(Vector<Object<BulkParticipantDto>>, participants);

    static auto CreateEmpty() -> Wrapper
    {
        auto dto = createShared();
        dto->system = BulkSystemDto::CreateEmpty();
        dto->participants = Vector<Object<BulkParticipantDto>>::createShared();
        return dto;
    }
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)
