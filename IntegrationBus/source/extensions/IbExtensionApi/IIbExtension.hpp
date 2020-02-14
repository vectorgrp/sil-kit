// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib { namespace extensions {

//! \brief The interface for all IB extensions.

class IIbExtension
{
public:
    virtual ~IIbExtension() = default;
    virtual const char* GetExtensionName() const = 0;
    virtual const char* GetVendorName() const = 0;
    virtual void GetVersion(uint32_t& major,
                    uint32_t& minor, uint32_t& patch) const = 0;
};  

}//end namespace extensions
}//end namespace ib
