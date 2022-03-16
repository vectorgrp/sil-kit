// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <functional>

#include "ib/extensions/CreateExtension.hpp"
#include "ib/mw/logging/ILogger.hpp"

#include "IbExtensions.hpp"
#include "IIbRegistry.hpp"
#include "FactorySingleton.hpp"
#include "ParticipantConfiguration.hpp"

namespace {
class EmptyLogger : public ib::mw::logging::ILogger
{
public:
    void Log(ib::mw::logging::Level /*level*/, const std::string& /*msg*/) override {}
    void Trace(const std::string& /*msg*/) override {}
    void Debug(const std::string& /*msg*/) override {}
    void Info(const std::string& /*msg*/) override {}
    void Warn(const std::string& /*msg*/) override {}
    void Error(const std::string& /*msg*/) override {}
    void Critical(const std::string& /*msg*/) override {}
    void RegisterRemoteLogging(const LogMsgHandlerT& /*handler*/) {}
    void LogReceivedMsg(const ib::mw::logging::LogMsg& /*msg*/) {}
protected:
    bool ShouldLog(ib::mw::logging::Level) const override { return true; }

};
} // end namespace
namespace ib {
namespace extensions {

auto CreateIbRegistry(std::shared_ptr<ib::cfg::IParticipantConfiguration> config)
    -> std::unique_ptr<IIbRegistry>
{
    EmptyLogger logger;
    return CreateIbRegistry(&logger, config);
}


auto CreateIbRegistry(mw::logging::ILogger* logger,
    std::shared_ptr<ib::cfg::IParticipantConfiguration> config)
    -> std::unique_ptr<IIbRegistry>
{
    auto cfg = std::dynamic_pointer_cast<cfg::ParticipantConfiguration>(config);
    auto& factory = FactorySingleton<IIbRegistryFactory>(logger, "vib-registry", cfg->extensions);
    return factory.Create(std::move(cfg));
}

} // namespace extensions
} // namespace ib
