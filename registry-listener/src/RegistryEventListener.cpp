#include "RegistryEventListener.hpp"

#include <chrono>
#include <cstdio>
#include <exception>
#include <iostream>
#include <locale>
#include <sstream>
#include <string_view>

#include "silkit/services/orchestration/string_utils.hpp"

namespace {

// The shared string escape helper does not cover every C0 control byte.
struct JsonString
{
    std::string_view value;

    friend auto operator<<(std::ostream& out, JsonString string) -> std::ostream&
    {
        constexpr char hex[] = "0123456789abcdef";
        out.put('"');
        for (const unsigned char ch : string.value)
        {
            if (ch == '"' || ch == '\\')
            {
                out.put('\\');
                out.put(static_cast<char>(ch));
            }
            else if (ch < 0x20)
            {
                out << "\\u00" << hex[ch >> 4] << hex[ch & 0x0f];
            }
            else
            {
                out.put(static_cast<char>(ch));
            }
        }
        return out.put('"');
    }
};

template <typename Writer>
void Dump(const char* event, Writer write) noexcept
{
    try
    {
        std::ostringstream line;
        line.imbue(std::locale::classic());
        line << R"({"source":"silkit_registry","version":1,"type":)" << JsonString{event};
        write(line);
        line << "}\n";
        std::cout << line.str() << std::flush;
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
    return out << R"(,"simulation":)" << JsonString{simulationName} << R"(,"name":)" << JsonString{participantName};
}

auto Generation(std::ostream& out, std::optional<uint64_t> generation) -> std::ostream&
{
    out << R"(,"generation":)";
    if (generation.has_value())
    {
        return out << *generation;
    }
    return out << "null";
}

} // namespace

void RegistryEventListener::OnLoggerCreated(SilKit::Services::Logging::ILoggerInternal*)
{
    Dump("logger_created", [](auto&) {});
}

void RegistryEventListener::OnRegistryUri(const std::string& registryUri)
{
    Dump("registry_uri", [&](auto& out) { out << R"(,"uri":)" << JsonString{registryUri}; });
}

void RegistryEventListener::OnGenerationCreated(const std::string& simulationName, uint64_t generation)
{
    Dump("generation_created", [&](auto& out) {
        out << R"(,"simulation":)" << JsonString{simulationName} << R"(,"generation":)" << generation;
    });
}

void RegistryEventListener::OnGenerationStateChanged(const std::string& simulationName, uint64_t generation,
                                                   SilKit::Services::Orchestration::SystemState state)
{
    Dump("generation_state_changed", [&](auto& out) {
        out << R"(,"simulation":)" << JsonString{simulationName} << R"(,"generation":)" << generation
            << R"(,"state":)" << JsonString{SilKit::Services::Orchestration::to_string(state)};
    });
}

void RegistryEventListener::OnGenerationRemoved(const std::string& simulationName, uint64_t generation)
{
    Dump("generation_removed", [&](auto& out) {
        out << R"(,"simulation":)" << JsonString{simulationName} << R"(,"generation":)" << generation;
    });
}

void RegistryEventListener::OnParticipantConnected(const std::string& simulationName, const std::string& participantName,
                                                  std::optional<uint64_t> generation)
{
    Dump("participant_connected", [&](auto& out) { Generation(Context(out, simulationName, participantName), generation); });
}

void RegistryEventListener::OnParticipantDisconnected(const std::string& simulationName, const std::string& participantName,
                                                     std::optional<uint64_t> generation)
{
    Dump("participant_disconnected", [&](auto& out) { Generation(Context(out, simulationName, participantName), generation); });
}

void RegistryEventListener::OnRequiredParticipantsUpdate(
    const std::string& simulationName, const std::string& participantName,
    SilKit::Util::Span<const std::string> requiredParticipantNames, std::optional<uint64_t> generation)
{
    Dump("required_participants", [&](auto& out) {
        Generation(Context(out, simulationName, participantName), generation) << R"(,"required":[)";
        const char* separator = "";
        for (const auto& name : requiredParticipantNames)
        {
            out << separator << JsonString{name};
            separator = ",";
        }
        out << ']';
    });
}

void RegistryEventListener::OnParticipantStatusUpdate(
    const std::string& simulationName, const std::string& participantName,
    const SilKit::Services::Orchestration::ParticipantStatus& participantStatus, std::optional<uint64_t> generation)
{
    Dump("participant_status", [&](auto& out) {
        Generation(Context(out, simulationName, participantName), generation)
            << R"(,"state":)" << JsonString{SilKit::Services::Orchestration::to_string(participantStatus.state)}
            << R"(,"enter_reason":)" << JsonString{participantStatus.enterReason}
            << R"(,"enter_time_ns":)"
            << std::chrono::duration_cast<std::chrono::nanoseconds>(participantStatus.enterTime.time_since_epoch()).count()
            << R"(,"refresh_time_ns":)"
            << std::chrono::duration_cast<std::chrono::nanoseconds>(participantStatus.refreshTime.time_since_epoch()).count();
    });
}

void RegistryEventListener::OnServiceDiscoveryEvent(
    const std::string& simulationName, const std::string& participantName,
    const SilKit::Core::Discovery::ServiceDiscoveryEvent& serviceDiscoveryEvent)
{
    Dump("service_discovery", [&](auto& out) {
        const auto& service = serviceDiscoveryEvent.serviceDescriptor;
        Context(out, simulationName, participantName)
            << R"(,"event":)" << static_cast<int>(serviceDiscoveryEvent.type)
            << R"(,"service":{"id":)" << service.GetServiceId()
            << R"(,"name":)" << JsonString{service.GetServiceName()}
            << R"(,"participant_name":)" << JsonString{service.GetParticipantName()}
            << R"(,"participant_id":)" << service.GetParticipantId()
            << R"(,"type":)" << static_cast<int>(service.GetServiceType())
            << R"(,"network_name":)" << JsonString{service.GetNetworkName()}
            << R"(,"network_type":)" << static_cast<int>(service.GetNetworkType())
            << R"(,"supplemental":{)";
        const char* separator = "";
        for (const auto& item : service.GetSupplementalDataRef())
        {
            out << separator << JsonString{item.first} << ':' << JsonString{item.second};
            separator = ",";
        }
        out << "}}";
    });
}

void RegistryEventListener::OnMetricsUpdate(const std::string& simulationName, const std::string& origin,
                                          const VSilKit::MetricsUpdate& metricsUpdate)
{
    Dump("metrics", [&](auto& out) {
        out << R"(,"simulation":)" << JsonString{simulationName} << R"(,"origin":)" << JsonString{origin}
            << R"(,"metrics":[)";
        const char* separator = "";
        for (const auto& metric : metricsUpdate.metrics)
        {
            out << separator << R"({"timestamp_ns":)" << metric.timestamp << R"(,"name":)" << JsonString{metric.name}
                << R"(,"kind":)" << static_cast<int>(metric.kind) << R"(,"value":)" << JsonString{metric.value} << '}';
            separator = ",";
        }
        out << ']';
    });
}
