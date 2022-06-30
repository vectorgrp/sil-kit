// Copyright (c) Vector Informatik GmbH. All rights reserved.
//
#pragma once

#include <map>
#include <string>
#include <thread>
#include <functional>
#include <future>
#include <chrono>
#include <memory>
#include <cstdint>

#include "ib/IntegrationBus.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/vendor/IIbRegistry.hpp"

namespace ib { namespace test {
//forward
class SimSystemController;

////////////////////////////////////////
// SimParticipant
////////////////////////////////////////
class SimParticipant
{
public:
    using FutureResult = std::future<ib::mw::sync::ParticipantState>;

    SimParticipant(const SimParticipant&) = delete;
    SimParticipant operator=(const SimParticipant&) = delete;
    SimParticipant() = default;

    const std::string& Name() const;
    ib::mw::IParticipant* Participant() const;
    FutureResult& Result();
    void Stop();

private:
    std::string _name;
    FutureResult _result;
    std::unique_ptr<ib::mw::IParticipant> _participant;

    friend class SimTestHarness;
};


/// ! \brief SimTestHarness allows creating a synchronized simulation
///          with multiple participants. 
class SimTestHarness
{
public:
    //!< when deferParticipantCreation is true, SimParticipants will be created in the GetParticipant calls instead of in the constructor.
    SimTestHarness(const std::vector<std::string>& syncParticipantNames, const std::string& registryUri,
                   bool deferParticipantCreation = false);
    ~SimTestHarness();
    //! \brief Run the simulation, return false if timeout is reached.
    bool Run(std::chrono::nanoseconds testRunTimeout = std::chrono::nanoseconds::min());
    //! \brief Get the SimParticipant by name
    SimParticipant* GetParticipant(const std::string& participantName);

private:
    void AddParticipant(const std::string& participantName);

    std::vector<std::string> _syncParticipantNames;
    std::string _registryUri;
    std::unique_ptr<SimSystemController> _simSystemController;
    std::map<std::string, std::unique_ptr<SimParticipant>> _simParticipants;
    std::unique_ptr<ib::vendor::IIbRegistry> _registry;
};



} //end ns ib
} //end ns test
