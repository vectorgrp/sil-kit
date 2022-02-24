// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <map>
#include <cstdint>
#include <iterator>

#include "ib/extensions/TraceMessage.hpp"
#include "ib/extensions/ITraceMessageSink.hpp" //for 'enum class Direction'
#include "ib/mw/EndpointAddress.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/sim/datatypes.hpp"

namespace ib {
namespace extensions {
//forwards
class IReplayMessage;
class IReplayChannel;
class IReplayFile;

//for use in DLL 
class IReplayDataProvider
{
public:
    virtual ~IReplayDataProvider() = default;
    //!< Pass the config (containing search path hints), the actual file to open
    //   and a logger to the extension.
    // TODO
    virtual auto OpenFile(/*const ib::cfg::Config& config,*/
        const std::string& filePath,
        ib::mw::logging::ILogger* ibLogger) -> std::shared_ptr<IReplayFile> = 0;
};

class IReplayFile
{
public:
    enum class FileType
    {
        PcapFile,
        Mdf4File
    };

    virtual ~IReplayFile() = default;
    //! Get the filesystem path of the replay file
    virtual auto FilePath() const -> const std::string& = 0;
    //! Returns embedded VIB config or empty string for non-VIB replay files
    virtual auto IntegrationBusConfig() const -> std::string = 0;

    //! Returns the file format type
    virtual FileType Type() const = 0;

    //! Use iterators to get access to the data channels of the file
    virtual std::vector<std::shared_ptr<IReplayChannel>>::iterator begin() = 0;
    virtual std::vector<std::shared_ptr<IReplayChannel>>::iterator end() = 0;
};

//! Interface shared among all ReplayMessage types.

//! Use a dynamic_cast to VIB message type to get actual data
class IReplayMessage
{
public:
    virtual ~IReplayMessage() = default;

    //! The timestamp associated with the replay message.
    virtual auto Timestamp() const -> std::chrono::nanoseconds = 0;
    //! The recorded direction of the replay message.
    virtual auto GetDirection() const -> ib::sim::TransmitDirection = 0;
    //! The endpoint address of the recording service.
    //! If unavailable from the underlying ReplayChannel, default value is returned.
    virtual auto EndpointAddress() const -> ib::mw::EndpointAddress = 0;
    //! Get the replay messages type, which is similar to the type used during tracing.
    virtual auto Type() const -> ib::extensions::TraceMessageType = 0;
};

class IReplayChannelReader
{
public:
    virtual ~IReplayChannelReader() = default;

    //! seek to the given message number, relative to current position
    virtual bool Seek(size_t messageNumber) = 0;
    virtual std::shared_ptr<IReplayMessage> Read() = 0;
};


class IReplayChannel
{
public:
    virtual ~IReplayChannel() = default;

    // Stream information
    //! Generic infos
    virtual auto Type() const -> ib::extensions::TraceMessageType = 0;
    virtual auto StartTime() const -> std::chrono::nanoseconds = 0;
    virtual auto EndTime() const -> std::chrono::nanoseconds = 0;
    virtual auto NumberOfMessages() const -> uint64_t = 0;
    //! a unique name suitable to identify this channel within its parent ReplayFile
    virtual auto Name() const -> const std::string& = 0;
    //!  File format specific meta data
    virtual auto GetMetaInfos() const -> const std::map<std::string, std::string>& = 0;

    //!  Get a reader instance that allows reading through the channel sequentially.
    virtual auto GetReader() -> std::shared_ptr<IReplayChannelReader> = 0;
};


} //end namespace extensions
} //end namespace ib
