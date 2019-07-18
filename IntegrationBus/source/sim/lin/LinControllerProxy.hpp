// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/lin/ILinController.hpp"
#include "ib/sim/lin/IIbToLinControllerProxy.hpp"

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "ib/mw/fwd_decl.hpp"

namespace spdlog {
class logger;
} // namespace spdlog

namespace ib {
namespace sim {
namespace lin {

class LinControllerProxy :
    public ILinController,
    public IIbToLinControllerProxy
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    LinControllerProxy() = delete;
    LinControllerProxy(const LinControllerProxy&) = default;
    LinControllerProxy(LinControllerProxy&&) = default;
    LinControllerProxy(mw::IComAdapter* comAdapter);

public:
    // ----------------------------------------
    // Operator Implementations
    LinControllerProxy& operator=(LinControllerProxy& other) = default;
    LinControllerProxy& operator=(LinControllerProxy&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ILinController
    void Init(ControllerConfig config) override;
    auto Status() const noexcept->ControllerStatus override;

    void SendFrame(Frame frame, FrameResponseType responseType) override;
    void SendFrameHeader(LinIdT linId) override;
    void SetFrameResponse(Frame frame, FrameResponseMode mode) override;
    void SetFrameResponses(std::vector<FrameResponse> responses) override;

    void GoToSleep() override;
    void GoToSleepInternal() override;
    void Wakeup() override;
    void WakeupInternal() override;

    void RegisterFrameStatusHandler(FrameStatusHandler handler) override;
    void RegisterGoToSleepHandler(GoToSleepHandler handler) override;
    void RegisterWakeupHandler(WakeupHandler handler) override;
    void RegisterFrameResponseUpdateHandler(FrameResponseUpdateHandler handler) override;

    // IIbToLinController
    void ReceiveIbMessage(mw::EndpointAddress from, const Transmission& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const WakeupPulse& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const ControllerConfig& msg) override;
    void ReceiveIbMessage(mw::EndpointAddress from, const FrameResponseUpdate& msg) override;

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

public:
    // ----------------------------------------
    // Public interface methods

private:
//    // ----------------------------------------
//    // private data types

private:
    // ----------------------------------------
    // private methods
    void SetControllerStatus(ControllerStatus status);

    template <typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

private:
    // ----------------------------------------
    // private members
    mw::IComAdapter* _comAdapter;
    mw::EndpointAddress _endpointAddr;
    std::shared_ptr<spdlog::logger> _logger;

    ControllerMode   _controllerMode{ControllerMode::Inactive};
    ControllerStatus _controllerStatus{ControllerStatus::Unknown};

    std::vector<FrameStatusHandler>         _frameStatusHandler;
    std::vector<GoToSleepHandler>           _goToSleepHandler;
    std::vector<WakeupHandler>              _wakeupHandler;
    std::vector<FrameResponseUpdateHandler> _frameResponseUpdateHandler;
};

// ================================================================================
//  Inline Implementations
// ================================================================================


} // namespace lin
} // namespace sim
} // namespace ib
