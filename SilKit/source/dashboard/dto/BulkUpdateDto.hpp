// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "dashboard/dto/DataSpecDto.hpp"
#include "dashboard/dto/ParticipantStatusDto.hpp"
#include "dashboard/dto/RpcSpecDto.hpp"
#include "dashboard/dto/SystemStatusDto.hpp"

// NB: field declaration order below fixes the order of the emitted JSON keys, so do not reorder
// members without checking Test_DashboardJsonWriter.

namespace SilKit {
namespace Dashboard {

struct BulkSystemDto
{
    std::vector<SystemStatusDto> statuses;
};

struct BulkControllerDto
{
    uint64_t id{};
    std::string name;
    std::string networkName;
};

struct BulkDataServiceDto
{
    uint64_t id{};
    std::string name;
    std::string networkName;
    DataSpecDto spec;
};

struct BulkRpcServiceDto
{
    uint64_t id{};
    std::string name;
    std::string networkName;
    RpcSpecDto spec;
};

struct BulkServiceInternalDto
{
    uint64_t id{};
    std::string name;
    std::string networkName;
    uint64_t parentId{};
};

struct BulkParticipantDto
{
    std::string name;
    std::vector<ParticipantStatusDto> statuses;
    std::vector<BulkControllerDto> canControllers;
    std::vector<BulkControllerDto> ethernetControllers;
    std::vector<BulkControllerDto> flexrayControllers;
    std::vector<BulkControllerDto> linControllers;
    std::vector<BulkDataServiceDto> dataPublishers;
    std::vector<BulkDataServiceDto> dataSubscribers;
    std::vector<BulkServiceInternalDto> dataSubscriberInternals;
    std::vector<BulkRpcServiceDto> rpcClients;
    std::vector<BulkRpcServiceDto> rpcServers;
    std::vector<BulkServiceInternalDto> rpcServerInternals;
    std::vector<std::string> canNetworks;
    std::vector<std::string> ethernetNetworks;
    std::vector<std::string> flexrayNetworks;
    std::vector<std::string> linNetworks;
};

struct BulkSimulationDto
{
    //! Absent until the simulation stops; emitted as JSON null while unset.
    std::optional<int64_t> stopped;
    BulkSystemDto system;
    std::vector<BulkParticipantDto> participants;
};

} // namespace Dashboard
} // namespace SilKit
