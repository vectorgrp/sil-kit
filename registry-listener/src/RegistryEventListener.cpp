#include "RegistryEventListener.hpp"

#include <cstdio>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "silkit/services/orchestration/string_utils.hpp"

namespace {

template <typename Writer>
void Dump(const char* event, Writer write) noexcept
{
    try
    {
        std::ostringstream line;
        line << event;
        write(line);
        std::cout << line.str() << std::endl;
        if (!std::cout)
        {
            throw std::ios_base::failure{"stdout write failed"};
        }
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "registry-listener: cannot dump %s: %s\n", event, error.what());
        std::terminate();
    }
    catch (...)
    {
        std::fprintf(stderr, "registry-listener: unknown failure dumping %s\n", event);
        std::terminate();
    }
}

auto Context(std::ostream& out, const std::string& simulationName, const std::string& participantName) -> std::ostream&
{
    return out << " simulation=" << std::quoted(simulationName) << " participant=" << std::quoted(participantName);
}

} // namespace

void RegistryEventListener::OnLoggerCreated(SilKit::Services::Logging::ILoggerInternal* logger)
{
    Dump("OnLoggerCreated", [&](auto& out) { out << " logger=" << static_cast<const void*>(logger); });
}

void RegistryEventListener::OnRegistryUri(const std::string& registryUri)
{
    Dump("OnRegistryUri", [&](auto& out) { out << " uri=" << std::quoted(registryUri); });
}

void RegistryEventListener::OnGenerationCreated(const std::string& simulationName, uint64_t generation)
{
    Dump("OnGenerationCreated", [&](auto& out) {
        out << " simulation=" << std::quoted(simulationName) << " generation=" << generation;
    });
}

void RegistryEventListener::OnGenerationStateChanged(const std::string& simulationName, uint64_t generation,
                                                   SilKit::Services::Orchestration::SystemState state)
{
    Dump("OnGenerationStateChanged", [&](auto& out) {
        out << " simulation=" << std::quoted(simulationName) << " generation=" << generation
            << " state=" << SilKit::Services::Orchestration::to_string(state);
    });
}

void RegistryEventListener::OnGenerationRemoved(const std::string& simulationName, uint64_t generation)
{
    Dump("OnGenerationRemoved", [&](auto& out) {
        out << " simulation=" << std::quoted(simulationName) << " generation=" << generation;
    });
}

void RegistryEventListener::OnParticipantConnected(const std::string& simulationName, const std::string& participantName)
{
    Dump("OnParticipantConnected", [&](auto& out) { Context(out, simulationName, participantName); });
}

void RegistryEventListener::OnParticipantDisconnected(const std::string& simulationName, const std::string& participantName)
{
    Dump("OnParticipantDisconnected", [&](auto& out) { Context(out, simulationName, participantName); });
}

void RegistryEventListener::OnRequiredParticipantsUpdate(
    const std::string& simulationName, const std::string& participantName,
    SilKit::Util::Span<const std::string> requiredParticipantNames)
{
    Dump("OnRequiredParticipantsUpdate", [&](auto& out) {
        Context(out, simulationName, participantName) << " required=[";
        const char* separator = "";
        for (const auto& name : requiredParticipantNames)
        {
            out << separator << std::quoted(name);
            separator = ", ";
        }
        out << ']';
    });
}

void RegistryEventListener::OnParticipantStatusUpdate(
    const std::string& simulationName, const std::string& participantName,
    const SilKit::Services::Orchestration::ParticipantStatus& participantStatus)
{
    Dump("OnParticipantStatusUpdate", [&](auto& out) {
        Context(out, simulationName, participantName) << " status=" << participantStatus;
    });
}

void RegistryEventListener::OnServiceDiscoveryEvent(
    const std::string& simulationName, const std::string& participantName,
    const SilKit::Core::Discovery::ServiceDiscoveryEvent& serviceDiscoveryEvent)
{
    Dump("OnServiceDiscoveryEvent", [&](auto& out) {
        Context(out, simulationName, participantName)
            << " event=" << serviceDiscoveryEvent
            << " serviceId=" << serviceDiscoveryEvent.serviceDescriptor.GetServiceId() << " supplemental={";
        const char* separator = "";
        for (const auto& item : serviceDiscoveryEvent.serviceDescriptor.GetSupplementalDataRef())
        {
            out << separator << std::quoted(item.first) << ": " << std::quoted(item.second);
            separator = ", ";
        }
        out << '}';
    });
}

void RegistryEventListener::OnMetricsUpdate(const std::string& simulationName, const std::string& origin,
                                          const VSilKit::MetricsUpdate& metricsUpdate)
{
    Dump("OnMetricsUpdate", [&](auto& out) {
        out << " simulation=" << std::quoted(simulationName) << " origin=" << std::quoted(origin)
            << " metrics=" << metricsUpdate;
    });
}
