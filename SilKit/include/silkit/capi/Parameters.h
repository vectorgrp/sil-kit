// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <stdint.h>
#include "SilKitMacros.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

/*! Internal parameter provider. */
typedef struct SilKit_ParamterProvider SilKit_ParamterProvider;

/*! A parameter set by an API call and/or the participant configuration. */
typedef int16_t SilKit_Parameter;

/*! An undefined parameter */
#define SilKit_Parameter_Undefined ((SilKit_Parameter)0)
/*! The name of the participant */
#define SilKit_Parameter_ParticipantName ((SilKit_Parameter)1)
/*! The registry URI */
#define SilKit_Parameter_ReistryUri ((SilKit_Parameter)2)

SILKIT_END_DECLS

#pragma pack(pop)
