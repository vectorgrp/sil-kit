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

#pragma once

#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"

SILKIT_BEGIN_DECLS

/*! \brief Create a lifecycle service at this SIL Kit simulation participant.
 *
 * The object returned must not be deallocated using free()!
 *
 * @param outRegistry Pointer that refers to the resulting registry (out parameter).
 * @param participantConfiguration Configuration of the participant (see \ref SilKit_ParticipantConfiguration_FromString).
 * @return
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_Create(
    SilKit_Vendor_Vector_SilKitRegistry** outRegistry, SilKit_ParticipantConfiguration* participantConfiguration);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Vendor_Vector_SilKitRegistry_Create_t)(
    SilKit_Vendor_Vector_SilKitRegistry** outRegistry, SilKit_ParticipantConfiguration* participantConfiguration);

/*! \brief Destroy a registry.
 *
 * @param registry The registry to be destroyed.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_Destroy(
    SilKit_Vendor_Vector_SilKitRegistry* registry);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Vendor_Vector_SilKitRegistry_Destroy_t)(
    SilKit_Vendor_Vector_SilKitRegistry* registry);

/*! \brief The handler to be called when all participants have disconnected.
 *
 * \param context The user provided context passed in \ref SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler.
 * \param registry The registry on which all participants have disconnected.
 */
typedef void (SilKitFPTR *SilKit_Vendor_Vector_SilKitRegistry_AllDisconnectedHandler_t)(
    void* context, SilKit_Vendor_Vector_SilKitRegistry* registry);

/*! \brief Register the handler that is called when all participants are disconnected
 *
 * @param registry The registry for which the handler should be set.
 * @param context A user provided context accessible in the handler.
 * @param handler The handler to be called when all participants have disconnected.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler(
    SilKit_Vendor_Vector_SilKitRegistry* registry, void* context,
    SilKit_Vendor_Vector_SilKitRegistry_AllDisconnectedHandler_t handler);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Vendor_Vector_SilKitRegistry_SetAllDisconnectedHandler_t)(
    SilKit_Vendor_Vector_SilKitRegistry* registry, void* context,
    SilKit_Vendor_Vector_SilKitRegistry_AllDisconnectedHandler_t handler);

/*! \brief Returns the logger that is used by the SIL Kit registry.
 *
 * The lifetime of the returned logger is directly bound to the lifetime of the registry. There is no further cleanup
 * necessary, except for destroying the registry.
 *
 * @param outLogger Pointer to the resulting logger instance (out parameter).
 * @param registry The registry whose logger should be returned.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_GetLogger(
    SilKit_Logger** outLogger, SilKit_Vendor_Vector_SilKitRegistry* registry);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Vendor_Vector_SilKitRegistry_GetLogger_t)(
    SilKit_Logger** outLogger, SilKit_Vendor_Vector_SilKitRegistry* registry);

/*! \brief Start to listen on the URI with scheme silkit://, e.g. silkit://localhost:8500, and return the URI with the
 *         port number used for listening for TCP connections.
 *
 * The port number will only be replaced, if the port number specified in the listen URI was 0, which indicates that
 * the port should be chosen by the operating system automatically.
 *
 * The host-part (IP address) of the listen URI is not modified.
 *
 * @param registry The registry which is instructed to start listening for incoming connections.
 * @param listenUri The listen URI on which the registry should listen.
 * @param outRegistryUri The modified listen URI which contains the actual port used for listening.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Vendor_Vector_SilKitRegistry_StartListening(
    SilKit_Vendor_Vector_SilKitRegistry* registry, const char* listenUri, const char** outRegistryUri);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Vendor_Vector_SilKitRegistry_StartListening_t)(
    SilKit_Vendor_Vector_SilKitRegistry* registry, const char* listenUri, const char** outRegistryUri);

SILKIT_END_DECLS
