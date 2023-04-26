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

#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"
#include "silkit/SilKitMacros.hpp"

#include "extensions/SilKitExtensionImpl/CreateMdf4Tracing.hpp"

/*! \brief Dummy compilation unit to pull in exports from other silkit libs
 *
 *  If you are missing an export from a silkit lib, just add a dummy
 *  function that uses the missing methods or functions.
 */

void __silkit_force_include_extensions_mdf4()
{
    SilKit::CreateMdf4Tracing({}, nullptr, "", "");
    SilKit::CreateMdf4Replay({}, nullptr, "");
}

void __silkit_force_include_participant_configuration()
{
    auto foo1 = SilKit::Config::ParticipantConfigurationFromString("");
    auto foo2 = SilKit::Config::ParticipantConfigurationFromFile("");
}

void __silkit_force_include_experimental()
{
    // Participant extensions
    auto systemController = SilKit::Experimental::Participant::CreateSystemController(nullptr);
    SILKIT_UNUSED_ARG(systemController);

    // LinController extensions
    auto handlerId =
        SilKit::Experimental::Services::Lin::AddLinSlaveConfigurationHandler(nullptr, nullptr);
    SILKIT_UNUSED_ARG(handlerId);
    SilKit::Experimental::Services::Lin::RemoveLinSlaveConfigurationHandler(nullptr, SilKit::Util::HandlerId{});
    auto slaveConfig = SilKit::Experimental::Services::Lin::GetSlaveConfiguration(nullptr);
    SILKIT_UNUSED_ARG(slaveConfig);
}
