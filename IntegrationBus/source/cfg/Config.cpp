#include "Config.hpp"
//#include "JsonConfig.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>

#include "tuple_tools/for_each.hpp"
#include "ib/cfg/string_utils.hpp"

#include "YamlConfig.hpp"

namespace ib {
namespace cfg {


namespace tt = ib::util::tuple_tools;

// Put helper functions in anonymous namespace
namespace {

template<typename ParticipantT>
constexpr auto Endpoints(ParticipantT&& participant)
{
    return std::forward_as_tuple(
        participant.canControllers,
        participant.linControllers,
        participant.ethernetControllers,
        participant.flexrayControllers,
        participant.digitalIoPorts,
        participant.analogIoPorts,
        participant.pwmPorts,
        participant.patternPorts,
        participant.genericPublishers,
        participant.genericSubscribers
    );
}


auto MakeQualifiedName(const std::string& participantName, const std::string& controllerName)
{
    std::stringstream qualifiedName;
    qualifiedName << participantName << "/" << controllerName;
    return qualifiedName.str();
}

auto SplitEndpointName(const std::string& qualifiedName) -> std::pair<std::string, std::string>
{
    auto pos = qualifiedName.find('/');
    if (pos == std::string::npos)
    {
        std::cerr << "IB Misconfiguration: " << qualifiedName << " does not contain a '/'" << std::endl;
        throw Misconfiguration("Invalid qualified name!");
    }
    else
    {
        return std::make_pair(
            qualifiedName.substr(0, pos),
            qualifiedName.substr(pos + 1)
        );
    }
}

inline auto ResolveLinkType(Link::Type lhs, Link::Type rhs) -> Link::Type
{
    if (lhs == rhs)
        return lhs;

    if (lhs == Link::Type::Undefined)
        return rhs;

    if (rhs == Link::Type::Undefined)
        return lhs;

    return Link::Type::Invalid;
}

template<class EndpointRange>
auto GetEndpointLinkType(EndpointRange&& endpoints, const std::string& endpointName)
{
    auto&& endpointIter = find_by_name(endpoints, endpointName);
    if (endpointIter != endpoints.end())
        return endpointIter->linkType;
    else
        return Link::Type::Undefined;
}



auto GetLinkType(const Config& config, const std::string& qualifiedEndpoint) -> Link::Type
{
    std::string participantName;
    std::string endpointName;
    std::tie(participantName, endpointName) = SplitEndpointName(qualifiedEndpoint);

    auto&& participantIter = find_by_name(config.simulationSetup.participants, participantName);
    if (participantIter != config.simulationSetup.participants.end())
    {
        // The Endpoint refers to a Participant -> Determine LinkType by its controllers
        auto&& participant = *participantIter;

        Link::Type linkType{Link::Type::Undefined};

        tt::for_each(Endpoints(participant),
            [&linkType, &endpointName](auto&& endpoints)
            {
                linkType = ResolveLinkType(linkType, GetEndpointLinkType(endpoints, endpointName));
            }
        );

        return linkType;
    }

    auto&& switchIter = find_by_name(config.simulationSetup.switches, participantName);
    if (switchIter != config.simulationSetup.switches.end())
    {
        // Endpoint refers to a Switch -> Try to find the corresponding Port
        auto&& ethSwitch = *switchIter;
        return GetEndpointLinkType(ethSwitch.ports, endpointName);
    }

    throw Misconfiguration("Unknown Endpoint");
}

void AssignEndpointAddresses(Config& config)
{
    // assign ParticipantIds and EndpointIds according to the order in which participants
    // and controllers occur in the config.
    uint16_t participantId = 1;
    uint16_t endpointId = 1;

    auto&& assignEndpointIds =
        [&endpointId](auto&& controllers)
        {
            for (auto&& controller : controllers)
                controller.endpointId = endpointId++;
        };

    for (auto&& participant : config.simulationSetup.participants)
    {
        participant.id = participantId++;
        tt::for_each(Endpoints(participant),
            assignEndpointIds
        );
    }
    for (auto&& ethSwitch : config.simulationSetup.switches)
    {
        assignEndpointIds(ethSwitch.ports);
    }
}


void AssignLinkIds(Config& config)
{
    // First, assign LinkIds to Links and setup a map for easy link lookup.
    // Then, set LinkIds of Controllers

    std::unordered_set<std::string> linkSet; // Only used to check uniqueness
    std::unordered_map<std::string, Link*> endpointToLink; // Used to later assign linkIds to controllers

    int16_t linkId = 0;
    for (auto&& link : config.simulationSetup.links)
    {
        link.id = linkId++;
        link.type = Link::Type::Undefined;

        // generate a set of link names to ensure uniquenes
        if (linkSet.find(link.name) != linkSet.end())
        {
            std::cerr << "Invalid IB Config: link name \"" << link.name << "\" used twice!" << std::endl;
            throw Misconfiguration("Duplicate link name");
        }
        linkSet.insert(link.name);


        if (link.endpoints.size() < 2)
        {
            std::cerr << "WARNING: Link " << link.name << " does not connect anything" << std::endl;
        }

        // Lookup LinkType of each endpoint and derive Type for Link accordingly.
        for (auto&& endpoint : link.endpoints)
        {
            try
            {
                auto linkType = GetLinkType(config, endpoint);
                link.type = ResolveLinkType(link.type, linkType);
            }
            catch (Misconfiguration& e)
            {
                std::cerr << "Invaild IB Config: endpoint " << endpoint << " at link " << link.name << " does not exist!" << std::endl;
                throw e;
            }

            
            if (link.type == Link::Type::Invalid)
            {
                std::cerr << "Invalid IB Config: link \"" << link.name << "\" has different controller types assigned!" << std::endl;
                throw Misconfiguration("Invalid link type!");
            }

            endpointToLink[endpoint] = &link;
        }
    }


    for (auto&& participant : config.simulationSetup.participants)
    {
        tt::for_each(Endpoints(participant),
            [&endpointToLink, &participant](auto&& endpoints)
            {
                for (auto&& endpoint : endpoints)
                {
                    auto qualifiedName = MakeQualifiedName(participant.name, endpoint.name);
                    auto&& linkIter = endpointToLink.find(qualifiedName);
                    if (linkIter == endpointToLink.end())
                    {
                        std::cerr << "WARNING: unconnected Controller " << qualifiedName << "\n";
                        continue;
                    }
                    auto&& link = *linkIter->second;
                    endpoint.linkId = link.id;
                }
            }
        );
    }
}

void AdjustLegacyPcapConfig(Config& config)
{
    auto addReplacementSink = [&](auto& participant, auto& ethCtrl, TraceSink::Type type, auto& path)
    {
        //turn the legacy pcapFile/pcapPipe fields into a controller specific trace sink
        TraceSink legacySink{};
        legacySink.type = type;
        // this name is unlikely to collide with a user supplied name of a different sink
        legacySink.name = ethCtrl.name + ":<Legacy" + to_string(type) + ">";
        legacySink.outputPath = path;

        std::cerr << "WARNING: turning deprecated EthernetController field "
            << to_string(type)
            << " on "
            << participant.name << "/" << ethCtrl.name
            << " to the newly created TraceSink: " << legacySink.name
            << "\n";

        ethCtrl.useTraceSinks.emplace_back(legacySink.name);
        participant.traceSinks.emplace_back(std::move(legacySink));
    };

    for (auto& participant : config.simulationSetup.participants)
    {
        for (auto& ethCtrl : participant.ethernetControllers)
        {
            if (!ethCtrl.pcapFile.empty())
            {
                addReplacementSink(participant, ethCtrl, TraceSink::Type::PcapFile, ethCtrl.pcapFile);
            }
            if (!ethCtrl.pcapPipe.empty())
            {
                addReplacementSink(participant, ethCtrl, TraceSink::Type::PcapPipe, ethCtrl.pcapPipe);
            }
        }
    }
}

auto ReadWholeFile(const std::string& filename) -> std::string
{
    std::ifstream fs(filename);

    if (!fs.is_open())
        throw Misconfiguration("Invalid IB config filename '" + filename + "'");

    std::stringstream buffer;
    buffer << fs.rdbuf();

    return buffer.str();

}

} // anonymous namespace

void PostProcess(Config& config)
{
    // Post-processing steps
    AssignEndpointAddresses(config);
    AssignLinkIds(config);
    UpdateGenericSubscribers(config);
    AdjustLegacyPcapConfig(config);
}

void UpdateGenericSubscribers(Config& config)
{
    std::unordered_map<int16_t, ib::cfg::GenericPort*> genericPublisherPerLink;
    std::unordered_map<int16_t, std::vector<ib::cfg::GenericPort*>> genericSubscribersPerLink;

    for (auto&& participant : config.simulationSetup.participants)
    {
        for (auto&& genericPublisher : participant.genericPublishers)
        {
            auto&& iter = genericPublisherPerLink.find(genericPublisher.linkId);
            if (iter != genericPublisherPerLink.end())
            {
                auto&& links = config.simulationSetup.links;
                auto&& linkIter = std::find_if(links.begin(), links.end(),
                    [&](const Link& link) { return link.id == genericPublisher.linkId; });
                if (linkIter == links.end())
                {
                    throw Misconfiguration("GenericPublisher \"" + genericPublisher.name
                        + "\" on participant \"" + participant.name 
                        + "\" refers to invalid LinkId=" + std::to_string(genericPublisher.linkId));
                }
                std::cerr << "WARNING: more than one GenericPublisher on link " << linkIter->name << "\n";

                continue;
            }

            if (genericPublisher.linkId < 0)
            {
                throw Misconfiguration("GenericPublisher \"" + genericPublisher.name 
                    + "\" on Participant \"" + participant.name + "\" has no Link specified");
            }

            genericPublisherPerLink[genericPublisher.linkId] = &genericPublisher;
        }
        for (auto&& genericSubscriber : participant.genericSubscribers)
        {
            if (genericSubscriber.linkId < 0)
            {
                throw Misconfiguration("GenericSubscriber \"" + genericSubscriber.name
                    + "\" on participant \"" + participant.name 
                    + "\" refers to invalid LinkId=" + std::to_string(genericSubscriber.linkId));
            }
            genericSubscribersPerLink[genericSubscriber.linkId].push_back(&genericSubscriber);
        }
    }

    // For each publisher, propagate the configured data to linked subscribers
    for (auto&& entry : genericPublisherPerLink)
    {
        auto* genericPublisher = entry.second;

        auto&& iter = genericSubscribersPerLink.find(entry.first);
        if (iter == genericSubscribersPerLink.end())
        {
            continue;
        }

        auto&& genericSubscribers = iter->second;
        for (auto* genericSubscriber : genericSubscribers)
        {
            genericSubscriber->definitionUri = genericPublisher->definitionUri;
            genericSubscriber->protocolType = genericPublisher->protocolType;
        }
    }
}

bool operator==(const Version& lhs, const Version& rhs)
{
    return lhs.major == rhs.major
        && lhs.minor == rhs.minor
        && lhs.patchLevel == rhs.patchLevel;
}

bool operator==(const Participant& lhs, const Participant& rhs)
{
    return lhs.name == rhs.name
        && lhs.id == rhs.id
        && lhs.logger == rhs.logger
        && lhs.participantController == rhs.participantController
        && lhs.canControllers == rhs.canControllers
        && lhs.linControllers == rhs.linControllers
        && lhs.ethernetControllers == rhs.ethernetControllers
        && lhs.flexrayControllers == rhs.flexrayControllers
        && lhs.networkSimulators == rhs.networkSimulators
        && lhs.digitalIoPorts == rhs.digitalIoPorts
        && lhs.analogIoPorts == rhs.analogIoPorts
        && lhs.pwmPorts == rhs.pwmPorts
        && lhs.patternPorts == rhs.patternPorts
        && lhs.genericPublishers == rhs.genericPublishers
        && lhs.genericSubscribers == rhs.genericSubscribers
        && lhs.isSyncMaster == rhs.isSyncMaster
        && lhs.traceSinks == rhs.traceSinks
        && lhs.traceSources == rhs.traceSources
        ;
}

bool operator==(const Link& lhs, const Link& rhs)
{
    return lhs.name == rhs.name
        && lhs.id == rhs.id
        && lhs.type == rhs.type
        && lhs.endpoints == rhs.endpoints;
}

bool operator==(const NetworkSimulator& lhs, const NetworkSimulator& rhs)
{
    return lhs.name == rhs.name
        && lhs.simulatedLinks == rhs.simulatedLinks
        && lhs.simulatedSwitches == rhs.simulatedSwitches
        && lhs.useTraceSinks == rhs.useTraceSinks
        ;
}

bool operator==(const Switch::Port& lhs, const Switch::Port& rhs)
{
    return lhs.name == rhs.name
        && lhs.endpointId == rhs.endpointId
        && lhs.vlanIds == rhs.vlanIds;
}

bool operator==(const Switch& lhs, const Switch& rhs)
{
    return lhs.name == rhs.name
        && lhs.description == rhs.description
        && lhs.ports == rhs.ports;
}

bool operator==(const TimeSync& lhs, const TimeSync& rhs)
{
    return lhs.tickPeriod == rhs.tickPeriod;
}

bool operator==(const SimulationSetup& lhs, const SimulationSetup& rhs)
{
    return lhs.participants == rhs.participants
        && lhs.links == rhs.links
        && lhs.switches == rhs.switches
        && lhs.timeSync == rhs.timeSync;
}

bool operator==(const FastRtps::Config &lhs, const FastRtps::Config& rhs)
{
    return lhs.discoveryType == rhs.discoveryType
        && lhs.unicastLocators == rhs.unicastLocators
        && lhs.configFileName == rhs.configFileName
        && lhs.sendSocketBufferSize == rhs.sendSocketBufferSize
        && lhs.listenSocketBufferSize == rhs.listenSocketBufferSize
        && lhs.historyDepth == rhs.historyDepth;
}

bool operator==(const VAsio::RegistryConfig &lhs, const VAsio::RegistryConfig &rhs)
{
    return lhs.port == rhs.port
        && lhs.hostname == rhs.hostname
        && lhs.logger == rhs.logger;
}

bool operator==(const VAsio::Config &lhs, const VAsio::Config &rhs)
{
    return lhs.registry == rhs.registry;
}

bool operator==(const MiddlewareConfig &lhs, const MiddlewareConfig& rhs)
{
    return lhs.activeMiddleware == rhs.activeMiddleware
        && lhs.fastRtps == rhs.fastRtps
        && lhs.vasio == rhs.vasio;
}

bool operator==(const ExtensionConfig &lhs, const ExtensionConfig& rhs)
{
    return lhs.searchPathHints == rhs.searchPathHints;
}

bool operator==(const Config& lhs, const Config& rhs)
{
    return lhs.version == rhs.version
        && lhs.name == rhs.name
        && lhs.simulationSetup == rhs.simulationSetup
        && lhs.middlewareConfig == rhs.middlewareConfig
        && lhs.extensionConfig == rhs.extensionConfig;
}

bool operator==(const Sink& lhs, const Sink& rhs)
{
    return lhs.type == rhs.type
        && lhs.level == rhs.level
        && lhs.logname == rhs.logname;
}

bool operator==(const Logger& lhs, const Logger& rhs)
{
    return lhs.logFromRemotes == rhs.logFromRemotes
        && lhs.flush_level == rhs.flush_level
        && lhs.sinks == rhs.sinks;
}

bool operator==(const ParticipantController& lhs, const ParticipantController& rhs)
{
    return lhs.syncType == rhs.syncType
        && lhs.execTimeLimitSoft == rhs.execTimeLimitSoft
        && lhs.execTimeLimitHard == rhs.execTimeLimitHard;
}

bool operator==(const CanController& lhs, const CanController& rhs)
{
    return lhs.name == rhs.name
        && lhs.endpointId == rhs.endpointId
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const LinController& lhs, const LinController& rhs)
{
    return lhs.name == rhs.name
        && lhs.endpointId == rhs.endpointId
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const EthernetController& lhs, const EthernetController& rhs)
{
    return lhs.name == rhs.name
        && lhs.endpointId == rhs.endpointId
        && lhs.macAddress == rhs.macAddress
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const FlexrayController& lhs, const FlexrayController& rhs)
{
    return lhs.name == rhs.name
        && lhs.endpointId == rhs.endpointId
        && lhs.clusterParameters == rhs.clusterParameters
        && lhs.nodeParameters == rhs.nodeParameters
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const DigitalIoPort& lhs, const DigitalIoPort& rhs)
{
    return lhs.direction == rhs.direction
        && lhs.endpointId == rhs.endpointId
        && lhs.name == rhs.name
        && lhs.initvalue == rhs.initvalue
        && lhs.linkId == rhs.linkId
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const AnalogIoPort& lhs, const AnalogIoPort& rhs)
{
    return lhs.direction == rhs.direction
        && lhs.endpointId == rhs.endpointId
        && lhs.name == rhs.name
        && lhs.initvalue == rhs.initvalue
        && lhs.unit == rhs.unit
        && lhs.linkId == rhs.linkId
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const PwmPort& lhs, const PwmPort& rhs)
{
    return lhs.direction == rhs.direction
        && lhs.endpointId == rhs.endpointId
        && lhs.name == rhs.name
        && lhs.initvalue.frequency == rhs.initvalue.frequency
        && lhs.initvalue.dutyCycle == rhs.initvalue.dutyCycle
        && lhs.unit == rhs.unit
        && lhs.linkId == rhs.linkId
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const PatternPort& lhs, const PatternPort& rhs)
{
    return lhs.direction == rhs.direction
        && lhs.endpointId == rhs.endpointId
        && lhs.name == rhs.name
        && lhs.initvalue == rhs.initvalue
        && lhs.linkId == rhs.linkId
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const GenericPort& lhs, const GenericPort& rhs)
{
    return lhs.name == rhs.name
        && lhs.endpointId == rhs.endpointId
        && lhs.linkType == rhs.linkType
        && lhs.protocolType == rhs.protocolType
        && lhs.definitionUri == rhs.definitionUri
        && lhs.useTraceSinks == rhs.useTraceSinks
        && lhs.replay == rhs.replay;
}

bool operator==(const TraceSink& lhs, const TraceSink& rhs)
{
    return lhs.enabled == rhs.enabled
        && lhs.name == rhs.name
        && lhs.outputPath == rhs.outputPath
        && lhs.type == rhs.type;
}

bool operator==(const TraceSource& lhs, const TraceSource& rhs)
{
    return lhs.enabled == rhs.enabled
        && lhs.inputPath == rhs.inputPath
        && lhs.type == rhs.type
        && lhs.name == rhs.name
        ;
}

bool operator==(const Replay& lhs, const Replay& rhs)
{
    return lhs.useTraceSource == rhs.useTraceSource
        && lhs.FilterMessage == rhs.FilterMessage
        && lhs.direction == rhs.direction
        ;
}

bool operator==(const MdfChannel& lhs, const MdfChannel& rhs)
{
    return lhs.channelName == rhs.channelName
        && lhs.channelSource == rhs.channelSource
        && lhs.channelPath == rhs.channelPath

        && lhs.groupName == rhs.groupName
        && lhs.groupSource == rhs.groupSource
        && lhs.groupPath == rhs.groupPath
        ;
}

std::ostream& operator<<(std::ostream& out, const Version& version)
{
    out << version.major << '.'
        << version.minor << '.'
        << version.patchLevel;
    return out;
}

std::istream& operator>>(std::istream& in, Version& version)
{
    char dot;
    in >> version.major >> dot
       >> version.minor >> dot
       >> version.patchLevel;
    return in;
}

std::ostream& to_ostream(std::ostream& out, const std::array<uint8_t, 6>& macAddr)
{
    //instead of ios::copyfmt (which set badbit) we use a temporary stream 
    std::stringstream buf;
    buf.width(2);
    buf.fill('0');

    auto&& iter = macAddr.begin();
    out << std::hex << std::uppercase << std::setw(2) << static_cast<uint16_t>(*iter++);
    for (; iter != macAddr.end(); ++iter)
    {
        out << ':' << std::hex << std::uppercase << std::setw(2) << static_cast<uint16_t>(*iter);
    }

    return out << buf.str();
}

std::istream& from_istream(std::istream& in, std::array<uint8_t, 6>& macAddr)
{
    for (auto&& iter = macAddr.begin();
        iter != macAddr.end();
        ++iter)
    {
        uint16_t not_a_char;
        in >> std::hex >> not_a_char;
        in.ignore(1, ':');
        *iter = static_cast<uint8_t>(not_a_char);
    }
    return in;
}


auto Config::FromYamlString(const std::string& yamlString) -> Config
{
    std::stringstream warnings;
    if (!Validate(yamlString, warnings))
    {
        throw Misconfiguration{ "YAML validation returned errors: \n" + warnings.str()};
    }
    if (warnings.str().size() > 0)
    {
        std::cout << "Config::FromYamlString: warnings: " << warnings.str() << std::endl;
    }
    YAML::Node doc = YAML::Load(yamlString);

    auto config =from_yaml<Config>(doc);
    config.configFilePath.clear();

    PostProcess(config);

    return config;
}

auto Config::FromYamlFile(const std::string& yamlFilename) -> Config
{
    auto yamlString = ReadWholeFile(yamlFilename);
    auto&& config = FromYamlString(yamlString);
    config.configFilePath = yamlFilename;

    return config;
}

auto Config::ToYamlString() -> std::string
{
    auto doc = to_yaml(*this);
    return YAML::Dump(doc);
}

auto Config::FromJsonString(const std::string& jsonString) -> Config
{
    // YAML is a superset of JSON, and as such we can parse
    // it with our YAML parser:
    return FromYamlString(jsonString);
}

auto Config::FromJsonFile(const std::string& jsonFilename) -> Config
{
    auto jsonString = ReadWholeFile(jsonFilename);
    auto&& config = FromJsonString(jsonString);
    config.configFilePath = jsonFilename;

    return config;
}

auto Config::ToJsonString() -> std::string
{
    auto doc = to_yaml(*this);
    return yaml_to_json(doc);
}


#if __cplusplus ==  201402L
// When compiling as C++14, the 'static constexpr variable definitions' are not
// properly exported in the dll (and not properly odr-used from users of the DLL).
// For example, when compiling an extention with GCC and loading the shared
// library, this will result in missing symbols for those linkType definitions.
// Note: In C++17 the static constexpr variable definitions is implicitly inline,
// and the separate definitions in this file are obsolete.
// 
// As a workaround, the missing symbols are defined in this translation
// unit.


constexpr Link::Type CanController::linkType;
constexpr Link::Type LinController::linkType;
constexpr Link::Type EthernetController::linkType;
constexpr Link::Type FlexrayController::linkType;
constexpr Link::Type Switch::Port::linkType;
constexpr Link::Type GenericPort::linkType;


#endif

} // namespace cfg
} // namespace ib
