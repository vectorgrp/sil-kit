// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OatppHeaders.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

ENUM(ParticipantState, v_int32,                                         //
     VALUE(Unknown, -1, "unknown"),                                     //
     VALUE(Invalid, 0, "invalid"),                                      //
     VALUE(ServicesCreated, 10, "servicescreated"),                     //
     VALUE(CommunicationInitializing, 20, "communicationinitializing"), //
     VALUE(CommunicationInitialized, 30, "communicationinitialized"),   //
     VALUE(ReadyToRun, 40, "readytorun"),                               //
     VALUE(Running, 50, "running"),                                     //
     VALUE(Paused, 60, "paused"),                                       //
     VALUE(Stopping, 70, "stopping"),                                   //
     VALUE(Stopped, 80, "stopped"),                                     //
     VALUE(Error, 90, "error"),                                         //
     VALUE(ShuttingDown, 100, "shuttingdown"),                          //
     VALUE(Shutdown, 110, "shutdown"),                                  //
     VALUE(Aborting, 120, "aborting"))

class ParticipantStatusDto : public oatpp::DTO
{
    DTO_INIT(ParticipantStatusDto, DTO)

    DTO_FIELD_INFO(state)
    {
        info->description = "Name of the state";
    }
    DTO_FIELD(Enum<ParticipantState>::AsString, state);

    DTO_FIELD_INFO(enterReason)
    {
        info->description = "Reason for entering the state";
    }
    DTO_FIELD(String, enterReason);

    DTO_FIELD_INFO(enterTime)
    {
        info->description = "Time when state got entered";
    }
    DTO_FIELD(UInt64, enterTime);
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)