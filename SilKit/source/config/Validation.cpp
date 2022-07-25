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

#include "Validation.hpp"

#include <iostream>
#include <sstream>
#include <set>
#include <unordered_set>

using namespace std::chrono_literals;

namespace SilKit {
namespace Config {

inline namespace v1 {

namespace {

void ValidateTraceSinks(const SilKit::Config::v1::ParticipantConfiguration& configuration)
{
    std::set<std::string> sinkNames;
    for (const auto& sink : configuration.tracing.traceSinks)
    {
        if (sink.name.empty())
        {
            throw SilKit::ConfigurationError{ "On Participant " + configuration.participantName + 
                ": TraceSink \"Name\" must not be empty!" };
        }
        if (sink.outputPath.empty())
        {
            throw SilKit::ConfigurationError{ "On Participant " + configuration.participantName + 
                ": TraceSink \"OutputPath\" must not be empty!" };
        }
        sinkNames.insert(sink.name);
    }

    if (sinkNames.size() != configuration.tracing.traceSinks.size())
    {
        throw SilKit::ConfigurationError{ "TraceSinks must have unique names!" };
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
                    throw SilKit::ConfigurationError{ ss.str() };
                }
                if (!sinkExists(traceSink))
                {
                    ss << "has a UseTraceSinks field which refers to a non-existing TraceSink: " << traceSink;
                    throw SilKit::ConfigurationError{ ss.str() };
                }
            }
        }
    };

    validateController(configuration.ethernetControllers);
    validateController(configuration.canControllers);
    validateController(configuration.linControllers);
    validateController(configuration.flexrayControllers);
    validateController(configuration.dataPublishers);
    validateController(configuration.dataSubscribers);
    validateController(configuration.rpcServers);
    validateController(configuration.rpcClients);
}

void ValidateTraceSources(const SilKit::Config::v1::ParticipantConfiguration& configuration)
{
    std::set<std::string> sourceNames;
    for (const auto& source : configuration.tracing.traceSources)
    {
        if (source.name.empty())
        {
            throw SilKit::ConfigurationError{ "On Participant " + configuration.participantName + 
                ": TraceSource \"Name\" must not be empty!" };
        }

        if (source.inputPath.empty())
        {
            throw SilKit::ConfigurationError{ "On Participant " + configuration.participantName + 
                ": TraceSource \"InputPath\" must not be empty!" };
        }

        auto ok = sourceNames.insert(source.name);
        if (!ok.second)
        {
            throw SilKit::ConfigurationError{ "TraceSources must have a unique name: duplicate entry is " + source.name };
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
                throw SilKit::ConfigurationError{ ss.str() };
            }
        }
    };

    validateController(configuration.ethernetControllers);
    validateController(configuration.canControllers);
    validateController(configuration.linControllers);
    validateController(configuration.flexrayControllers);
    validateController(configuration.dataPublishers);
    validateController(configuration.dataSubscribers);
    validateController(configuration.rpcServers);
    validateController(configuration.rpcClients);
}

} // anonymous namespace

void Validate(const SilKit::Config::v1::ParticipantConfiguration& configuration)
{
    ValidateTraceSinks(configuration);
    ValidateTraceSources(configuration);
}

} // namespace v1

} // namespace Config
} // namespace SilKit
