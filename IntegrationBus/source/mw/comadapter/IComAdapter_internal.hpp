// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IComAdapter.hpp"

// IbInternal component:
#include "IIbToCanSimulator.hpp"
#include "IIbToCanController.hpp"
#include "IIbToCanControllerProxy.hpp"

#include "IIbToEthSimulator.hpp"
#include "IIbToEthController.hpp"
#include "IIbToEthControllerProxy.hpp"

#include "IIbToLinSimulator.hpp"
#include "IIbToLinController.hpp"
#include "IIbToLinControllerProxy.hpp"

#include "IIbToFrBusSimulator.hpp"
#include "IIbToFrController.hpp"
#include "IIbToFrControllerProxy.hpp"

#include "IIbToInPort.hpp"
#include "IIbToOutPort.hpp"

#include "IIbToGenericSubscriber.hpp"
#include "IIbToGenericPublisher.hpp"

#include "IIbToSystemMonitor.hpp"
#include "IIbToSystemController.hpp"
#include "IIbToParticipantController.hpp"
#include "IIbToSyncMaster.hpp"

namespace ib {
namespace mw {

class IComAdapterInternal : public IComAdapter
{
public:
    // ----------------------------------------
    // Public methods

    /*! \brief Join the middleware domain as a participant.
    *
    * Join the middleware domain and become a participant.
    * \param domainId ID of the domain
    *
    * \throw std::exception A participant was created previously, or a
    * participant could not be created.
    */
    virtual void joinIbDomain(uint32_t domainId) = 0;

    virtual void RegisterCanSimulator(sim::can::IIbToCanSimulator* busSim) = 0 ;
    virtual void RegisterEthSimulator(sim::eth::IIbToEthSimulator* busSim) = 0 ;
    virtual void RegisterFlexraySimulator(sim::fr::IIbToFrBusSimulator* busSim) = 0 ;
    virtual void RegisterLinSimulator(sim::lin::IIbToLinSimulator* busSim) = 0;
};

} // mw
} // namespace ib

