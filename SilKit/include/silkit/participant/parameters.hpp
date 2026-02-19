// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/Parameters.h"

namespace SilKit {

// note: never reuse old numbers!
//! \brief Available parameters to query
enum class Parameter : SilKit_Parameter
{
    //! An undefined parameter
    Undefined = SilKit_Parameter_Undefined,
    //! The name of the participant
    ParticipantName = SilKit_Parameter_ParticipantName,
    //! The registry URI
    RegistryUri = SilKit_Parameter_ReistryUri,
};

} // namespace SilKit
