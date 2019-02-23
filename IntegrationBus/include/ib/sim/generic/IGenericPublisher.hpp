// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/cfg/Config.hpp"

#include "GenericMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace generic {

class IGenericPublisher
{
public:
    virtual ~IGenericPublisher() = default;

    //! \brief Get the config struct used to setup this IGenericPublisher
    virtual auto Config() const -> const cfg::GenericPort& = 0;

    /*! \brief Publish a new value
     *
     * This is the preferred publishing method as C-style methods with
     * pointer and size are generally disregarded in modern C++. The
     * vector is passed to the underlying middleware using std::move.
     *
     * \param data An opaque block of raw data
     */
    virtual void Publish(std::vector<uint8_t> data) = 0;
    
    /*! \brief Publish a new value
     *
     * Convenience method to publish data. Creates a new std::vector
     * with content copied from \param data. For highest efficiency,
     * use \ref Publish(std::vector<uint8_t>) in combination with
     * std::move.
     *
     * \param data C-style pointer to an opaque block of data
     * \param size Size of the data block to be copied
     */
    virtual void Publish(const uint8_t* data, std::size_t size) = 0;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

} // namespace generic
} // namespace sim
} // namespace ib
