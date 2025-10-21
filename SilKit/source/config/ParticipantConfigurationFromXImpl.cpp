// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ParticipantConfiguration.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "FileHelpers.hpp"
#include "ParticipantConfigurationFromXImpl.hpp"
#include "silkit/services/logging/string_utils.hpp"

#include "YamlParser.hpp"
#include "YamlValidator.hpp"

#include "fmt/format.h"

namespace SilKit {
namespace Config {

inline namespace V1 {

namespace {

// =================================================================================
//  Helper structs to keep metadata when processing/merging included config snippets
// =================================================================================
using ConfigInclude = std::pair<std::string, SilKit::Config::V1::ParticipantConfiguration>;
struct MiddlewareCache
{
    std::vector<std::string> acceptorUris;
    std::optional<std::string> registryUri;
    std::optional<double> connectTimeoutSeconds;
    std::optional<int> connectAttempts;
    std::optional<int> tcpReceiveBufferSize;
    std::optional<int> tcpSendBufferSize;
    std::optional<bool> tcpNoDelay;
    std::optional<bool> tcpQuickAck;
    std::optional<bool> enableDomainSockets;
    std::optional<bool> registryAsFallbackProxy;
    std::optional<bool> experimentalRemoteParticipantConnection;
};

struct GlobalLogCache
{
    std::optional<bool> logFromRemotes;
    std::optional<Services::Logging::Level> flushLevel;
    std::set<Sink> fileSinks;
    std::optional<Sink> stdOutSink;
    std::optional<Sink> remoteSink;
    std::set<std::string> fileNames;
};

struct TimeSynchronizationCache
{
    std::optional<double> animationFactor;
    std::optional<Aggregation> enableMessageAggregation;
};

struct MetricsCache
{
    std::optional<bool> collectFromRemote;
    std::set<MetricsSink> jsonFileSinks;
    std::set<std::string> fileNames;
    std::optional<MetricsSink> remoteSink;
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
    if (!ValidateWithSchema(text, warnings))
    {
        throw SilKit::ConfigurationError{"YAML validation returned errors:\n" + warnings.str()};
    }
    if (warnings.str().size() > 0)
    {
        std::cout << "YAML validation returned warnings:\n" << warnings.str() << std::endl;
    }
}


// ================================================================================
//  Helper functions to find and open config snippets
// ================================================================================
std::string GetConfigParentPath(const std::string& configFile)
{
    namespace fs = std::filesystem;
    const auto filePath = fs::current_path() / configFile;
    return filePath.parent_path().string();
}

void AppendToSearchPaths(const ParticipantConfiguration& config, ConfigIncludeData& configData)
{
    for (auto&& searchPath : config.includes.searchPathHints)
    {
        if (searchPath.empty())
        {
            std::cout << "Warning: Got empty SearchPathHint!";
            continue;
        }

        std::string suffix = "";
        auto lastChar = searchPath.back();
        // We assume that the user provides a Path
        // Make sure they have a seperator
        if (lastChar != '/' && lastChar != '\\')
        {
            suffix = std::filesystem::path::preferred_separator;
        }

        configData.searchPaths.insert(searchPath + suffix);
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
void MergeCacheField(const std::optional<FieldT>& includeObject, FieldT& rootObject)
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

template <typename T>
void CacheNonDefault(const T& defaultValue, const T& value, const std::string& configName, std::optional<T>& cacheValue)
{
    if (defaultValue != value)
    {
        std::optional<T> optValue{value};

        if (cacheValue.has_value() && (cacheValue != optValue))
        {
            std::stringstream error_msg;
            error_msg << "Config element " << configName << "='" << value << "' " << " is already set to '"
                      << cacheValue.value() << "'!";
            throw SilKit::ConfigurationError(error_msg.str());
        }
        cacheValue = optValue;
    }
}

void Cache(const Middleware& root, MiddlewareCache& cache)
{
    if (!root.acceptorUris.empty())
    {
        if (cache.acceptorUris.size() > 0)
        {
            throw SilKit::ConfigurationError{"AcceptorUris already defined!"};
        }
        cache.acceptorUris = root.acceptorUris;
    }
    static const Middleware defaultObject;
    CacheNonDefault(defaultObject.connectAttempts, root.connectAttempts, "Middleware.ConnectAttempts",
                    cache.connectAttempts);
    CacheNonDefault(defaultObject.tcpNoDelay, root.tcpNoDelay, "Middleware.TcpNoDelay", cache.tcpNoDelay);
    CacheNonDefault(defaultObject.tcpQuickAck, root.tcpQuickAck, "Middleware.TcpQuickAck", cache.tcpQuickAck);
    CacheNonDefault(defaultObject.tcpReceiveBufferSize, root.tcpReceiveBufferSize, "Middleware.TcpReceiveBufferSize",
                    cache.tcpReceiveBufferSize);
    CacheNonDefault(defaultObject.tcpSendBufferSize, root.tcpSendBufferSize, "Middleware.TcpSendBufferSize",
                    cache.tcpSendBufferSize);
    CacheNonDefault(defaultObject.enableDomainSockets, root.enableDomainSockets, "Middleware.EnableDomainSockets",
                    cache.enableDomainSockets);
    CacheNonDefault(defaultObject.registryAsFallbackProxy, root.registryAsFallbackProxy,
                    "Middleware.RegistryAsFallbackProxy", cache.registryAsFallbackProxy);
    CacheNonDefault(defaultObject.registryUri, root.registryUri, "Middleware.RegistryUri", cache.registryUri);
    CacheNonDefault(defaultObject.experimentalRemoteParticipantConnection, root.experimentalRemoteParticipantConnection,
                    "Middleware.ExperimentalRemoteParticipantConnection",
                    cache.experimentalRemoteParticipantConnection);
    CacheNonDefault(defaultObject.connectTimeoutSeconds, root.connectTimeoutSeconds, "Middleware.ConnectTimeoutSeconds",
                    cache.connectTimeoutSeconds);
}
void CacheLoggingOptions(const Logging& config, GlobalLogCache& cache)
{
    const Logging defaultObject{};
    CacheNonDefault(defaultObject.flushLevel, config.flushLevel, "Logging.FlushLevel", cache.flushLevel);
    CacheNonDefault(defaultObject.logFromRemotes, config.logFromRemotes, "Logging.LogFromRemotes",
                    cache.logFromRemotes);
}

void CacheLoggingSinks(const Logging& config, GlobalLogCache& cache)
{
    for (auto&& sink : config.sinks)
    {
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
                throw SilKit::ConfigurationError{"Stdout Sink already exists!"};
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
                throw SilKit::ConfigurationError{"Remote Sink already exists!"};
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

void Cache(const TimeSynchronization& root, TimeSynchronizationCache& cache)
{
    static const TimeSynchronization defaultObject;
    CacheNonDefault(defaultObject.animationFactor, root.animationFactor, "TimeSynchronization.AnimationFactor",
                    cache.animationFactor);
    CacheNonDefault(defaultObject.enableMessageAggregation, root.enableMessageAggregation,
                    "TimeSynchronization.EnableMessageAggregation", cache.enableMessageAggregation);
}

void Cache(const Metrics& root, MetricsCache& cache)
{
    static const Metrics defaultObject;
    if (root.collectFromRemote.has_value())
    {
        CacheNonDefault(false, root.collectFromRemote.value(), "Metrics.CollectFromRemote", cache.collectFromRemote);
    }

    for (const auto& sink : root.sinks)
    {
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

void Cache(const Experimental& root, ExperimentalCache& cache)
{
    Cache(root.timeSynchronization, cache.timeSynchronizationCache);
    Cache(root.metrics, cache.metricsCache);
}


void PopulateCaches(const ParticipantConfiguration& config, ConfigIncludeData& configIncludeData)
{
    const ParticipantConfiguration defaultConfiguration{};
    // Cache those config options that need to be default constructed, since we lose the information
    // about default constructed vs. explicitly set to the default value later

    if (!(config.middleware == defaultConfiguration.middleware))
    {
        Cache(config.middleware, configIncludeData.middlewareCache);
    }
    if (!(config.logging == defaultConfiguration.logging))
    {
        CacheLoggingOptions(config.logging, configIncludeData.logCache);
        CacheLoggingSinks(config.logging, configIncludeData.logCache);
    }
    if (!(config.experimental == defaultConfiguration.experimental))
    {
        Cache(config.experimental, configIncludeData.experimentalCache);
    }
}

// =========================================================================================================================
// Functions to merge the different fields
// =========================================================================================================================
void MergeExtensions(const SilKit::Config::V1::Extensions& child, SilKit::Config::V1::Extensions& parent)
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
    if (metrics.collectFromRemote.has_value())
    {
        MergeCacheField(cache.collectFromRemote, metrics.collectFromRemote.value());
    }
    MergeCacheSet(cache.jsonFileSinks, metrics.sinks);

    if (cache.remoteSink.has_value() && cache.collectFromRemote.value_or(false))
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
        MergeNamedVector<SilKit::Config::V1::CanController>(include.second.canControllers, "CanController",
                                                            config.canControllers, configIncludeData.canCache);
        MergeNamedVector<SilKit::Config::V1::LinController>(include.second.linControllers, "LinController",
                                                            config.linControllers, configIncludeData.linCache);
        MergeNamedVector<SilKit::Config::V1::EthernetController>(include.second.ethernetControllers,
                                                                 "EthernetController", config.ethernetControllers,
                                                                 configIncludeData.ethCache);
        MergeNamedVector<SilKit::Config::V1::FlexrayController>(include.second.flexrayControllers, "FlexRayController",
                                                                config.flexrayControllers,
                                                                configIncludeData.flexrayCache);
        MergeNamedVector<SilKit::Config::V1::DataSubscriber>(include.second.dataSubscribers, "DataSubscriber",
                                                             config.dataSubscribers, configIncludeData.subCache);
        MergeNamedVector<SilKit::Config::V1::DataPublisher>(include.second.dataPublishers, "DataPublisher",
                                                            config.dataPublishers, configIncludeData.pubCache);
        MergeNamedVector<SilKit::Config::V1::RpcServer>(include.second.rpcServers, "RpcServer", config.rpcServers,
                                                        configIncludeData.rpcServerCache);
        MergeNamedVector<SilKit::Config::V1::RpcClient>(include.second.rpcClients, "RpcClient", config.rpcClients,
                                                        configIncludeData.rpcClientCache);

        MergeNamedVector<SilKit::Config::V1::TraceSink>(include.second.tracing.traceSinks, "TraceSink",
                                                        config.tracing.traceSinks, configIncludeData.traceSinkCache);
        MergeNamedVector<SilKit::Config::V1::TraceSource>(include.second.tracing.traceSources, "TraceSource",
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
void ProcessIncludes(const ParticipantConfiguration& config, ConfigIncludeData& configData)
{
    std::vector<std::string> levelIncludes;

    auto collectIncludes = [](auto&& config, auto&& levelIncludes) {
        for (auto&& include : config.includes.files)
        {
            levelIncludes.push_back(include);
        }
    };

    collectIncludes(config, levelIncludes);
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
            auto nextConfigText = SilKit::Config::OpenFileWithSearchHints(include, configData.searchPaths);
            SilKit::Config::Validate(nextConfigText);

            // Load and Parse the file as Yaml
            auto nextConfig = SilKit::Config::Deserialize<ParticipantConfiguration>(nextConfigText);
            configData.configBuffer.push_back(ConfigInclude(include, nextConfig));

            // Append the config to our metadata buffer, so we can make informed decisions whether the compiled config is valid
            configData.includeSet.insert(include);
            AppendToSearchPaths(nextConfig, configData);

            // Collect Caches and Include for the next level within the tree
            collectIncludes(nextConfig, tmpIncludes);
            PopulateCaches(nextConfig, configData);
        }

        // Goto next Level
        levelIncludes = tmpIncludes;

        if (levelIncludes.size() == 0)
        {
            break;
        }
    }
}


auto PaticipantConfigurationWithIncludes(const std::string& text, struct ConfigIncludeData& configData)
    -> SilKit::Config::ParticipantConfiguration
{
    auto configuration = SilKit::Config::Deserialize<ParticipantConfiguration>(text);
    if (configuration.schemaVersion.empty())
    {
        configuration.schemaVersion = SilKit::Config::GetSchemaVersion();
    }

    if (!configuration.schemaVersion.empty()
        && configuration.schemaVersion != SilKitRegistry::Config::V1::GetSchemaVersion())
    {
        throw SilKit::ConfigurationError{fmt::format("Unknown schema version '{}' found in participant configuration!",
                                                     configuration.schemaVersion)};
    }
    configData.configBuffer.push_back(ConfigInclude("root", configuration));

    AppendToSearchPaths(configuration, configData);

    // Get all configs
    ProcessIncludes(configuration, configData);
    // Merge the root and included configs
    return MergeConfigs(configData);
}


} // anonymous namespace
} // namespace V1

// ============================================================================
// Interface Layer functions
// ============================================================================

auto ParticipantConfigurationFromStringImpl(const std::string& text)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    auto configData = ConfigIncludeData{};
    auto configuration = PaticipantConfigurationWithIncludes(text, configData);
    return std::make_shared<SilKit::Config::ParticipantConfiguration>(std::move(configuration));
}

auto ParticipantConfigurationFromFileImpl(const std::string& filename)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    auto configData = ConfigIncludeData{};
    configData.searchPaths.insert(SilKit::Config::GetConfigParentPath(filename));

    // Parse the root config
    std::string text;
    text = SilKit::Config::OpenFileWithSearchHints(filename, configData.searchPaths);

    auto configuration = PaticipantConfigurationWithIncludes(text, configData);
    return std::make_shared<SilKit::Config::ParticipantConfiguration>(std::move(configuration));
}

} // namespace Config
} // namespace SilKit
