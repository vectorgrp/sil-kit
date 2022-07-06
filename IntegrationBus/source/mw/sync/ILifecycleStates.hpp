// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <string>

namespace SilKit {
namespace Core {
namespace Orchestration {

class ILifecycleState
{
public:
    virtual ~ILifecycleState() = default;
    virtual void RunSimulation(std::string reason) = 0;
    virtual void PauseSimulation(std::string reason) = 0;
    virtual void ContinueSimulation(std::string reason) = 0;

    virtual void StopNotifyUser(std::string reason) = 0;
    virtual void StopHandlerDone(std::string reason) = 0;

    virtual void Restart(std::string reason) = 0;

    virtual void ShutdownNotifyUser(std::string reason) = 0;
    virtual void ShutdownHandlerDone(std::string reason) = 0;

    virtual void AbortSimulation(std::string reason) = 0;
    virtual void Error(std::string reason) = 0;

    virtual void NewSystemState(SystemState systemState) = 0;

    virtual auto toString() -> std::string = 0;
    virtual auto GetParticipantState() -> ParticipantState = 0;
};

} // namespace Orchestration
} // namespace Core
} // namespace SilKit
