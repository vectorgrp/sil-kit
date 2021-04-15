// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "json11.hpp"
#include "Config.hpp"

namespace ib {
namespace cfg {

template <typename T>
struct is_std_vector : std::false_type {} ;
template <typename T>
struct is_std_vector<std::vector<T>> : std::true_type {} ;

template <typename T>
struct is_map : std::false_type {} ;
template <typename T>
struct is_map<std::map<std::string, T>> : std::true_type {} ;

template <typename T>
struct is_optional : std::false_type{};
template <typename T>
struct is_optional<OptionalCfg<T>> : std::true_type{};


template<typename T,
    std::enable_if_t<
        !is_optional<T>::value &&
        !is_map<T>::value &&
        !is_std_vector<T>::value,
    int> = 0
>
auto from_json(const json11::Json&) -> T {}
template <>
auto from_json<uint16_t>(const json11::Json& json) -> uint16_t;
template <>
auto from_json<int32_t>(const json11::Json& json) -> int32_t;
template <>
auto from_json<bool>(const json11::Json& json) -> bool;
template <>
auto from_json<std::string>(const json11::Json& json) -> std::string;
template <>
auto from_json<Version>(const json11::Json& json) -> Version;
template <>
auto from_json<Sink::Type>(const json11::Json& json) -> Sink::Type;
template <>
auto from_json<mw::logging::Level>(const json11::Json& json) -> mw::logging::Level;
template <>
auto from_json<Sink>(const json11::Json& json) -> Sink;
template <>
auto from_json<Logger>(const json11::Json& json) -> Logger;
template <>
auto from_json<CanController>(const json11::Json& json) -> CanController;
template <>
auto from_json<LinController>(const json11::Json& json) -> LinController;
template <>
auto from_json<EthernetController>(const json11::Json& json) -> EthernetController;
template <>
auto from_json<FlexrayController>(const json11::Json& json) -> FlexrayController;
template <>
auto from_json(const json11::Json& json) -> DigitalIoPort;
template <>
auto from_json(const json11::Json& json) -> AnalogIoPort;
template <>
auto from_json(const json11::Json& json) -> PwmPort;
template <>
auto from_json(const json11::Json& json) -> PatternPort;
template <>
auto from_json(const json11::Json& json) -> GenericPort;
template <>
auto from_json<SyncType>(const json11::Json& json) -> SyncType;
template<>
auto from_json<ParticipantController>(const json11::Json& json) -> ParticipantController;
template <>
auto from_json<Participant>(const json11::Json& json) -> Participant;
template <>
auto from_json<Switch::Port>(const json11::Json& json) -> Switch::Port;
template <>
auto from_json<Switch>(const json11::Json& json) -> Switch;
template <>
auto from_json<Link>(const json11::Json& json) -> Link;
template <>
auto from_json<NetworkSimulator>(const json11::Json& json) -> NetworkSimulator;
template <>
auto from_json<TimeSync::SyncPolicy>(const json11::Json& json) -> TimeSync::SyncPolicy;
template <>
auto from_json<TimeSync>(const json11::Json& json) -> TimeSync;
template <>
auto from_json<SimulationSetup>(const json11::Json& json) -> SimulationSetup;
template <>
auto from_json<FastRtps::DiscoveryType>(const json11::Json& json) -> FastRtps::DiscoveryType;
template <>
auto from_json<FastRtps::Config>(const json11::Json& json) -> FastRtps::Config;
template <>
auto from_json<VAsio::RegistryConfig>(const json11::Json& json) -> VAsio::RegistryConfig;
template <>
auto from_json<VAsio::Config>(const json11::Json& json) -> VAsio::Config;
template <>
auto from_json<MiddlewareConfig>(const json11::Json& json) -> MiddlewareConfig;
template <>
auto from_json<ExtensionConfig>(const json11::Json& json) -> ExtensionConfig;
template <>
auto from_json<Config>(const json11::Json& json) -> Config;
template <>
auto from_json<TraceSink>(const json11::Json& json) -> TraceSink;
template <>
auto from_json<TraceSink::Type>(const json11::Json& json) -> TraceSink::Type;
template <>
auto from_json<TraceSource>(const json11::Json& json) -> TraceSource;
template <>
auto from_json<TraceSource::Type>(const json11::Json& json) -> TraceSource::Type;
template <>
auto from_json<Replay>(const json11::Json& json) -> Replay;
template <>
auto from_json<Replay::Direction>(const json11::Json& json) -> Replay::Direction;
template <>
auto from_json<MdfChannel>(const json11::Json& json) -> MdfChannel;

auto to_json(uint16_t value) -> json11::Json;
auto to_json(int32_t value) -> json11::Json;
auto to_json(bool value) -> json11::Json;
auto to_json(const std::string& value) -> json11::Json;
auto to_json(const Version& version) -> json11::Json;
auto to_json(const Sink::Type& type) -> json11::Json;
auto to_json(const mw::logging::Level& level) -> json11::Json;
auto to_json(const Sink& sink) -> json11::Json;
auto to_json(const Logger& logger) -> json11::Json;
auto to_json(const CanController& controller) -> json11::Json;
auto to_json(const LinController& controller) -> json11::Json;
auto to_json(const EthernetController& controller) -> json11::Json;
auto to_json(const FlexrayController& controller) -> json11::Json;
auto to_json(const DigitalIoPort& port) -> json11::Json;
auto to_json(const AnalogIoPort& port) -> json11::Json;
auto to_json(const PwmPort& port) -> json11::Json;
auto to_json(const PatternPort& port) -> json11::Json;
auto to_json(const GenericPort& port) -> json11::Json;
auto to_json(SyncType syncType) -> json11::Json;
auto to_json(const Participant& participant) -> json11::Json;
auto to_json(const Switch::Port& switchPort) -> json11::Json;
auto to_json(const Switch& switch_) -> json11::Json;
auto to_json(const Link& link) -> json11::Json;
auto to_json(const NetworkSimulator& networkSimulator) -> json11::Json;
auto to_json(TimeSync::SyncPolicy syncPolicy) -> json11::Json;
auto to_json(const TimeSync& timeSync) -> json11::Json;
auto to_json(const SimulationSetup& simulationSetup) -> json11::Json;
auto to_json(FastRtps::DiscoveryType discoveryType) -> json11::Json;
auto to_json(const FastRtps::Config& fastRtps) -> json11::Json;
auto to_json(const VAsio::RegistryConfig& config) -> json11::Json;
auto to_json(const VAsio::Config& config) -> json11::Json;
auto to_json(const MiddlewareConfig& middlewareConfig) -> json11::Json;
auto to_json(const ExtensionConfig& config) -> json11::Json;
auto to_json(const Config& cfg) -> json11::Json;
auto to_json(const TraceSink& cfg) -> json11::Json;
auto to_json(const TraceSink::Type& type) -> json11::Json;
auto to_json(const TraceSource& cfg) -> json11::Json;
auto to_json(const TraceSource::Type& cfg) -> json11::Json;
auto to_json(const Replay& cfg) -> json11::Json;
auto to_json(const Replay::Direction& cfg) -> json11::Json;
auto to_json(const MdfChannel& cfg) -> json11::Json;


template<typename T, std::enable_if_t<is_std_vector<T>::value, int> = 0>
auto from_json(const json11::Json::array& array) -> T
{
    T vector;
    vector.reserve(array.size());
    std::transform(array.begin(), array.end(),
                   std::back_inserter(vector),
                   [](auto&& t) { return from_json<typename T::value_type>(t); });
    return vector;
}

template<typename T, std::enable_if_t<is_optional<T>::value, int> = 0>
auto from_json(const json11::Json& json) -> T
{
    if (json.is_null())
        return T{};
    else
        return T{from_json<typename T::value_type>(json)};
}

template<typename T, std::enable_if_t<is_map<T>::value, int> = 0>
auto from_json(const json11::Json& json) -> T
{
    T map;
    for (auto&& kv : json.object_items())
    {
        map[kv.first] = from_json<typename T::mapped_type>(kv.second);
    }

    return map;
}



template<typename T>
auto to_json(const std::vector<T>& vector) -> json11::Json::array;
template<typename T>
auto to_json(const std::map<std::string, T>& map) -> json11::Json::object;



} // namespace cfg
} // namespace ib
