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

#include "SilKit.hpp"

#include "Validation.hpp"
#include "CreateParticipant.hpp"
#include "ParticipantConfiguration.hpp"


namespace SilKit {

auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                       const std::string& participantName)
    -> std::unique_ptr<IParticipant>
{
    const auto uri = dynamic_cast<Config::ParticipantConfiguration&>(*participantConfig).middleware.registryUri;
    return CreateParticipant(participantConfig, participantName, uri);
}

SilKitAPI auto CreateParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                         const std::string& participantName, const std::string& registryUri)
    -> std::unique_ptr<IParticipant>
{
    auto participant = Core::CreateParticipantImpl(std::move(participantConfig), participantName);
    participant->JoinSilKitSimulation(registryUri);
    return participant;
}

}//namespace SilKit

