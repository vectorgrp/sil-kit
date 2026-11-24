// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include "core/vasio/IRegistryEventListener.hpp"

namespace SilKit {
namespace Config {
inline namespace V1 {
struct ParticipantConfiguration;
} // namespace V1
} // namespace Config
} // namespace SilKit

namespace VSilKit {

//! True for simulation names reserved by SIL Kit (`.well-known/...`), e.g. the management simulation.
inline bool IsWellKnownSimulation(const std::string& simulationName)
{
    return simulationName.rfind(".well-known/", 0) == 0;
}

/*! The registry side of the management simulation: mirrors the registry events and offers them to clients via a
 *  participant in the management simulation.
 *
 *  Registry events arrive on the registry I/O thread (see IRegistryEventListener).
 */
struct IRegistryManagementParticipant : SilKit::Core::IRegistryEventListener
{
    //! Called before OnParticipantConnected if the participant shared its configuration.
    virtual void OnParticipantConfiguration(const std::string& simulationName, const std::string& participantName,
                                            const std::string& configurationJson) = 0;

    //! Joins the management simulation of the registry at \p registryUri. Blocks until joined, so it must not be
    //! called on the registry I/O thread. Errors are logged, the registry keeps running without management.
    virtual void Start(const std::string& registryUri) = 0;

    //! Leaves the management simulation. Registry events are still accepted afterwards.
    virtual void Stop() = 0;
};

auto CreateRegistryManagementParticipant(SilKit::Services::Logging::ILoggerInternal* logger,
                                         const SilKit::Config::ParticipantConfiguration& registryConfiguration)
    -> std::unique_ptr<IRegistryManagementParticipant>;

} // namespace VSilKit
