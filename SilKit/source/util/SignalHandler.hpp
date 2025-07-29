// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <functional>

namespace SilKit {
namespace Util {

//! \brief RegisterSignalHandler can be used to portably register a single signal handler.
// It only relies on async-signal-safe C functions internally, but
// it uses a dedicated thread which safely runs the user-provided handler.

using SignalHandler = std::function<void(int)>;
void RegisterSignalHandler(SignalHandler handler);
void ShutdownSignalHandler();

} // namespace Util
} // namespace SilKit
