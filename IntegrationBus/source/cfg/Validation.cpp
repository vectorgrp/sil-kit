// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Validation.hpp"

#include <iostream>
#include <sstream>
#include <set>
#include <unordered_set>

using namespace std::chrono_literals;

namespace ib {
namespace cfg {
inline namespace v1 {

namespace {

void ValidateTraceSinks(const ib::cfg::v1::datatypes::ParticipantConfiguration& configuration)
{
    std::set<std::string> sinkNames;
    for (const auto& sink : configuration.tracing.traceSinks)
    {
        if (sink.name.empty())
        {
            throw ib::configuration_error{ "On Participant " + configuration.participantName + ": TraceSink \"Name\" must not be empty!" };
        }
        if (sink.outputPath.empty())
        {
            throw ib::configuration_error{ "On Participant " + configuration.participantName + ": TraceSink \"OutputPath\" must not be empty!" };
        }
        sinkNames.insert(sink.name);
    }

    if (sinkNames.size() != configuration.tracing.traceSinks.size())
    {
        throw ib::configuration_error{ "TraceSinks must have unique names!" };
    }

    auto sinkExists = [&sinkNames](const auto& name) -> bool {
        return std::end(sinkNames) != std::find(sinkNames.cbegin(), sinkNames.cend(), name);
    };

    auto validateController = [&](const auto& controllers)
    {
        for (const auto& controller : controllers)
        {
            std::stringstream ss;
            ss << "Error: Participant \"" << configuration.participantName << "/" << controller.name << "\" ";

            for (const auto& traceSink : controller.useTraceSinks)
            {
                if (traceSink.empty())
                {
                    ss << "has an empty string in a  UseTraceSinks field!";
                    throw ib::configuration_error{ ss.str() };
                }
                if (!sinkExists(traceSink))
                {
                    ss << "has a UseTraceSinks field which refers to a non-existing TraceSink: " << traceSink;
                    throw ib::configuration_error{ ss.str() };
                }
            }
        }
    };

    validateController(configuration.ethernetControllers);
    validateController(configuration.canControllers);
    validateController(configuration.linControllers);
    validateController(configuration.flexRayControllers);
    validateController(configuration.dataPublishers);
    validateController(configuration.dataSubscribers);
    validateController(configuration.rpcServers);
    validateController(configuration.rpcClients);
}

void ValidateTraceSources(const ib::cfg::v1::datatypes::ParticipantConfiguration& configuration)
{
    std::set<std::string> sourceNames;
    for (const auto& source : configuration.tracing.traceSources)
    {
        if (source.name.empty())
        {
            throw ib::configuration_error{ "On Participant " + configuration.participantName + ": TraceSource \"Name\" must not be empty!" };
        }

        if (source.inputPath.empty())
        {
            throw ib::configuration_error{ "On Participant " + configuration.participantName + ": TraceSource \"InputPath\" must not be empty!" };
        }

        auto ok = sourceNames.insert(source.name);
        if (!ok.second)
        {
            throw ib::configuration_error{ "TraceSources must have a unique name: duplicate entry is " + source.name };
        }
    }

    auto sourceExists = [&sourceNames](const auto& name) -> bool {
        return std::end(sourceNames) != std::find(sourceNames.cbegin(), sourceNames.cend(), name);
    };

    auto validateController = [&](const auto& controllers)
    {
        for (const auto& controller : controllers)
        {
            std::stringstream ss;
            ss << "Error: Participant \"" << configuration.participantName << "/" << controller.name << "\" ";

            if (controller.replay.useTraceSource.empty())
            {
                continue;
            }

            if (!sourceExists(controller.replay.useTraceSource))
            {
                ss << "has a Replay::UseTraceSource field which refers to a non-existing TraceSource: "
                    << controller.replay.useTraceSource;
                throw ib::configuration_error{ ss.str() };
            }
        }
    };

    validateController(configuration.ethernetControllers);
    validateController(configuration.canControllers);
    validateController(configuration.linControllers);
    validateController(configuration.flexRayControllers);
    validateController(configuration.dataPublishers);
    validateController(configuration.dataSubscribers);
    validateController(configuration.rpcServers);
    validateController(configuration.rpcClients);
}

} // anonymous namespace

void Validate(const ib::cfg::v1::datatypes::ParticipantConfiguration& configuration)
{
    ValidateTraceSinks(configuration);
    ValidateTraceSources(configuration);
}

} // namespace v1
} // namespace cfg
} // namespace ib
