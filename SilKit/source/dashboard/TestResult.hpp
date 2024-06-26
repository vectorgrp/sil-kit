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