// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "core/service/ServiceDatatypes.hpp"
#include "services/metrics/MetricsDatatypes.hpp"

namespace VSilKit {

struct SimulationStart
{
    std::string connectUri;
    uint64_t time;
};

struct ServiceData
{
    SilKit::Core::Discovery::ServiceDiscoveryEvent::Type discoveryType;
    SilKit::Core::ServiceDescriptor serviceDescriptor;
};

struct SimulationEnd
{
    uint64_t time;
};

using MetricsUpdatePair = std::pair<std::string, VSilKit::MetricsUpdate>;

//! Payload of a SilKitEvent. The alternatives are in the same order as SilKitEventType.
using SilKitEventData = std::variant<SimulationStart,                                                   //
                                     SilKit::Services::Orchestration::ParticipantConnectionInformation, //
                                     SilKit::Services::Orchestration::SystemState,                      //
                                     SilKit::Services::Orchestration::ParticipantStatus,                //
                                     ServiceData,                                                       //
                                     SimulationEnd,                                                     //
                                     MetricsUpdatePair>;

//! Discriminator for SilKitEventData. Enumerator values must match the variant alternative order;
//! the static_asserts below enforce that.
enum class SilKitEventType
{
    OnSimulationStart,
    OnParticipantConnected,
    OnSystemStateChanged,
    OnParticipantStatusChanged,
    OnServiceDiscoveryEvent,
    OnSimulationEnd,
    OnMetricUpdate,
};

namespace Detail {

template <SilKitEventType eventType, typename T>
constexpr auto EventTypeMatchesAlternative() -> bool
{
    return std::is_same<std::variant_alternative_t<static_cast<size_t>(eventType), SilKitEventData>, T>::value;
}

} // namespace Detail

static_assert(std::variant_size<SilKitEventData>::value == 7, "SilKitEventType and SilKitEventData disagree");
static_assert(Detail::EventTypeMatchesAlternative<SilKitEventType::OnSimulationStart, SimulationStart>(), "");
static_assert(Detail::EventTypeMatchesAlternative<SilKitEventType::OnParticipantConnected,
                                                  SilKit::Services::Orchestration::ParticipantConnectionInformation>(),
              "");
static_assert(Detail::EventTypeMatchesAlternative<SilKitEventType::OnSystemStateChanged,
                                                  SilKit::Services::Orchestration::SystemState>(),
              "");
static_assert(Detail::EventTypeMatchesAlternative<SilKitEventType::OnParticipantStatusChanged,
                                                  SilKit::Services::Orchestration::ParticipantStatus>(),
              "");
static_assert(Detail::EventTypeMatchesAlternative<SilKitEventType::OnServiceDiscoveryEvent, ServiceData>(), "");
static_assert(Detail::EventTypeMatchesAlternative<SilKitEventType::OnSimulationEnd, SimulationEnd>(), "");
static_assert(Detail::EventTypeMatchesAlternative<SilKitEventType::OnMetricUpdate, MetricsUpdatePair>(), "");

/*! One thing that happened in one simulation, queued for the dashboard worker thread.
 *
 *  Copyable and movable with the compiler-generated operations; the payload lives inline in the
 *  variant, so enqueueing an event costs no allocation beyond the payload's own.
 */
class SilKitEvent
{
public:
    SilKitEvent() = delete;

    template <typename T>
    explicit SilKitEvent(std::string simulationName, T&& value)
        : _simulationName{std::move(simulationName)}
        , _data{std::forward<T>(value)}
    {
    }

    auto Type() const -> SilKitEventType
    {
        return static_cast<SilKitEventType>(_data.index());
    }

    auto GetSimulationName() const -> const std::string&
    {
        return _simulationName;
    }

    auto Data() const -> const SilKitEventData&
    {
        return _data;
    }

    // Typed accessors. Each throws std::bad_variant_access if the event holds a different type.

    auto GetSimulationStart() const -> const SimulationStart&
    {
        return std::get<SimulationStart>(_data);
    }

    auto GetParticipantConnectionInformation() const
        -> const SilKit::Services::Orchestration::ParticipantConnectionInformation&
    {
        return std::get<SilKit::Services::Orchestration::ParticipantConnectionInformation>(_data);
    }

    auto GetParticipantStatus() const -> const SilKit::Services::Orchestration::ParticipantStatus&
    {
        return std::get<SilKit::Services::Orchestration::ParticipantStatus>(_data);
    }

    auto GetSystemState() const -> const SilKit::Services::Orchestration::SystemState&
    {
        return std::get<SilKit::Services::Orchestration::SystemState>(_data);
    }

    auto GetServiceData() const -> const ServiceData&
    {
        return std::get<ServiceData>(_data);
    }

    auto GetSimulationEnd() const -> const SimulationEnd&
    {
        return std::get<SimulationEnd>(_data);
    }

    auto GetMetricsUpdate() const -> const MetricsUpdatePair&
    {
        return std::get<MetricsUpdatePair>(_data);
    }

private:
    std::string _simulationName;
    SilKitEventData _data;
};

} // namespace VSilKit
