// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OatppHeaders.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

namespace SilKit {
namespace Dashboard {

ENUM(SystemState, v_int32,                                              //
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

class SystemStatusDto : public oatpp::DTO
{
    DTO_INIT(SystemStatusDto, DTO)

    DTO_FIELD_INFO(state)
    {
        info->description = "Name of the state";
    }
    DTO_FIELD(Enum<SystemState>::AsString, state);
};

} // namespace Dashboard
} // namespace SilKit

#include OATPP_CODEGEN_END(DTO)