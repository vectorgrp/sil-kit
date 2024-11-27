// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>

#include "silkit/services/all.hpp"

namespace SilKit {
namespace Dashboard {

struct Spec
{
    std::string topic;
    std::string functionName;
    std::string mediaType;
    std::vector<SilKit::Services::MatchingLabel> labels;
};

struct Service
{
    std::string parentServiceId;
    std::string serviceType;
    std::string serviceName;
    std::string networkName;
    Spec spec;
};

struct Link
{
    std::string type;
    std::string name;
    bool operator<(const Link& rhs) const
    {
        return type < rhs.type || (type == rhs.type && name < rhs.name);
    }
};

struct SimulationData
{
    std::set<std::string> participants;
    std::map<std::string, std::set<std::string>> statesByParticipant;
    std::map<std::string, std::map<uint64_t, Service>> servicesByParticipant;
    std::map<std::string, std::set<Link>> linksByParticipant;
    std::set<std::string> systemStates;
    size_t metricCount{};
    bool stopped{false};
};

struct TestResult
{
    int32_t errorStatus{-1};
    std::string errorMessage{};
    int64_t objectCount{-1};
    std::map<uint64_t, SimulationData> dataBySimulation;
    bool allSimulationsFinished{true};
};

} // namespace Dashboard
} // namespace SilKit