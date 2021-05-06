// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <stdexcept>

namespace ib { namespace extensions {
	//! \brief ExtensionError is thrown when an extension could not be loaded
	class ExtensionError : public std::runtime_error
	{
        using std::runtime_error::runtime_error;
	};
}//end namespace extensions
}//end namespace ib

