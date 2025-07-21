// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

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
