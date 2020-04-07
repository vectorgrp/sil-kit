// Copyright (c) 2020 Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IbMacros.hpp"
#include "ib/extensions/IIbRegistry.hpp"
#include "ib/cfg/Config.hpp"
#include <memory>
#include <functional>

namespace ib { namespace extensions {

//! \brief Loads the IbRegistry extension from a shared library in the current
//!        working directory if necessary, and returns an instance of
//!        IIbRegistry to the caller. 
//Throws std::runtime_error on error.

IntegrationBusAPI auto CreateIbRegistry(ib::cfg::Config config)
    -> std::unique_ptr<IIbRegistry>;

}//end namespace extensions
}//end namespace ib
