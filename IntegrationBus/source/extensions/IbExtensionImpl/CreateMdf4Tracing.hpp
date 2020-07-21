// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>

#include "ib/extensions/ITraceMessageSink.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/cfg/fwd_decl.hpp"


namespace ib { namespace extensions {
//forwards
class IIbExtension;

//! \brief Helper factory to instantiate Mdf4tracing
//         Required because we don't have control over the CTor when an extension is loaded.

class Mdf4TraceSinkFactory
{
public:
    virtual ~Mdf4TraceSinkFactory() = default;
    virtual auto Create(cfg::Config config,
            ib::mw::logging::ILogger* logger,
            std::string name
        )
       -> std::unique_ptr<ITraceMessageSink> = 0;

};

auto CreateMdf4Tracing(cfg::Config config,
    ib::mw::logging::ILogger* logger, const std::string& sinkName)
    -> std::unique_ptr<ITraceMessageSink>;

}//end namespace extensions
}//end namespace ib
