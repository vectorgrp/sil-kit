#pragma once

#include "ib/sim/can/ICanController.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"

#include "IIbToCanControllerFacade.hpp"
#include "IComAdapterInternal.hpp"
#include "IIbServiceEndpoint.hpp"

#include "CanController.hpp"
#include "ParticipantConfiguration.hpp"
#include "CanControllerProxy.hpp"

namespace ib {
namespace sim {
namespace can {

class CanControllerFacade
    : public ICanController
    , public IIbToCanControllerFacade
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    CanControllerFacade() = delete;
    CanControllerFacade(CanControllerFacade&&) = default;
    CanControllerFacade(mw::IComAdapterInternal* comAdapter, ib::cfg::CanController config,
                        mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    CanControllerFacade& operator=(CanControllerFacade&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ICanController
    void SetBaudRate(uint32_t rate, uint32_t fdRate) override;

    void Reset() override;
    void Start() override;
    void Stop() override;
    void Sleep() override;

    auto SendMessage(const CanMessage& msg, void* userContext = nullptr) -> CanTxId override;
    auto SendMessage(CanMessage&& msg, void* userContext = nullptr) -> CanTxId override;

    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler,
                                       DirectionMask directionMask = (DirectionMask)TransmitDirection::RX
                                                                     | (DirectionMask)TransmitDirection::TX) override;
    void RegisterStateChangedHandler(StateChangedHandler handler) override;
    void RegisterErrorStateChangedHandler(ErrorStateChangedHandler handler) override;
    void RegisterTransmitStatusHandler(
        MessageStatusHandler handler,
        CanTransmitStatusMask statusMask = (CanTransmitStatusMask)CanTransmitStatus::Transmitted
                                           | (CanTransmitStatusMask)CanTransmitStatus::Canceled
                                           | (CanTransmitStatusMask)CanTransmitStatus::DuplicatedTransmitId
                                           | (CanTransmitStatusMask)CanTransmitStatus::TransmitQueueFull) override;

    // IIbToCanController / IIbToCanControllerProxy
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanMessage& msg) override;
    // IIbToCanControllerProxy only
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanControllerStatus& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanTransmitAcknowledge& msg) override;

    //ITimeConsumer
    void SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider) override;

    //ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const mw::ServiceDescriptor& override;

private:
    // ----------------------------------------
    // Private helper methods
    //
    auto AllowForwardToDefault(const IIbServiceEndpoint* from) const -> bool;
    auto AllowForwardToProxy(const IIbServiceEndpoint* from) const -> bool;
    auto IsNetworkSimulated() const -> bool;
    auto IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool;

private:
    mw::IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceDescriptor _serviceDescriptor;

    bool _simulatedLinkDetected = false;
    mw::ServiceDescriptor _simulatedLink;

    ICanController* _currentController;
    cfg::CanController _config;
    std::unique_ptr<CanController> _canController;
    std::unique_ptr<CanControllerProxy> _canControllerProxy;
};

} // namespace can
} // namespace sim
} // namespace ib
