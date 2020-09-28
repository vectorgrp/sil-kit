// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include <functional>

#
namespace ib { namespace registry  {
//! \brief RegisterSignalHandler can be used to portably register a single signal handler.
// It only relies on async-signal-safe C functions internally, but 
// it uses a dedicated thread which safely runs the user-provided handler.

using SignalHandlerT = std::function<void(int)>;
void RegisterSignalHandler(SignalHandlerT handler);


} // registry
} // ib
