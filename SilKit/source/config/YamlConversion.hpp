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

#pragma once

#include <chrono>
#include <string>

#include "silkit/participant/exception.hpp"

#include "Configuration.hpp"
#include "ParticipantConfiguration.hpp"
#include "rapidyaml.hpp"

#include "rapidyaml.hpp"

//#include "SilKitYamlHelper.hpp"


namespace VSilKit {
namespace Config {


struct DesCtx
{
    ryml::Parser& parser;
    ryml::Tree& tree;
    ryml::ConstNodeRef node;

    template <typename T>
    auto operator>>(T& t) -> DesCtx&
    {
        this->node >> t;
        return *this;
    }

    auto IsStr(const char* str) const -> bool
    {
        if (!this->node.is_val())
        {
            return false;
        }

        return this->node.val() == ryml::to_csubstr(str);
    }

    auto IsMap() const -> bool
    {
        return this->node.is_map();
    }

    auto IsEmpty() const -> bool
    {
        return this->node.empty();
    }

    auto HasKeyValue(const char* key) const -> bool
    {
        if (!this->node.is_map())
        {
            return false;
        }

        return this->node.has_child(key);
    }

    auto GetKeyValue(const char* key) const -> DesCtx
    {
        if (!this->node.is_map())
        {
            throw MakeConfigurationError("expected node to be map");
        }

        auto ctx = *this;
        ctx.node = this->node.find_child(key);

        if (ctx.node.invalid())
        {
            throw MakeConfigurationError("key not found");
        }

        return ctx;
    }

    auto MakeConfigurationError(const char* message) const -> SilKit::ConfigurationError
    {
        const auto location = this->parser.location(this->node);

        std::ostringstream s;

        s << "error parsing configuration";

        if (location.name.empty())
        {
            s << " file " << location.name << ": ";
        }
        else
        {
            s << " string: ";
        }

        s << "line " << location.line << " column " << location.col << ": " << message;

        return SilKit::ConfigurationError{s.str()};
    }
};

struct SerCtx
{
    ryml::Tree& tree;
    ryml::NodeRef node;

    template <typename T>
    auto operator<<(const T& t) -> SerCtx&
    {
        this->node << t;
        return *this;
    }

    void TurnIntoMap()
    {
        this->node |= ryml::MAP;
    }

    auto StartKeyValue(const char* key) -> SerCtx
    {
        auto ctx = *this;
        ctx.node = (ctx.node << ryml::key(key));
        return ctx;
    }
};


void DesRoot(DesCtx ctx, SilKit::Config::ParticipantConfiguration& obj);


inline auto DeserializeParticipantConfiguration(const std::string& input) -> SilKit::Config::ParticipantConfiguration
{
    ryml::ParserOptions options{};
    options.locations(true);

    ryml::Callbacks cb{};
    ryml::EventHandlerTree eventHandler{cb};

    auto parser = ryml::Parser(&eventHandler, options);
    parser.reserve_locations(100u);

    auto tree = ryml::parse_in_arena(&parser, ryml::to_csubstr(input));
    auto node = tree.crootref();

    DesCtx ctx{parser, tree, node};

    SilKit::Config::ParticipantConfiguration obj;
    DesRoot(ctx, obj);
    return obj;
}


} // namespace Config
} // namespace VSilKit


#define DECLARE_READ_WRITE_FUNCS(TYPE) \
    void write(ryml::NodeRef* node, const TYPE& obj); \
    bool read(const ryml::ConstNodeRef& node, TYPE* obj);

// XXXXXXXXXX RAPID YML XXXXXXXXXXXXXX

namespace std {
namespace chrono {
DECLARE_READ_WRITE_FUNCS(milliseconds);
DECLARE_READ_WRITE_FUNCS(nanoseconds);
} // namespace chrono
} // namespace std

namespace SilKit {
namespace Services {
namespace Logging {
DECLARE_READ_WRITE_FUNCS(Services::Logging::Level);
} // namespace Logging
namespace Flexray {
DECLARE_READ_WRITE_FUNCS(Services::Flexray::FlexrayChannel);
DECLARE_READ_WRITE_FUNCS(Services::Flexray::FlexrayClockPeriod);
DECLARE_READ_WRITE_FUNCS(Services::Flexray::FlexrayTransmissionMode);
DECLARE_READ_WRITE_FUNCS(Services::Flexray::FlexrayClusterParameters);
DECLARE_READ_WRITE_FUNCS(Services::Flexray::FlexrayNodeParameters);
DECLARE_READ_WRITE_FUNCS(Services::Flexray::FlexrayTxBufferConfig);
} // namespace Flexray
} // namespace Services
namespace Config {
inline namespace v1 {
DECLARE_READ_WRITE_FUNCS(Sink);
DECLARE_READ_WRITE_FUNCS(Sink::Type);
DECLARE_READ_WRITE_FUNCS(Sink::Format);
DECLARE_READ_WRITE_FUNCS(Logging);
DECLARE_READ_WRITE_FUNCS(Metrics);
DECLARE_READ_WRITE_FUNCS(MetricsSink);
DECLARE_READ_WRITE_FUNCS(MetricsSink::Type);
DECLARE_READ_WRITE_FUNCS(MdfChannel);
DECLARE_READ_WRITE_FUNCS(Replay);
DECLARE_READ_WRITE_FUNCS(Replay::Direction);
DECLARE_READ_WRITE_FUNCS(CanController);
DECLARE_READ_WRITE_FUNCS(LinController);
DECLARE_READ_WRITE_FUNCS(EthernetController);
DECLARE_READ_WRITE_FUNCS(FlexrayController);
DECLARE_READ_WRITE_FUNCS(Label::Kind);
DECLARE_READ_WRITE_FUNCS(Label);
DECLARE_READ_WRITE_FUNCS(DataPublisher);
DECLARE_READ_WRITE_FUNCS(DataSubscriber);
DECLARE_READ_WRITE_FUNCS(RpcServer);
DECLARE_READ_WRITE_FUNCS(RpcClient);
DECLARE_READ_WRITE_FUNCS(Tracing);
DECLARE_READ_WRITE_FUNCS(TraceSink::Type);
DECLARE_READ_WRITE_FUNCS(TraceSink);
DECLARE_READ_WRITE_FUNCS(TraceSource::Type);
DECLARE_READ_WRITE_FUNCS(TraceSource);
DECLARE_READ_WRITE_FUNCS(Extensions);
DECLARE_READ_WRITE_FUNCS(Middleware);
DECLARE_READ_WRITE_FUNCS(Includes);
DECLARE_READ_WRITE_FUNCS(Aggregation);
DECLARE_READ_WRITE_FUNCS(TimeSynchronization);
DECLARE_READ_WRITE_FUNCS(Experimental);
DECLARE_READ_WRITE_FUNCS(ParticipantConfiguration);
DECLARE_READ_WRITE_FUNCS(HealthCheck);

} // namespace v1

} //end namespace Config
namespace Services {
DECLARE_READ_WRITE_FUNCS(MatchingLabel::Kind);
DECLARE_READ_WRITE_FUNCS(MatchingLabel);

} // namespace Services
} //end namespace SilKit

// XXXXXXXXXX END RAPID YML XXXXXXXXXXXXXX

#undef DECLARE_READ_WRITE_FUNCS
