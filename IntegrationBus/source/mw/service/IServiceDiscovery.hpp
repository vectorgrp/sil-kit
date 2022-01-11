#pragma once

namespace ib {
namespace mw {
namespace service {

class IServiceDiscovery
{
public: //types
  enum class Type {
    ServiceCreated,
    ServiceRemoved,
  };
  using ServiceDiscoveryHandlerT = std::function<void(Type discoveryType, const ServiceDescriptor&)>;

  //!< Publish a locally created new ServiceDescriptor to all other participants
  virtual void NotifyServiceCreated(const ServiceDescriptor& serviceDescriptor) = 0;
  //!< Publish a participant-local service removal to all other participants
  virtual void NotifyServiceRemoved(const ServiceDescriptor& serviceDescriptor) = 0;

  //!< Register a handler for asynchronous service creation notifications
  virtual void RegisterServiceDiscoveryHandler(ServiceDiscoveryHandlerT handler) = 0;


};

} // namespace sync
} // namespace mw
} // namespace ib
