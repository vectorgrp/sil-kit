// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <tuple>
#include <vector>

#include "silkit/services/i2c/II2cController.hpp"

#include "ITimeConsumer.hpp"
#include "IMsgForI2cController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "ParticipantConfiguration.hpp"

#include "SimBehavior.hpp"

#include "SynchronizedHandlers.hpp"

#include "LoggerMessage.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

class I2cController
    : public II2cController
    , public IMsgForI2cController
    , public ITraceMessageSource
    , public Core::IServiceEndpoint
{
public:
    I2cController() = delete;
    I2cController(const I2cController&) = delete;
    I2cController(I2cController&&) = delete;
    I2cController(Core::IParticipantInternal* participant, SilKit::Config::I2cController config,
                  Services::Orchestration::ITimeProvider* timeProvider);

    I2cController& operator=(I2cController& other) = delete;
    I2cController& operator=(I2cController&& other) = delete;

public:
    // II2cController
    void Init(I2cControllerConfig config) override;
    void SendFrame(const I2cFrame& frame, void* userContext = nullptr) override;
    void SetReadResponse(Util::Span<const uint8_t> data) override;

    auto AddFrameHandler(FrameHandler handler,
                         DirectionMask directionMask = (DirectionMask)TransmitDirection::RX) -> HandlerId override;
    void RemoveFrameHandler(HandlerId handlerId) override;
    auto AddFrameTransmitHandler(FrameTransmitHandler handler) -> HandlerId override;
    void RemoveFrameTransmitHandler(HandlerId handlerId) override;

    // IMsgForI2cController
    void ReceiveMsg(const IServiceEndpoint* from, const WireI2cFrameEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const I2cAcknowledge& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const WireI2cControllerConfig& msg) override;

    // ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType networkType) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

public:
    void RegisterServiceDiscovery();

    void SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor);
    void SetTrivialBehavior();

    auto GetControllerMode() const -> I2cControllerMode;
    auto GetTracer() -> Tracer*;

private:
    template <typename MsgT>
    using FilterT = std::function<bool(const MsgT& msg)>;

    template <typename MsgT>
    struct FilteredCallback
    {
        CallbackT<MsgT> callback;
        FilterT<MsgT> filter;

        void operator()(II2cController* controller, const MsgT& msg) const
        {
            if (!filter || filter(msg))
            {
                callback(controller, msg);
            }
        }
    };

private:
    template <typename MsgT>
    HandlerId AddHandler(CallbackT<MsgT> handler, FilterT<MsgT> filter = nullptr);

    template <typename MsgT>
    bool RemoveHandler(HandlerId handlerId);

    template <typename MsgT>
    void CallHandlers(const MsgT& msg);

    auto IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool;
    auto AllowReception(const IServiceEndpoint* from) const -> bool;

    template <typename MsgT>
    inline void SendMsg(MsgT&& msg);

private:
    Core::IParticipantInternal* _participant = nullptr;
    Config::I2cController _config;
    SimBehavior _simulationBehavior;
    Core::ServiceDescriptor _serviceDescriptor;
    Tracer _tracer;
    Services::Logging::ILogger* _logger;

    I2cControllerConfig _controllerConfig{};
    Util::SharedVector<uint8_t> _readResponseData;

    template <typename MsgT>
    using FilteredCallbacks = Util::SynchronizedHandlers<FilteredCallback<MsgT>>;

    std::tuple<FilteredCallbacks<I2cFrameEvent>, FilteredCallbacks<I2cFrameTransmitEvent>> _callbacks;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void I2cController::AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType /*networkType*/)
{
    _tracer.AddSink(GetServiceDescriptor(), *sink);
}

void I2cController::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto I2cController::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto I2cController::GetTracer() -> Tracer*
{
    return &_tracer;
}

} // namespace I2c
} // namespace Services
} // namespace SilKit
