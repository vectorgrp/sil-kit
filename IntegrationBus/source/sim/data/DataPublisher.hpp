// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>

#include "silkit/core/fwd_decl.hpp"
#include "silkit/services/pubsub/IDataPublisher.hpp"
#include "ITimeConsumer.hpp"

#include "IMsgForDataPublisher.hpp"
#include "IParticipantInternal.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class DataPublisher
    : public IDataPublisher
    , public IMsgForDataPublisher
    , public Core::Orchestration::ITimeConsumer
    , public Core::IServiceEndpoint
{
public:
    DataPublisher(Core::IParticipantInternal* participant, Core::Orchestration::ITimeProvider* timeProvider, const std::string& topic,
                  const std::string& mediaType, const std::map<std::string, std::string>& labels,
                  const std::string& pubUUID);
    
    void Publish(std::vector<uint8_t> data) override;
    void Publish(const uint8_t* data, std::size_t size) override;

    //SilKit::Core::Orchestration::ITimeConsumer
    void SetTimeProvider(Core::Orchestration::ITimeProvider* provider) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;


private:
    std::string _topic;
    std::string _mediaType;
    std::map<std::string, std::string> _labels;
    std::string _pubUUID;

    Core::ServiceDescriptor _serviceDescriptor{};
    Core::Orchestration::ITimeProvider* _timeProvider{nullptr};
    Core::IParticipantInternal* _participant{nullptr};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void DataPublisher::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto DataPublisher::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace PubSub
} // namespace Services
} // namespace SilKit
