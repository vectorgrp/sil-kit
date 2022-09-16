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

#include <fmt/ostream.h>
#include <type_traits>
#include <tuple>

// Include all operator<<(std::ostream&...) for libfmt here
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/logging/string_utils.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/can/string_utils.hpp"
#include "silkit/services/ethernet/string_utils.hpp"
#include "silkit/services/flexray/string_utils.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/services/pubsub/string_utils.hpp"
#include "silkit/services/rpc/string_utils.hpp"

#include "string_utils_sync.hpp"
#include "string_utils_internal.hpp"
#include "ServiceDatatypes.hpp"
#include "LoggingDatatypesInternal.hpp"

#include "WireCanMessages.hpp"
#include "WireEthernetMessages.hpp"
#include "WireFlexrayMessages.hpp"
#include "WireLinMessages.hpp"
#include "WireRpcMessages.hpp"

#include "TestDataTypes.hpp" // for operator<<
#include "IServiceEndpoint.hpp" // for operator<<(... ServiceDescriptor)

