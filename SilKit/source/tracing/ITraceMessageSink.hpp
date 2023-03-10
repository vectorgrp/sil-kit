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

#include <string>
#include <memory>
#include <chrono>
#include <tuple>

#include "silkit/services/logging/ILogger.hpp"

#include "ServiceDescriptor.hpp"
#include "TraceMessage.hpp"
#include "ParticipantConfiguration.hpp"

namespace SilKit {

//! \brief SinkType specifies the type of the output backend to use for permanent storage.
enum class SinkType
{
    PcapFile,
    PcapNamedPipe,
    Mdf4File
};

//! \brief Messages traces are written to a message sink.
//         It might be provided by an SIL Kit extension or built into SilKit

class ITraceMessageSink
{
public:
    virtual ~ITraceMessageSink() = default;

    virtual void Open(SinkType type, const std::string& outputPath) = 0;
    virtual void Close() = 0;
    virtual auto GetLogger() const -> Services::Logging::ILogger* = 0;
    virtual auto Name() const -> const std::string& = 0;

    virtual void Trace(
        SilKit::Services::TransmitDirection dir,
        const Core::ServiceDescriptor& address, //!< the address is used to identify the controller this message is from
        std::chrono::nanoseconds timestamp, const TraceMessage& message) = 0;
};

//! \brief Helper class to instantiate a trace message sink from a shared library module.
class ITraceMessageSinkFactory
{
public:
    virtual ~ITraceMessageSinkFactory() = default;
    virtual auto Create(SilKit::Config::ParticipantConfiguration config,
                        SilKit::Services::Logging::ILogger* logger, std::string participantName, std::string sinkName)
        -> std::unique_ptr<ITraceMessageSink> = 0;
};

} // namespace SilKit
