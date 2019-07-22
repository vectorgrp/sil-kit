// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IComAdapter.hpp"

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
};

} // mw
} // namespace ib

