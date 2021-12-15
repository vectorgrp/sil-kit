// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>

#include "IIbEndpoint.hpp" //for ServiceId
namespace ib {
namespace mw {
namespace service {

struct ServiceDescription
{
    ServiceId serviceId; //endpointId must be unique per participant
    //std::string serviceName;
    //std::string linkName;
    std::map<std::string, std::string> supplementalData; //key value pairs, e.g. "dataExchangeformat=foobar"
    /* Serdes map
    MAP_SIZE == N
    STR_SIZE key1 
    STR_SIZE value1
    ...
    STR_SIZE keyN
    STR_SIZE valueN

    /*
    Participant Receive ServiceDescription:
    if serviceId.type == PubSub:
        auto dataExchangeformat= supplementalData["dataExchangeFormat"] 
        if dataExchangeFormat matches:
            internalSubscriber(serviceId.linkName + dataExchangeFormat);
    if serviceId.type == NetworkSImulator:
        _netSimId = serviceId
        
    */
};

struct ServiceAnnouncement //requires history >= 1
{
    std::string participantName; 
    std::vector<ServiceDescription> services; //!< list of services provided by the participant
};

// ComAdapter->CreateController("neuer controller") ==>
//   => ServiceDiscoveryController: public IIbReceiver<ParticipantServiceAnnouncement>, public IIbSender<ParticipantServiceAnnouncement>
//          std::vector<ParticipantServiceAnnouncement> knownServices;
//          oder: std::set<ServiceDescription> knownServices;
//          std::map<IVAsioPeer*, serviceIndex> _serviceLookup; //fuer receive
//    => _serviceDescription.push_back(ServiceDescription{"neuer controller"...})
//    => IbLink<ServiceDescription>->SendMessage(_serviceDescription)


// Inline string utils
inline std::ostream& operator<<(std::ostream& out, const ServiceAnnouncement& serviceAnnouncement)
{
    out << "ServiceAnnouncement{" << serviceAnnouncement.participantName
        << "}"
        ;
    return out;
}
inline std::string to_string(const ServiceAnnouncement& serviceAnnouncement)
{
    std::stringstream str;
    str << serviceAnnouncement;
    return str.str();
}

} // namespace service
} // namespace mw
} // namespace ib
