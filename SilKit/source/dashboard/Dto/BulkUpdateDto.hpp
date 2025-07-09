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

    DTO_FIELD(Vector<Object<SystemStatusDto>>, statuses) = Vector<Object<SystemStatusDto>>::createShared();

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
    DTO_FIELD(Object<DataSpecDto>, spec) = Object<DataSpecDto>::createShared();
};

class BulkRpcServiceDto : public oatpp::DTO
{
    DTO_INIT(BulkRpcServiceDto, DTO)

    DTO_FIELD(UInt64, id);
    DTO_FIELD(String, name);
    DTO_FIELD(String, networkName);
    DTO_FIELD(Object<RpcSpecDto>, spec) = Object<RpcSpecDto>::createShared();
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
    DTO_FIELD(Vector<Object<ParticipantStatusDto>>, statuses) = Vector<Object<ParticipantStatusDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkControllerDto>>, canControllers) = Vector<Object<BulkControllerDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkControllerDto>>,
              ethernetControllers) = Vector<Object<BulkControllerDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkControllerDto>>,
              flexrayControllers) = Vector<Object<BulkControllerDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkControllerDto>>, linControllers) = Vector<Object<BulkControllerDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkDataServiceDto>>, dataPublishers) = Vector<Object<BulkDataServiceDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkDataServiceDto>>, dataSubscribers) = Vector<Object<BulkDataServiceDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkServiceInternalDto>>,
              dataSubscriberInternals) = Vector<Object<BulkServiceInternalDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkRpcServiceDto>>, rpcClients) = Vector<Object<BulkRpcServiceDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkRpcServiceDto>>, rpcServers) = Vector<Object<BulkRpcServiceDto>>::createShared();
    DTO_FIELD(Vector<Object<BulkServiceInternalDto>>,
              rpcServerInternals) = Vector<Object<BulkServiceInternalDto>>::createShared();
    DTO_FIELD(Vector<String>, canNetworks) = Vector<String>::createShared();
    DTO_FIELD(Vector<String>, ethernetNetworks) = Vector<String>::createShared();
    DTO_FIELD(Vector<String>, flexrayNetworks) = Vector<String>::createShared();
    DTO_FIELD(Vector<String>, linNetworks) = Vector<String>::createShared();

};

class BulkSimulationDto : public oatpp::DTO
{
    DTO_INIT(BulkSimulationDto, DTO)

    DTO_FIELD(Int64, stopped);
    DTO_FIELD(Object<BulkSystemDto>, system) = Object<BulkSystemDto>::createShared();
    DTO_FIELD(Vector<Object<BulkParticipantDto>>, participants) = Vector<Object<BulkParticipantDto>>::createShared();
};

class MetricDataDto : public oatpp::DTO
{
    DTO_INIT(MetricDataDto, DTO)

    DTO_FIELD(Int64, ts);
    DTO_FIELD(String, pn);
    DTO_FIELD(Vector<String>, mn) = Vector<String>::createShared();
};

class AttributeDataDto : public MetricDataDto
{
    DTO_INIT(AttributeDataDto, MetricDataDto)

    DTO_FIELD(String, mv);
};

class CounterDataDto : public MetricDataDto
{
    DTO_INIT(CounterDataDto, MetricDataDto)

    DTO_FIELD(Int64, mv);
};

class StatisticDataDto : public MetricDataDto
{
    DTO_INIT(StatisticDataDto, MetricDataDto)

    DTO_FIELD(Vector<Float64>, mv) = Vector<Float64>::createShared();
};


class MetricsUpdateDto : public oatpp::DTO
{
    DTO_INIT(MetricsUpdateDto, oatpp::DTO)

    DTO_FIELD(Vector<Object<AttributeDataDto>>, attributes) = Vector<Object<AttributeDataDto>>::createShared();
    DTO_FIELD(Vector<Object<CounterDataDto>>, counters) = Vector<Object<CounterDataDto>>::createShared();
    DTO_FIELD(Vector<Object<StatisticDataDto>>, statistics) = Vector<Object<StatisticDataDto>>::createShared();
};
} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)
