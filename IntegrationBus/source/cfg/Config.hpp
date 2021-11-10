// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "ib/cfg/Config.hpp"

namespace ib {
namespace cfg {

//!< Apply postprocessing steps to the config, like assigning endpoint addresses and applying legacy fixups.
void PostProcess(Config& config);
void UpdateGenericSubscribers(Config& config);

} // namespace cfg
} // namespace ib
