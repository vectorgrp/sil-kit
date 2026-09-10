// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


namespace SilKit {
namespace Core {
struct IRegistryEventListener;
} // namespace Core
} // namespace SilKit


namespace VSilKit {


/*! A live connection from the registry to the SIL Kit Dashboard.
 *
 *  Deliberately minimal: the registry only needs to hand the listener to VAsioRegistry and then keep
 *  the instance alive. Everything else - connecting, batching, shutting down - is driven by the
 *  registry events themselves and by the destructor.
 *
 *  This is also the seam that SILKIT_BUILD_DASHBOARD switches: in a build without dashboard support
 *  DashboardInstance does not exist, and this abstract base is the only type that can name the
 *  registry's member.
 */
struct IDashboardInstance
{
    virtual ~IDashboardInstance() = default;

    /*! The listener to pass to the registry.
     *
     *  Remains owned by this instance, which must outlive the registry.
     */
    virtual auto GetRegistryEventListener() -> SilKit::Core::IRegistryEventListener* = 0;
};


} // namespace VSilKit
