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

#include "ParticipantConfiguration.hpp"
#include "Filesystem.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
#include <utility>
#include <vector>

#include "FileHelpers.hpp"
#include "ParticipantConfigurationFromXImpl.hpp"
#include "silkit/services/logging/string_utils.hpp"
#include "YamlParser.hpp"
#include "YamlValidator.hpp"

namespace SilKit {
namespace Config {

inline namespace v1 {

namespace {

// =================================================================================
//  Helper structs to keep metadata when processing/merging included config snippets
// =================================================================================
using ConfigInclude = std::pair<std::string, SilKit::Config::v1::ParticipantConfiguration>;
struct MiddlewareCache
{
    std::vector<std::string> acceptorUris;
    SilKit::Util::Optional<std::string> registryUri;
    SilKit::Util::Optional<double> connectTimeoutSeconds;
    SilKit::Util::Optional<int> connectAttempts;
    SilKit::Util::Optional<int> tcpReceiveBufferSize;
    SilKit::Util::Optional<int> tcpSendBufferSize;
    SilKit::Util::Optional<bool> tcpNoDelay;
    SilKit::Util::Optional<bool> tcpQuickAck;
    SilKit::Util::Optional<bool> enableDomainSockets;
    SilKit::Util::Optional<bool> registryAsFallbackProxy;
    SilKit::Util::Optional<bool> experimentalRemoteParticipantConnection;
};

struct GlobalLogCache
{
    SilKit::Util::Optional<bool> logFromRemotes;
    SilKit::Util::Optional<Services::Logging::Level> flushLevel;
    std::set<Sink> fileSinks;
    SilKit::Util::Optional<Sink> stdOutSink;
    SilKit::Util::Optional<Sink> remoteSink;
    std::set<std::string> fileNames;
};

struct TimeSynchronizationCache
{
    SilKit::Util::Optional<double> animationFactor;
    SilKit::Util::Optional<Aggregation> enableMessageAggregation;
};

struct MetricsCache
{
    SilKit::Util::Optional<bool> collectFromRemote;
    std::set<MetricsSink> jsonFileSinks;
    std::set<std::string> fileNames;
    SilKit::Util::Optional<MetricsSink> remoteSink;
};

struct ExperimentalCache
{
    TimeSynchronizationCache timeSynchronizationCache;
    MetricsCache metricsCache;
};

struct ConfigIncludeData
{
    std::set<std::string> searchPaths;
    std::set<std::string> includeSet;
    std::vector<ConfigInclude> configBuffer;
    MiddlewareCache middlewareCache;
    GlobalLogCache logCache;
    ExperimentalCache experimentalCache;
    std::map<std::string, SilKit::Config::CanController> canCache;
    std::map<std::string, SilKit::Config::LinController> linCache;
    std::map<std::string, SilKit::Config::EthernetController> ethCache;
    std::map<std::string, SilKit::Config::FlexrayController> flexrayCache;
    std::map<std::string, SilKit::Config::DataSubscriber> subCache;
    std::map<std::string, SilKit::Config::DataPublisher> pubCache;
    std::map<std::string, SilKit::Config::RpcServer> rpcServerCache;
    std::map<std::string, SilKit::Config::RpcClient> rpcClientCache;
    std::map<std::string, SilKit::Config::TraceSink> traceSinkCache;
    std::map<std::string, SilKit::Config::TraceSource> traceSourceCache;
};


// ================================================================================
//  Helper functions to work with text and YAML
// ================================================================================
void Validate(const std::string& text)
{
    std::stringstream warnings;
    SilKit::Config::YamlValidator validator;
    if (!validator.Validate(text, warnings))
    {
        throw SilKit::ConfigurationError{"YAML validation returned errors: \n" + warnings.str()};
    }
    if (warnings.str().size() > 0)
    {
        std::cout << "YAML validation returned warnings: \n" << warnings.str() << std::endl;
    }
}

auto Parse(const YAML::Node& doc) -> SilKit::Config::ParticipantConfiguration
{
    auto configuration = !doc.IsNull() ? SilKit::Config::from_yaml<SilKit::Config::v1::ParticipantConfiguration>(doc)
                                       : SilKit::Config::v1::ParticipantConfiguration{};
    configuration.configurationFilePath.clear();
    return configuration;
}

void CollectIncludes(const YAML::Node& config, std::vector<std::string>& levelIncludes)
{
    if (config["Includes"])
    {
        for (const auto& include : config["Includes"]["Files"])
        {
            levelIncludes.push_back(include.as<std::string>());
        }
    }
}

// ================================================================================
//  Helper functions to find and open config snippets
// ================================================================================
std::string GetConfigParentPath(const std::string& configFile)
{
    namespace fs = SilKit::Filesystem;
    auto filePath = fs::concatenate_paths(fs::current_path().string(), configFile);
    return fs::parent_path(filePath).string();
}

void AppendToSearchPaths(const YAML::Node& doc, ConfigIncludeData& configData)
{
    if (doc["Includes"])
    {
        for (auto& searchPath : doc["Includes"]["SearchPathHints"])
        {
            auto tmpString = searchPath.as<std::string>();

            if (tmpString.empty())
            {
                std::cout << "Warning: Got empty SearchPathHint!";
                continue;
            }

            std::string suffix = "";
            auto lastChar = tmpString.back();
            // We assume that the user provides a Path
            // Make sure they have a seperator
            if (lastChar != '/' && lastChar != '\\')
            {
                suffix = SilKit::Filesystem::path::preferred_separator;
            }
            configData.searchPaths.insert(searchPath.as<std::string>() + suffix);
        }
    }
}

std::string OpenFileWithSearchHints(const std::string& configFile, const std::set<std::string>& searchPathHints)
{
    std::stringstream buffer;
    std::string text;

    auto ifs = Util::OpenIFStream(configFile);
    // Try cwd first
    if (ifs.is_open())
    {
        buffer << ifs.rdbuf();
        return buffer.str();
    }

    for (auto& searchPathHint : searchPathHints)
    {
        auto completePath = searchPathHint + configFile;

        ifs = Util::OpenIFStream(completePath);

        if (ifs.is_open())
        {
            buffer << ifs.rdbuf();
            return buffer.str();
        }
    }

    std::stringstream error_msg;
    error_msg << "The config file " << configFile << " could not be opened!";
    throw SilKit::ConfigurationError(error_msg.str());
}

// ================================================================================
//  Helper functions to merge config fields/vectors/sets
// ================================================================================
template <typename FieldT>
void MergeCacheField(const SilKit::Util::Optional<FieldT>& includeObject, FieldT& rootObject)
{
    if (includeObject.has_value())
    {
        rootObject = includeObject.value();
    }
}

template <typename ConfigT>
void MergeNamedVector(const std::vector<ConfigT>& child, const std::string& field_name, std::vector<ConfigT>& parent,
                      std::map<std::string, ConfigT>& cache)
{
    std::map<std::string, unsigned> parentState;

    for (const auto& element : child)
    {
        auto duplicate = cache.find(element.name);
        if (duplicate == cache.end())
        {
            parent.push_back(element);
            cache[element.name] = element;
        }
        else
        {
            if (duplicate->second == element)
            {
                std::cout << "Warning: exact duplicate of " << element.name << " found already!" << std::endl;
            }
            else
            {
                std::stringstream error_msg;
                error_msg << "Config element " << field_name << " with name " << element.name
                          << " but different config already exists!";
                throw SilKit::ConfigurationError(error_msg.str());
            }
        }
    }
}

template <typename T>
void MergeCacheSet(const std::set<T>& cache, std::vector<T>& root)
{
    for (auto& element : cache)
    {
        root.push_back(element);
    }
}

// ================================================================================
//  Helper functions to cache config entries with complicated merge strategies
// ================================================================================
template <typename FieldT>
void PopulateCacheField(const YAML::Node& root, std::string rootname, const std::string& field,
                        SilKit::Util::Optional<FieldT>& obj)
{
    SilKit::Util::Optional<FieldT> tmpObj;
    optional_decode(tmpObj, root, field);

    if (tmpObj.has_value())
    {
        if (obj.has_value() && (obj != tmpObj))
        {
            std::stringstream error_msg;
            error_msg << "Config element " << field << "(" << tmpObj << ") for " << rootname << " already set to \'"
                      << obj.value() << "\'!";
            throw SilKit::ConfigurationError(error_msg.str());
        }
        obj = tmpObj;
    }
}

void CacheMiddleware(const YAML::Node& root, MiddlewareCache& cache)
{
    if (root["AcceptorUris"])
    {
        if (cache.acceptorUris.size() > 0)
        {
            throw SilKit::ConfigurationError{"AcceptorUris already defined!"};
        }
        optional_decode(cache.acceptorUris, root, "AcceptorUris");
    }

    PopulateCacheField(root, "Middleware", "ConnectAttempts", cache.connectAttempts);
    PopulateCacheField(root, "Middleware", "TcpNoDelay", cache.tcpNoDelay);
    PopulateCacheField(root, "Middleware", "TcpQuickAck", cache.tcpQuickAck);
    PopulateCacheField(root, "Middleware", "TcpReceiveBufferSize", cache.tcpReceiveBufferSize);
    PopulateCacheField(root, "Middleware", "TcpSendBufferSize", cache.tcpSendBufferSize);
    PopulateCacheField(root, "Middleware", "EnableDomainSockets", cache.enableDomainSockets);
    PopulateCacheField(root, "Middleware", "RegistryAsFallbackProxy", cache.registryAsFallbackProxy);
    PopulateCacheField(root, "Middleware", "RegistryUri", cache.registryUri);
    PopulateCacheField(root, "Middleware", "ExperimentalRemoteParticipantConnection",
                       cache.experimentalRemoteParticipantConnection);
    PopulateCacheField(root, "Middleware", "ConnectTimeoutSeconds", cache.connectTimeoutSeconds);
}

void CacheLoggingOptions(const YAML::Node& root, GlobalLogCache& cache)
{
    PopulateCacheField(root, "Logging", "FlushLevel", cache.flushLevel);
    PopulateCacheField(root, "Logging", "LogFromRemotes", cache.logFromRemotes);
}

void CacheLoggingSinks(const YAML::Node& config, GlobalLogCache& cache)
{
    for (const auto& sinkNode : config["Sinks"])
    {
        auto sink = parse_as<Sink>(sinkNode);
        if (sink.type == Sink::Type::Stdout)
        {
            if (!cache.stdOutSink.has_value())
            {
                // Replace the already included sink with this one
                // since we have not set it yet
                cache.stdOutSink = sink;
            }
            else
            {
                std::stringstream error_msg;
                error_msg << "Stdout Sink already exists!";
                throw SilKit::ConfigurationError(error_msg.str());
            }
        }
        else if (sink.type == Sink::Type::Remote)
        {
            if (!cache.remoteSink.has_value())
            {
                // Replace the already included sink with this one
                // since we have not set it yet
                cache.remoteSink = sink;
            }
            else
            {
                std::stringstream error_msg;
                error_msg << "Remote Sink already exists!";
                throw SilKit::ConfigurationError(error_msg.str());
            }
        }
        else
        {
            if (cache.fileNames.count(sink.logName) == 0)
            {
                cache.fileSinks.insert(sink);
                cache.fileNames.insert(sink.logName);
            }
            else
            {
                std::stringstream error_msg;
                error_msg << "Filesink " << sink.logName << " already exists!";
                throw SilKit::ConfigurationError(error_msg.str());
            }
        }
    }
}

void CacheTimeSynchronization(const YAML::Node& root, TimeSynchronizationCache& cache)
{
    PopulateCacheField(root, "TimeSynchronization", "AnimationFactor", cache.animationFactor);
    PopulateCacheField(root, "TimeSynchronization", "EnableMessageAggregation", cache.enableMessageAggregation);
}

void CacheMetrics(const YAML::Node& root, MetricsCache& cache)
{
    PopulateCacheField(root, "Metrics", "CollectFromRemote", cache.collectFromRemote);

    if (root["Sinks"])
    {
        for (const auto& sinkNode : root["Sinks"])
        {
            auto sink = parse_as<MetricsSink>(sinkNode);
            if (sink.type == MetricsSink::Type::JsonFile)
            {
                if (cache.fileNames.count(sink.name) == 0)
                {
                    cache.jsonFileSinks.insert(sink);
                    cache.fileNames.insert(sink.name);
                }
                else
                {
                    std::stringstream error_msg;
                    error_msg << "JSON file metrics sink " << sink.name << " already exists!";
                    throw SilKit::ConfigurationError(error_msg.str());
                }
            }
            else if (sink.type == MetricsSink::Type::Remote)
            {
                if (!cache.remoteSink.has_value())
                {
                    // Replace the already included sink with this one
                    // since we have not set it yet
                    cache.remoteSink = sink;
                }
                else
                {
                    std::stringstream error_msg;
                    error_msg << "Remote metrics sink already exists!";
                    throw SilKit::ConfigurationError(error_msg.str());
                }
            }
            else
            {
                std::stringstream error_msg;
                error_msg << "Invalid MetricsSink::Type("
                          << static_cast<std::underlying_type_t<MetricsSink::Type>>(sink.type) << ")";
                throw SilKit::ConfigurationError(error_msg.str());
            }
        }
    }
}

void CacheExperimental(const YAML::Node& root, ExperimentalCache& cache)
{
    if (root["TimeSynchronization"])
    {
        CacheTimeSynchronization(root["TimeSynchronization"], cache.timeSynchronizationCache);
    }

    if (root["Metrics"])
    {
        CacheMetrics(root["Metrics"], cache.metricsCache);
    }
}

void PopulateCaches(const YAML::Node& config, ConfigIncludeData& configIncludeData)
{
    // Cache those config options that need to be default constructed, since we lose the information
    // about default constructed vs. explicitly set to the default value later
    if (config["Middleware"])
    {
        CacheMiddleware(config["Middleware"], configIncludeData.middlewareCache);
    }

    if (config["Logging"])
    {
        CacheLoggingOptions(config["Logging"], configIncludeData.logCache);
        CacheLoggingSinks(config["Logging"], configIncludeData.logCache);
    }

    if (config["Experimental"])
    {
        CacheExperimental(config["Experimental"], configIncludeData.experimentalCache);
    }
}

// =========================================================================================================================
// Functions to merge the different fields
// =========================================================================================================================
void MergeExtensions(const SilKit::Config::v1::Extensions& child, SilKit::Config::v1::Extensions& parent)
{
    parent.searchPathHints.insert(parent.searchPathHints.end(), child.searchPathHints.begin(),
                                  child.searchPathHints.end());
}

void MergeHealthCheck(const SilKit::Config::HealthCheck& include, SilKit::Config::HealthCheck& healthCheck)
{
    if (include.softResponseTimeout.has_value())
    {
        if (healthCheck.softResponseTimeout.has_value()
            && (healthCheck.softResponseTimeout.value() == include.softResponseTimeout.value()))
        {
            std::stringstream error_msg;
            error_msg << "HealthCheck.SoftResponseTimeout already set to: "
                      << healthCheck.softResponseTimeout.value().count() << "ms";
            throw SilKit::ConfigurationError(error_msg.str());
        }

        healthCheck.softResponseTimeout = include.softResponseTimeout;
    }

    if (include.hardResponseTimeout.has_value())
    {
        if (healthCheck.hardResponseTimeout.has_value()
            && healthCheck.hardResponseTimeout.value() == include.hardResponseTimeout.value())
        {
            std::stringstream error_msg;
            error_msg << "HealthCheck.HardResponseTimeout already set to: "
                      << healthCheck.hardResponseTimeout.value().count() << "ms";
            throw SilKit::ConfigurationError(error_msg.str());
        }

        healthCheck.hardResponseTimeout = include.hardResponseTimeout;
    }
}

void MergeParticipantName(const SilKit::Config::ParticipantConfiguration& include,
                          SilKit::Config::ParticipantConfiguration& config)
{
    if (include.participantName.size())
    {
        if (config.participantName.empty())
        {
            config.participantName = include.participantName;
        }
        else
        {
            throw SilKit::ConfigurationError("Participant Name already set to " + config.participantName);
        }
    }
}

void MergeMiddleware(const MiddlewareCache& cache, Middleware& middleware)
{
    MergeCacheField(cache.connectAttempts, middleware.connectAttempts);
    MergeCacheField(cache.tcpNoDelay, middleware.tcpNoDelay);
    MergeCacheField(cache.tcpQuickAck, middleware.tcpQuickAck);
    MergeCacheField(cache.tcpSendBufferSize, middleware.tcpSendBufferSize);
    MergeCacheField(cache.tcpReceiveBufferSize, middleware.tcpReceiveBufferSize);
    MergeCacheField(cache.enableDomainSockets, middleware.enableDomainSockets);
    MergeCacheField(cache.registryUri, middleware.registryUri);
    MergeCacheField(cache.registryAsFallbackProxy, middleware.registryAsFallbackProxy);
    MergeCacheField(cache.experimentalRemoteParticipantConnection, middleware.experimentalRemoteParticipantConnection);
    MergeCacheField(cache.connectTimeoutSeconds, middleware.connectTimeoutSeconds);

    middleware.acceptorUris = cache.acceptorUris;
}

void MergeLogCache(const GlobalLogCache& cache, Logging& logging)
{
    MergeCacheField(cache.flushLevel, logging.flushLevel);
    MergeCacheField(cache.logFromRemotes, logging.logFromRemotes);
    MergeCacheSet(cache.fileSinks, logging.sinks);

    if (cache.stdOutSink.has_value())
    {
        logging.sinks.push_back(cache.stdOutSink.value());
    }

    if (cache.remoteSink.has_value())
    {
        logging.sinks.push_back(cache.remoteSink.value());
    }
}

void MergeTimeSynchronizationCache(const TimeSynchronizationCache& cache, TimeSynchronization& timeSynchronization)
{
    MergeCacheField(cache.animationFactor, timeSynchronization.animationFactor);
    MergeCacheField(cache.enableMessageAggregation, timeSynchronization.enableMessageAggregation);
}

void MergeMetricsCache(const MetricsCache& cache, Metrics& metrics)
{
    MergeCacheField(cache.collectFromRemote, metrics.collectFromRemote);
    MergeCacheSet(cache.jsonFileSinks, metrics.sinks);

    if (cache.remoteSink.has_value() && metrics.collectFromRemote)
    {
        throw SilKit::ConfigurationError{
            "Cannot have 'Remote' metrics sink together with 'CollectFromRemote' being true"};
    }

    if (cache.remoteSink.has_value())
    {
        metrics.sinks.push_back(cache.remoteSink.value());
    }
}

void MergeExperimentalCache(const ExperimentalCache& cache, Experimental& experimental)
{
    MergeTimeSynchronizationCache(cache.timeSynchronizationCache, experimental.timeSynchronization);
    MergeMetricsCache(cache.metricsCache, experimental.metrics);
}


auto MergeConfigs(ConfigIncludeData& configIncludeData) -> SilKit::Config::ParticipantConfiguration
{
    SilKit::Config::ParticipantConfiguration config;
    for (const auto& include : configIncludeData.configBuffer)
    {
        // Merge all vectors first!
        MergeNamedVector<SilKit::Config::v1::CanController>(include.second.canControllers, "CanController",
                                                            config.canControllers, configIncludeData.canCache);
        MergeNamedVector<SilKit::Config::v1::LinController>(include.second.linControllers, "LinController",
                                                            config.linControllers, configIncludeData.linCache);
        MergeNamedVector<SilKit::Config::v1::EthernetController>(include.second.ethernetControllers,
                                                                 "EthernetController", config.ethernetControllers,
                                                                 configIncludeData.ethCache);
        MergeNamedVector<SilKit::Config::v1::FlexrayController>(include.second.flexrayControllers, "FlexRayController",
                                                                config.flexrayControllers,
                                                                configIncludeData.flexrayCache);
        MergeNamedVector<SilKit::Config::v1::DataSubscriber>(include.second.dataSubscribers, "DataSubscriber",
                                                             config.dataSubscribers, configIncludeData.subCache);
        MergeNamedVector<SilKit::Config::v1::DataPublisher>(include.second.dataPublishers, "DataPublisher",
                                                            config.dataPublishers, configIncludeData.pubCache);
        MergeNamedVector<SilKit::Config::v1::RpcServer>(include.second.rpcServers, "RpcServer", config.rpcServers,
                                                        configIncludeData.rpcServerCache);
        MergeNamedVector<SilKit::Config::v1::RpcClient>(include.second.rpcClients, "RpcClient", config.rpcClients,
                                                        configIncludeData.rpcClientCache);

        MergeNamedVector<SilKit::Config::v1::TraceSink>(include.second.tracing.traceSinks, "TraceSink",
                                                        config.tracing.traceSinks, configIncludeData.traceSinkCache);
        MergeNamedVector<SilKit::Config::v1::TraceSource>(include.second.tracing.traceSources, "TraceSource",
                                                          config.tracing.traceSources,
                                                          configIncludeData.traceSourceCache);

        // Merge "scalar" config fields
        MergeExtensions(include.second.extensions, config.extensions);
        MergeHealthCheck(include.second.healthCheck, config.healthCheck);
        MergeParticipantName(include.second, config);
    }

    MergeMiddleware(configIncludeData.middlewareCache, config.middleware);
    MergeLogCache(configIncludeData.logCache, config.logging);
    MergeExperimentalCache(configIncludeData.experimentalCache, config.experimental);

    return config;
}

// =========================================================================================================================
// Include Logic
// =========================================================================================================================
void ProcessIncludes(const YAML::Node& config, ConfigIncludeData& configData)
{
    std::vector<std::string> levelIncludes;

    CollectIncludes(config, levelIncludes);
    PopulateCaches(config, configData);

    // Breadth first traversal, which means we collect all includes per "level" first
    // and then use the collected includes as the "next level"
    const auto upperbound = 127u;

    for (auto i = 0u; i < upperbound; ++i)
    {
        std::vector<std::string> tmpIncludes;
        for (const auto& include : levelIncludes)
        {
            if (configData.includeSet.count(include) > 0)
            {
                std::cout << "Warning: Config " << include << " already included!" << std::endl;
                continue;
            }

            // Get the next Include to be processed within this tree level
            auto nextConfig = SilKit::Config::OpenFileWithSearchHints(include, configData.searchPaths);
            SilKit::Config::Validate(nextConfig);

            // Load and Parse the file as Yaml
            auto nextConfigNode = YAML::Load(nextConfig);
            configData.configBuffer.push_back((ConfigInclude(include, SilKit::Config::Parse(nextConfigNode))));

            // Append the config to our metadata buffer, so we can make informed decisions whether the compiled config is valid
            configData.includeSet.insert(include);
            AppendToSearchPaths(nextConfigNode, configData);

            // Collect Caches and Include for the next level within the tree
            CollectIncludes(nextConfigNode, tmpIncludes);
            PopulateCaches(nextConfigNode, configData);
        }

        // Goto next Level
        levelIncludes = tmpIncludes;

        if (levelIncludes.size() == 0)
        {
            break;
        }
    }
}

auto ParticipantConfigurationFromXImpl(const std::string& text,
                                       struct ConfigIncludeData& configData) -> SilKit::Config::ParticipantConfiguration
{
    SilKit::Config::Validate(text);
    YAML::Node doc = YAML::Load(text);

    auto configuration = SilKit::Config::Parse(doc);
    configData.configBuffer.push_back(ConfigInclude("root", configuration));

    // Check search Paths
    if (doc["Includes"])
    {
        AppendToSearchPaths(doc, configData);
    }

    // Get all configs
    ProcessIncludes(doc, configData);
    // Merge the root and included configs
    return MergeConfigs(configData);
}


} // anonymous namespace

// ================================================================================
//  Implementation data types
// ================================================================================
bool operator==(const CanController& lhs, const CanController& rhs)
{
    return lhs.name == rhs.name && lhs.network == rhs.network && lhs.replay == rhs.replay
           && lhs.useTraceSinks == rhs.useTraceSinks;
}

bool operator==(const LinController& lhs, const LinController& rhs)
{
    return lhs.name == rhs.name && lhs.network == rhs.network && lhs.useTraceSinks == rhs.useTraceSinks
           && lhs.replay == rhs.replay;
}

bool operator==(const EthernetController& lhs, const EthernetController& rhs)
{
    return lhs.name == rhs.name && lhs.network == rhs.network && lhs.useTraceSinks == rhs.useTraceSinks
           && lhs.replay == rhs.replay;
}

bool operator==(const FlexrayController& lhs, const FlexrayController& rhs)
{
    return lhs.name == rhs.name && lhs.network == rhs.network && lhs.clusterParameters == rhs.clusterParameters
           && lhs.nodeParameters == rhs.nodeParameters && lhs.useTraceSinks == rhs.useTraceSinks
           && lhs.replay == rhs.replay;
}

bool operator==(const DataPublisher& lhs, const DataPublisher& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks && lhs.replay == rhs.replay;
}

bool operator==(const DataSubscriber& lhs, const DataSubscriber& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks && lhs.replay == rhs.replay;
}

bool operator==(const RpcServer& lhs, const RpcServer& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks && lhs.replay == rhs.replay;
}

bool operator==(const RpcClient& lhs, const RpcClient& rhs)
{
    return lhs.useTraceSinks == rhs.useTraceSinks && lhs.replay == rhs.replay;
}

bool operator==(const HealthCheck& lhs, const HealthCheck& rhs)
{
    return lhs.softResponseTimeout == rhs.softResponseTimeout && lhs.hardResponseTimeout == rhs.hardResponseTimeout;
}

bool operator==(const Tracing& lhs, const Tracing& rhs)
{
    return lhs.traceSinks == rhs.traceSinks && lhs.traceSources == rhs.traceSources;
}

bool operator==(const MetricsSink& lhs, const MetricsSink& rhs)
{
    return lhs.type == rhs.type && lhs.name == rhs.name;
}

bool operator==(const Metrics& lhs, const Metrics& rhs)
{
    return lhs.sinks == rhs.sinks && lhs.collectFromRemote == rhs.collectFromRemote;
}

bool operator==(const Extensions& lhs, const Extensions& rhs)
{
    return lhs.searchPathHints == rhs.searchPathHints;
}


bool operator==(const Middleware& lhs, const Middleware& rhs)
{
    return lhs.registryUri == rhs.registryUri && lhs.connectAttempts == rhs.connectAttempts
           && lhs.enableDomainSockets == rhs.enableDomainSockets && lhs.tcpNoDelay == rhs.tcpNoDelay
           && lhs.tcpQuickAck == rhs.tcpQuickAck && lhs.tcpReceiveBufferSize == rhs.tcpReceiveBufferSize
           && lhs.tcpSendBufferSize == rhs.tcpSendBufferSize && lhs.acceptorUris == rhs.acceptorUris;
}

bool operator==(const ParticipantConfiguration& lhs, const ParticipantConfiguration& rhs)
{
    return lhs.participantName == rhs.participantName && lhs.canControllers == rhs.canControllers
           && lhs.linControllers == rhs.linControllers && lhs.ethernetControllers == rhs.ethernetControllers
           && lhs.flexrayControllers == rhs.flexrayControllers && lhs.dataPublishers == rhs.dataPublishers
           && lhs.dataSubscribers == rhs.dataSubscribers && lhs.rpcClients == rhs.rpcClients
           && lhs.rpcServers == rhs.rpcServers && lhs.logging == rhs.logging && lhs.healthCheck == rhs.healthCheck
           && lhs.tracing == rhs.tracing && lhs.extensions == rhs.extensions && lhs.experimental == rhs.experimental;
}

bool operator==(const TimeSynchronization& lhs, const TimeSynchronization& rhs)
{
    return lhs.animationFactor == rhs.animationFactor && lhs.enableMessageAggregation == rhs.enableMessageAggregation;
}

bool operator==(const Experimental& lhs, const Experimental& rhs)
{
    return lhs.timeSynchronization == rhs.timeSynchronization && lhs.metrics == rhs.metrics;
}

bool operator<(const MetricsSink& lhs, const MetricsSink& rhs)
{
    return std::make_tuple(lhs.type, lhs.name) < std::make_tuple(rhs.type, rhs.name);
}

bool operator>(const MetricsSink& lhs, const MetricsSink& rhs)
{
    return !(lhs < rhs);
}

} // namespace v1


// ============================================================================
// Interface Layer functions
// ============================================================================
auto ParticipantConfigurationFromStringImpl(const std::string& text)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    auto configData = ConfigIncludeData();
    auto configuration = SilKit::Config::ParticipantConfigurationFromXImpl(text, configData);
    return std::make_shared<SilKit::Config::ParticipantConfiguration>(std::move(configuration));
}

auto ParticipantConfigurationFromFileImpl(const std::string& filename)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    auto configData = ConfigIncludeData();
    configData.searchPaths.insert(SilKit::Config::GetConfigParentPath(filename));

    // Parse the root config
    std::string text;
    try
    {
        text = SilKit::Config::OpenFileWithSearchHints(filename, configData.searchPaths);
    }
    catch (...)
    {
        throw;
    }

    auto configuration = SilKit::Config::ParticipantConfigurationFromXImpl(text, configData);
    return std::make_shared<SilKit::Config::ParticipantConfiguration>(std::move(configuration));
}

} // namespace Config
} // namespace SilKit
