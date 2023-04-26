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

#include "CreateParticipantImpl.hpp"
#include "CreateSilKitRegistryImpl.hpp"
#include "ParticipantConfigurationFromXImpl.hpp"
#include "SilKitVersionImpl.hpp"

#include "participant/ParticipantExtensionsImpl.hpp"
#include "services/lin/LinControllerExtensionsImpl.hpp"

#include "silkit/capi/SilKitMacros.h"
#include "silkit/participant/IParticipant.hpp"
#include "silkit/experimental/services/lin/LinDatatypesExtensions.hpp"
#include "silkit/vendor/ISilKitRegistry.hpp"


namespace SilKit {
namespace Config {

SilKitAPI auto ParticipantConfigurationFromString(const std::string& text)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    return ParticipantConfigurationFromStringImpl(text);
}

SilKitAPI auto ParticipantConfigurationFromFile(const std::string& filename)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    return ParticipantConfigurationFromFileImpl(filename);
}

} // namespace Config
} // namespace SilKit


namespace SilKit {
namespace Experimental {
namespace Participant {

SilKitAPI auto CreateSystemController(IParticipant* participant)
    -> SilKit::Experimental::Services::Orchestration::ISystemController*
{
    return CreateSystemControllerImpl(participant);
}

} // namespace Participant
} // namespace Experimental
} // namespace SilKit


namespace SilKit {
namespace Experimental {
namespace Services {
namespace Lin {

SilKitAPI auto AddLinSlaveConfigurationHandler(
    SilKit::Services::Lin::ILinController* linController,
    SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler handler) -> SilKit::Util::HandlerId
{
    return AddLinSlaveConfigurationHandlerImpl(linController, std::move(handler));
}

SilKitAPI void RemoveLinSlaveConfigurationHandler(SilKit::Services::Lin::ILinController* linController,
                                                  SilKit::Util::HandlerId handlerId)
{
    return RemoveLinSlaveConfigurationHandlerImpl(linController, handlerId);
}

SilKitAPI auto GetSlaveConfiguration(SilKit::Services::Lin::ILinController* linController)
    -> SilKit::Experimental::Services::Lin::LinSlaveConfiguration
{
    return GetSlaveConfigurationImpl(linController);
}

} // namespace Lin
} // namespace Services
} // namespace Experimental
} // namespace SilKit


namespace SilKit {
namespace Vendor {
namespace Vector {

SilKitAPI auto CreateSilKitRegistry(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config)
    -> std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry>
{
    return CreateSilKitRegistryImpl(std::move(config));
}

} // namespace Vector
} // namespace Vendor
} // namespace SilKit


namespace SilKit {
namespace Version {

SilKitAPI auto Major() -> uint32_t
{
    return MajorImpl();
}

SilKitAPI auto Minor() -> uint32_t
{
    return MinorImpl();
}

SilKitAPI auto Patch() -> uint32_t
{
    return PatchImpl();
}

SilKitAPI auto BuildNumber() -> uint32_t
{
    return BuildNumberImpl();
}

SilKitAPI auto String() -> const char*
{
    return StringImpl();
}

SilKitAPI auto VersionSuffix() -> const char*
{
    return VersionSuffixImpl();
}

SilKitAPI auto GitHash() -> const char*
{
    return GitHashImpl();
}

} // namespace Version
} // namespace SilKit


namespace SilKit {

SilKitAPI auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                 const std::string& participantName, const std::string& registryUri)
    -> std::unique_ptr<IParticipant>
{
    return CreateParticipantImpl(std::move(participantConfig), participantName, registryUri);
}

SilKitAPI auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                 const std::string& participantName) -> std::unique_ptr<IParticipant>
{
    return CreateParticipantImpl(std::move(participantConfig), participantName);
}

} // namespace SilKit
