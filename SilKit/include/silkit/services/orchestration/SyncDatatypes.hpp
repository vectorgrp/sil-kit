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

#include <chrono>
#include <string>
#include <vector>

#include "silkit/capi/Orchestration.h"

namespace SilKit {
    
/*! \brief Deprecated identifier for SIL Kit participants
 * Will be fully replaced by participant name in future versions. 
*/
using ParticipantId = uint64_t;

namespace Services {
namespace Orchestration {

// note: never reuse old numbers!
//! \brief Available participant states
enum class ParticipantState : SilKit_ParticipantState
{
    Invalid                     =   0, //!< An invalid participant state
    ServicesCreated             =  10, //!< The controllers created state
    CommunicationInitializing   =  20, //!< The communication initializing state
    CommunicationInitialized    =  30, //!< The communication initialized state
    ReadyToRun                  =  40, //!< The initialized state
    Running                     =  50, //!< The running state
    Paused                      =  60, //!< The paused state
    Stopping                    =  70, //!< The stopping state
    Stopped                     =  80, //!< The stopped state
    Error                       =  90, //!< The error state
    ShuttingDown                = 100, //!< The shutting down state
    Shutdown                    = 110, //!< The shutdown state
};

//! \brief Details about a participant state change.
struct ParticipantStatus
{
    std::string participantName; //!< Name of the participant.
    ParticipantState state{ParticipantState::Invalid}; //!< The new state of the participant.
    std::string enterReason; //!< The reason for the participant to enter the new state.
    std::chrono::system_clock::time_point enterTime; //!< The enter time of the participant.
    std::chrono::system_clock::time_point refreshTime; //!< The refresh time.
};

// note: never reuse old numbers!
//! \brief Available system states
enum class SystemState : SilKit_SystemState
{
    Invalid                     =   0, //!< An invalid participant state
    ServicesCreated             =  10, //!< The controllers created state
    CommunicationInitializing   =  20, //!< The communication initializing state
    CommunicationInitialized    =  30, //!< The communication initialized state
    ReadyToRun                  =  40, //!< The initialized state
    Running                     =  50, //!< The running state
    Paused                      =  60, //!< The paused state
    Stopping                    =  70, //!< The stopping state
    Stopped                     =  80, //!< The stopped state
    Error                       =  90, //!< The error state
    ShuttingDown                = 100, //!< The shutting down state
    Shutdown                    = 110, //!< The shutdown state
};

//! \brief Details of the simulation workflow regarding lifecycle and participant coordination.
struct WorkflowConfiguration
{
    std::vector<std::string> requiredParticipantNames; //!< Participants that are waited for when coordinating the simulation start/stop.
};

//! \brief Available operation modes of the lifecycle service
enum class OperationMode : SilKit_OperationMode
{
    Invalid = 0,
    Coordinated = 10, 
    Autonomous = 20
};

//!< The lifecycle start configuration.
struct LifecycleConfiguration
{
    OperationMode operationMode;
};

struct ParticipantConnectionInformation
{
    std::string participantName;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
