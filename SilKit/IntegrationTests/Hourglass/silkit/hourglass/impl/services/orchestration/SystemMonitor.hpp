#pragma once

#include "silkit/capi/Orchestration.h"

#include "silkit/participant/exception.hpp"
#include "silkit/services/orchestration/ISystemMonitor.hpp"

#include "silkit/hourglass/impl/Macros.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace Orchestration {

// NB: SystemMonitor is final here due to ISystemMonitor not having a virtual destructor (at the moment).

class SystemMonitor final : public SilKit::Services::Orchestration::ISystemMonitor
{
public:
    explicit SystemMonitor(SilKit_Participant *participant)
    {
        const auto returnCode = SilKit_SystemMonitor_Create(&_systemMonitor, participant);
        ThrowOnError(returnCode);
    }

#define SILKIT_THROW_NOT_IMPLEMENTED_(FUNCTION_NAME) \
    SILKIT_HOURGLASS_IMPL_THROW_NOT_IMPLEMENTED("Services::Orchestration::SystemMonitor", FUNCTION_NAME)

    auto AddSystemStateHandler(SystemStateHandler handler) -> Util::HandlerId override
    {
        const auto cHandler = [](void *context, SilKit_SystemMonitor *systemMonitor, SilKit_SystemState state) {
            SILKIT_UNUSED_ARG(systemMonitor);

            const auto data = static_cast<HandlerData<SystemStateHandler> *>(context);
            data->handler(static_cast<SilKit::Services::Orchestration::SystemState>(state));
        };

        SilKit_HandlerId handlerId;

        auto handlerData = std::make_unique<HandlerData<SystemStateHandler>>();
        handlerData->systemMonitor = this;
        handlerData->handler = std::move(handler);

        const auto returnCode =
            SilKit_SystemMonitor_AddSystemStateHandler(_systemMonitor, handlerData.get(), cHandler, &handlerId);
        ThrowOnError(returnCode);

        _systemStateHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

        return static_cast<SilKit::Services::HandlerId>(handlerId);
    }

    void RemoveSystemStateHandler(Util::HandlerId handlerId) override
    {
        const auto returnCode =
            SilKit_SystemMonitor_RemoveSystemStateHandler(_systemMonitor, static_cast<SilKit_HandlerId>(handlerId));
        ThrowOnError(returnCode);

        _systemStateHandlers.erase(handlerId);
    }

    auto AddParticipantStatusHandler(ParticipantStatusHandler handler) -> Util::HandlerId override
    {
        SILKIT_UNUSED_ARG(handler);
        SILKIT_THROW_NOT_IMPLEMENTED_("AddParticipantStatusHandler");
    }

    void RemoveParticipantStatusHandler(Util::HandlerId handlerId) override
    {
        SILKIT_UNUSED_ARG(handlerId);
        SILKIT_THROW_NOT_IMPLEMENTED_("RemoveParticipantStatusHandler");
    }

    auto SystemState() const -> SilKit::Services::Orchestration::SystemState override
    {
        SILKIT_THROW_NOT_IMPLEMENTED_("SystemState");
    }

    auto ParticipantStatus(const std::string &participantName) const
        -> SilKit::Services::Orchestration::ParticipantStatus const & override
    {
        SILKIT_UNUSED_ARG(participantName);
        SILKIT_THROW_NOT_IMPLEMENTED_("ParticipantStatus");
    }

    void SetParticipantConnectedHandler(ParticipantConnectedHandler handler) override
    {
        SILKIT_UNUSED_ARG(handler);
        SILKIT_THROW_NOT_IMPLEMENTED_("SetParticipantConnectedHandler");
    }

    void SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler) override
    {
        SILKIT_UNUSED_ARG(handler);
        SILKIT_THROW_NOT_IMPLEMENTED_("SetParticipantDisconnectedHandler");
    }

    auto IsParticipantConnected(const std::string &participantName) const -> bool override
    {
        SILKIT_UNUSED_ARG(participantName);
        SILKIT_THROW_NOT_IMPLEMENTED_("IsParticipantConnected");
    }

#undef SILKIT_THROW_NOT_IMPLEMENTED_

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::Orchestration::ISystemMonitor *systemMonitor{nullptr};
        HandlerFunction handler{};
    };

    template <typename HandlerFunction>
    using HandlerDataMap = std::unordered_map<SilKit::Util::HandlerId, std::unique_ptr<HandlerData<HandlerFunction>>>;

private:
    SilKit_SystemMonitor *_systemMonitor{nullptr};

    HandlerDataMap<SystemStateHandler> _systemStateHandlers;
};

} // namespace Orchestration
} // namespace Services
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
