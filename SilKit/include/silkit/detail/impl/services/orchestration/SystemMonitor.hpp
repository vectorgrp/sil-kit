// Copyright (c) 2023 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "silkit/capi/Orchestration.h"

#include "silkit/participant/exception.hpp"
#include "silkit/services/orchestration/ISystemMonitor.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Orchestration {

// NB: SystemMonitor is final here due to ISystemMonitor not having a virtual destructor (at the moment).

class SystemMonitor final : public SilKit::Services::Orchestration::ISystemMonitor
{
public:
    inline explicit SystemMonitor(SilKit_Participant *participant);

    inline auto AddSystemStateHandler(SystemStateHandler handler) -> Util::HandlerId override;

    inline void RemoveSystemStateHandler(Util::HandlerId handlerId) override;

    inline auto AddParticipantStatusHandler(ParticipantStatusHandler handler) -> Util::HandlerId override;

    inline void RemoveParticipantStatusHandler(Util::HandlerId handlerId) override;

    inline auto SystemState() const -> SilKit::Services::Orchestration::SystemState override;

    inline auto ParticipantStatus(const std::string &participantName) const
        -> SilKit::Services::Orchestration::ParticipantStatus const & override;

    inline void SetParticipantConnectedHandler(ParticipantConnectedHandler handler) override;

    inline void SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler) override;

    inline auto IsParticipantConnected(const std::string &participantName) const -> bool override;

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

    mutable SilKit::Services::Orchestration::ParticipantStatus _lastParticipantStatus;

    HandlerDataMap<SystemStateHandler> _systemStateHandlers;
    HandlerDataMap<ParticipantStatusHandler> _participantStatusHandlers;

    std::unique_ptr<ParticipantConnectedHandler> _participantConnectedHandler;
    std::unique_ptr<ParticipantDisconnectedHandler> _participantDisconnectedHandler;
};

} // namespace Orchestration
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/detail/impl/ThrowOnError.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Orchestration {

namespace {

inline void CToCxx(
    const SilKit_ParticipantConnectionInformation &cParticipantConnectionInformation,
    SilKit::Services::Orchestration::ParticipantConnectionInformation &cxxParticipantConnectionInformation);

} // namespace

SystemMonitor::SystemMonitor(SilKit_Participant *participant)
{
    const auto returnCode = SilKit_SystemMonitor_Create(&_systemMonitor, participant);
    ThrowOnError(returnCode);
}

auto SystemMonitor::AddSystemStateHandler(SystemStateHandler handler) -> Util::HandlerId
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

void SystemMonitor::RemoveSystemStateHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_SystemMonitor_RemoveSystemStateHandler(_systemMonitor, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _systemStateHandlers.erase(handlerId);
}

auto SystemMonitor::AddParticipantStatusHandler(ParticipantStatusHandler handler) -> Util::HandlerId
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    using time_point = decltype(SilKit::Services::Orchestration::ParticipantStatus::enterTime);
    using time_point_duration = typename time_point::duration;
    using duration = std::chrono::nanoseconds;

    const auto cHandler = [](void *context, SilKit_SystemMonitor *systemMonitor, char const *participantName,
                             SilKit_ParticipantStatus *cParticipantStatus) {
        SILKIT_UNUSED_ARG(systemMonitor);
        SILKIT_UNUSED_ARG(participantName);

        SilKit::Services::Orchestration::ParticipantStatus cxxParticipantStatus;
        cxxParticipantStatus.participantName = std::string{cParticipantStatus->participantName};
        cxxParticipantStatus.state =
            static_cast<SilKit::Services::Orchestration::ParticipantState>(cParticipantStatus->participantState);
        cxxParticipantStatus.enterReason = std::string{cParticipantStatus->enterReason};
        cxxParticipantStatus.enterTime =
            time_point{std::chrono::duration_cast<time_point_duration>(duration{cParticipantStatus->enterTime})};
        cxxParticipantStatus.refreshTime =
            time_point{std::chrono::duration_cast<time_point_duration>(duration{cParticipantStatus->enterTime})};

        const auto data = static_cast<HandlerData<ParticipantStatusHandler> *>(context);
        data->handler(cxxParticipantStatus);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<ParticipantStatusHandler>>();
    handlerData->systemMonitor = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_SystemMonitor_AddParticipantStatusHandler(_systemMonitor, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _participantStatusHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void SystemMonitor::RemoveParticipantStatusHandler(Util::HandlerId handlerId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode =
        SilKit_SystemMonitor_RemoveParticipantStatusHandler(_systemMonitor, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _participantStatusHandlers.erase(handlerId);
}

auto SystemMonitor::SystemState() const -> SilKit::Services::Orchestration::SystemState
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    SilKit_SystemState systemState;

    const auto returnCode = SilKit_SystemMonitor_GetSystemState(&systemState, _systemMonitor);
    ThrowOnError(returnCode);

    return static_cast<SilKit::Services::Orchestration::SystemState>(systemState);
}

auto SystemMonitor::ParticipantStatus(const std::string &participantName) const
    -> SilKit::Services::Orchestration::ParticipantStatus const &
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    SilKit_ParticipantStatus cParticipantStatus;
    SilKit_Struct_Init(SilKit_ParticipantStatus, cParticipantStatus);

    const auto returnCode =
        SilKit_SystemMonitor_GetParticipantStatus(&cParticipantStatus, _systemMonitor, participantName.c_str());
    ThrowOnError(returnCode);

    using time_point = decltype(SilKit::Services::Orchestration::ParticipantStatus::enterTime);
    using time_point_duration = typename time_point::duration;
    using duration = std::chrono::nanoseconds;

    // NB: Unfortunately the signature doesn't really allow for another implementation, since we are returning a
    //     reference to the ParticipantStatus structure.
    //
    //     Calling the ParticipantStatus function from multiple threads simultaneously will likely cause data-races.
    //
    //     The C function does not (necessarily) have this issue as it fills a user-provided structure. The current
    //     implementation however is also not thread-safe, as it returns a reference to a structure stored in a map,
    //     which could be updated from another thread.

    _lastParticipantStatus.participantName = std::string{cParticipantStatus.participantName};
    _lastParticipantStatus.state =
        static_cast<SilKit::Services::Orchestration::ParticipantState>(cParticipantStatus.participantState);
    _lastParticipantStatus.enterReason = std::string{cParticipantStatus.enterReason};
    _lastParticipantStatus.enterTime =
        time_point{std::chrono::duration_cast<time_point_duration>(duration{cParticipantStatus.enterTime})};
    _lastParticipantStatus.refreshTime =
        time_point{std::chrono::duration_cast<time_point_duration>(duration{cParticipantStatus.refreshTime})};

    return _lastParticipantStatus;
}

void SystemMonitor::SetParticipantConnectedHandler(ParticipantConnectedHandler handler)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    auto ownedHandlerPtr = std::make_unique<ParticipantConnectedHandler>(std::move(handler));

    const auto cHandler = [](void *context, SilKit_SystemMonitor *systemMonitor,
                             const SilKit_ParticipantConnectionInformation *cParticipantConnectionInformation) {
        SILKIT_UNUSED_ARG(systemMonitor);

        SilKit::Services::Orchestration::ParticipantConnectionInformation cxxParticipantConnectionInformation;
        CToCxx(*cParticipantConnectionInformation, cxxParticipantConnectionInformation);

        const auto handlerPtr = static_cast<ParticipantConnectedHandler *>(context);
        (*handlerPtr)(cxxParticipantConnectionInformation);
    };

    const auto returnCode =
        SilKit_SystemMonitor_SetParticipantConnectedHandler(_systemMonitor, ownedHandlerPtr.get(), cHandler);
    ThrowOnError(returnCode);

    _participantConnectedHandler = std::move(ownedHandlerPtr);
}

void SystemMonitor::SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    auto ownedHandlerPtr = std::make_unique<ParticipantDisconnectedHandler>(std::move(handler));

    const auto cHandler = [](void *context, SilKit_SystemMonitor *,
                             const SilKit_ParticipantConnectionInformation *cParticipantConnectionInformation) {
        SilKit::Services::Orchestration::ParticipantConnectionInformation cxxParticipantConnectionInformation;
        CToCxx(*cParticipantConnectionInformation, cxxParticipantConnectionInformation);

        const auto handlerPtr = static_cast<ParticipantDisconnectedHandler *>(context);
        (*handlerPtr)(cxxParticipantConnectionInformation);
    };

    const auto returnCode =
        SilKit_SystemMonitor_SetParticipantDisconnectedHandler(_systemMonitor, ownedHandlerPtr.get(), cHandler);
    ThrowOnError(returnCode);

    _participantDisconnectedHandler = std::move(ownedHandlerPtr);
}

auto SystemMonitor::IsParticipantConnected(const std::string &participantName) const -> bool
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    SilKit_Bool result;

    const auto returnCode =
        SilKit_SystemMonitor_IsParticipantConnected(_systemMonitor, participantName.c_str(), &result);
    ThrowOnError(returnCode);

    return result;
}

namespace {

void CToCxx(const SilKit_ParticipantConnectionInformation &cParticipantConnectionInformation,
            SilKit::Services::Orchestration::ParticipantConnectionInformation &cxxParticipantConnectionInformation)
{
    cxxParticipantConnectionInformation.participantName =
        std::string{cParticipantConnectionInformation.participantName};
}

} // namespace

} // namespace Orchestration
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
