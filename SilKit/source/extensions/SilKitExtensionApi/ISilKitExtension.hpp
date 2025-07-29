// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

namespace SilKit {

//! \brief The interface for all SIL Kit extensions.

class ISilKitExtension
{
public:
    virtual ~ISilKitExtension() = default;
    virtual const char* GetExtensionName() const = 0;
    virtual const char* GetVendorName() const = 0;
    virtual void GetVersion(uint32_t& major, uint32_t& minor, uint32_t& patch) const = 0;
};


} //end namespace SilKit
