// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"
#include "silkit/capi/SilKitMacros.h"

#include "silkit/capi/EventProducer.h"

#include "silkit/capi/Can.h"
#include "silkit/capi/Flexray.h"
#include "silkit/capi/Ethernet.h"
#include "silkit/capi/Lin.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

typedef uint32_t SilKit_Experimental_SimulatedNetworkType;
#define SilKit_NetworkType_Undefined ((SilKit_Experimental_SimulatedNetworkType)0)
#define SilKit_NetworkType_CAN ((SilKit_Experimental_SimulatedNetworkType)1)
#define SilKit_NetworkType_LIN ((SilKit_Experimental_SimulatedNetworkType)2)
#define SilKit_NetworkType_Ethernet ((SilKit_Experimental_SimulatedNetworkType)3)
#define SilKit_NetworkType_FlexRay ((SilKit_Experimental_SimulatedNetworkType)4)

typedef struct SilKit_Experimental_NetworkSimulator SilKit_Experimental_NetworkSimulator;
typedef struct SilKit_Experimental_SimulatedNetwork SilKit_Experimental_SimulatedNetwork;
typedef struct SilKit_Experimental_SimulatedNetworkFunctions SilKit_Experimental_SimulatedNetworkFunctions;

typedef struct SilKit_Experimental_SimulatedCanController SilKit_Experimental_SimulatedCanController;
typedef struct SilKit_Experimental_SimulatedFlexRayController SilKit_Experimental_SimulatedFlexRayController;

typedef struct SilKit_Experimental_SimulatedControllerFunctions SilKit_Experimental_SimulatedControllerFunctions;
typedef struct SilKit_Experimental_SimulatedCanControllerFunctions SilKit_Experimental_SimulatedCanControllerFunctions;
typedef struct SilKit_Experimental_SimulatedFlexRayControllerFunctions SilKit_Experimental_SimulatedFlexRayControllerFunctions;

struct SilKit_Experimental_SimulatedNetworkFunctions
{
    SilKit_StructHeader structHeader;
    void (*ProvideSimulatedController)(void** outSimulatedController, const void** outSimulateControllerFunctions,
                                       SilKit_Experimental_ControllerDescriptor controllerDescriptor, void* simulatedNetwork);
    void (*SimulatedControllerRemoved)(SilKit_Experimental_ControllerDescriptor controllerDescriptor, void* simulatedNetwork);
    void (*SetEventProducer)(void* eventProducer, void* simulatedNetwork);
};

// --------------------------------
// Can
// --------------------------------

struct SilKit_Experimental_NetSim_CanFrameRequest
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_CanFrame* frame; //!< The CAN Frame that corresponds to the meta data
    void* userContext; //!< Optional pointer provided by user when sending the frame
};
typedef struct SilKit_Experimental_NetSim_CanFrameRequest SilKit_Experimental_NetSim_CanFrameRequest;

struct SilKit_Experimental_NetSim_CanConfigureBaudrate
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    uint32_t rate;
    uint32_t fdRate;
    uint32_t xlRate;
};
typedef struct SilKit_Experimental_NetSim_CanConfigureBaudrate SilKit_Experimental_NetSim_CanConfigureBaudrate;

typedef int32_t SilKit_Experimental_NetSim_CanControllerModeFlags;

#define SilKit_Experimental_NetSim_CanControllerModeFlags_ResetErrorHandling \
    ((SilKit_Experimental_NetSim_CanControllerModeFlags)BIT(0)) //!< Reset the error counters to zero and the error state to error active.

#define SilKit_Experimental_NetSim_CanControllerModeFlags_CancelTransmitRequests \
    ((SilKit_Experimental_NetSim_CanControllerModeFlags)BIT(1)) //!< Cancel all outstanding transmit requests (flush transmit queue of controller).

struct SilKit_Experimental_NetSim_CanControllerMode
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_Experimental_NetSim_CanControllerModeFlags canControllerModeFlags;
    SilKit_CanControllerState state; 
};
typedef struct SilKit_Experimental_NetSim_CanControllerMode SilKit_Experimental_NetSim_CanControllerMode;

struct SilKit_Experimental_SimulatedCanControllerFunctions
{
    SilKit_StructHeader structHeader;
    void (*OnSetBaudrate)(void* controller, const SilKit_Experimental_NetSim_CanConfigureBaudrate* configureBautrate);
    void (*OnFrameRequest)(void* controller, const SilKit_Experimental_NetSim_CanFrameRequest* frameRequest);
    void (*OnSetControllerMode)(void* controller, const SilKit_Experimental_NetSim_CanControllerMode* controllerMode);
};

// --------------------------------
// FlexRay
// --------------------------------

struct SilKit_Experimental_NetSim_FlexrayHostCommand
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_FlexrayChiCommand chiCommand;
};
typedef struct SilKit_Experimental_NetSim_FlexrayHostCommand SilKit_Experimental_NetSim_FlexrayHostCommand;

struct SilKit_Experimental_NetSim_FlexrayControllerConfig
{
    //! The interface id specifying which version of this struct was obtained
    SilKit_StructHeader structHeader;
    //! FlexRay cluster parameters
    SilKit_FlexrayClusterParameters* clusterParams;
    //! FlexRay node parameters
    SilKit_FlexrayNodeParameters* nodeParams;
    //! FlexRay buffer configs
    uint32_t numBufferConfigs;
    SilKit_FlexrayTxBufferConfig* bufferConfigs;
};
typedef struct SilKit_Experimental_NetSim_FlexrayControllerConfig SilKit_Experimental_NetSim_FlexrayControllerConfig;

struct SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    uint16_t txBufferIdx;
    SilKit_FlexrayTxBufferConfig* txBufferConfig;
};
typedef struct SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate;

struct SilKit_Experimental_NetSim_FlexrayTxBufferUpdate
{
    //! The interface id specifying which version of this struct was obtained
    SilKit_StructHeader structHeader;
    //! Index of the TX Buffers according to the configured buffers (cf. FlexrayControllerConfig).
    uint16_t txBufferIndex;
    //! Payload data valid flag
    SilKit_Bool payloadDataValid;
    //! Raw payload containing 0 to 254 bytes.
    SilKit_ByteVector payload;
};
typedef struct SilKit_Experimental_NetSim_FlexrayTxBufferUpdate SilKit_Experimental_NetSim_FlexrayTxBufferUpdate;


struct SilKit_Experimental_SimulatedFlexRayControllerFunctions
{
    SilKit_StructHeader structHeader;
    void (*OnHostCommand)(void* controller, const SilKit_Experimental_NetSim_FlexrayHostCommand* hostCommand);
    void (*OnControllerConfig)(void* controller, const SilKit_Experimental_NetSim_FlexrayControllerConfig* controllerConfig);
    void (*OnTxBufferConfigUpdate)(void* controller, const SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate* txBufferConfigUpdate);
    void (*OnTxBufferUpdate)(void* controller, const SilKit_Experimental_NetSim_FlexrayTxBufferUpdate* txBufferUpdate);
};

// --------------------------------
// Ethernet
// --------------------------------

struct SilKit_Experimental_NetSim_EthernetFrameRequest
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_EthernetFrame* ethernetFrame;
    void* userContext; //!< Optional pointer provided by user when sending the frame
};
typedef struct SilKit_Experimental_NetSim_EthernetFrameRequest SilKit_Experimental_NetSim_EthernetFrameRequest;

typedef uint8_t SilKit_EthernetControllerMode;
#define SilKit_EthernetControllerMode_Inactive ((SilKit_EthernetControllerMode)0)
#define SilKit_EthernetControllerMode_Active ((SilKit_EthernetControllerMode)1)

struct SilKit_Experimental_NetSim_EthernetControllerMode
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_EthernetControllerMode mode;
};
typedef struct SilKit_Experimental_NetSim_EthernetControllerMode SilKit_Experimental_NetSim_EthernetControllerMode;

struct SilKit_Experimental_SimulatedEthernetControllerFunctions
{
    SilKit_StructHeader structHeader;
    void (*OnFrameRequest)(void* controller, const SilKit_Experimental_NetSim_EthernetFrameRequest* frameRequest);
    void (*OnSetControllerMode)(void* controller, const SilKit_Experimental_NetSim_EthernetControllerMode* controllerMode);
};

// --------------------------------
// Lin
// --------------------------------

typedef uint8_t SilKit_Experimental_NetSim_LinSimulationMode;
#define SilKit_Experimental_NetSim_LinSimulationMode_Default ((SilKit_Experimental_NetSim_LinSimulationMode)0)
#define SilKit_Experimental_NetSim_LinSimulationMode_Dynamic ((SilKit_Experimental_NetSim_LinSimulationMode)1)

struct SilKit_Experimental_NetSim_LinFrameRequest
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_LinFrame* frame;
    SilKit_LinFrameResponseType responseType;
};
typedef struct SilKit_Experimental_NetSim_LinFrameRequest SilKit_Experimental_NetSim_LinFrameRequest;

struct SilKit_Experimental_NetSim_LinFrameHeaderRequest
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_LinId id;
};
typedef struct SilKit_Experimental_NetSim_LinFrameHeaderRequest SilKit_Experimental_NetSim_LinFrameHeaderRequest;

struct SilKit_Experimental_NetSim_LinWakeupPulse
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Timestamp of the wakeup pulse 
};
typedef struct SilKit_Experimental_NetSim_LinWakeupPulse SilKit_Experimental_NetSim_LinWakeupPulse;

struct SilKit_Experimental_NetSim_LinControllerConfig
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_LinControllerMode controllerMode;
    SilKit_LinBaudRate baudRate;
    size_t numFrameResponses;
    SilKit_LinFrameResponse* frameResponses;
    SilKit_Experimental_NetSim_LinSimulationMode simulationMode;
};
typedef struct SilKit_Experimental_NetSim_LinControllerConfig SilKit_Experimental_NetSim_LinControllerConfig;

struct SilKit_Experimental_NetSim_LinFrameResponseUpdate
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    size_t numFrameResponses;
    SilKit_LinFrameResponse* frameResponses;
};
typedef struct SilKit_Experimental_NetSim_LinFrameResponseUpdate SilKit_Experimental_NetSim_LinFrameResponseUpdate;

struct SilKit_Experimental_NetSim_LinControllerStatusUpdate
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_LinControllerStatus status;
    SilKit_NanosecondsTime timestamp; //!< Timestamp of the wakeup pulse
};
typedef struct SilKit_Experimental_NetSim_LinControllerStatusUpdate SilKit_Experimental_NetSim_LinControllerStatusUpdate;

struct SilKit_Experimental_SimulatedLinControllerFunctions
{
    SilKit_StructHeader structHeader;
    void (*OnFrameRequest)(void* controller, const SilKit_Experimental_NetSim_LinFrameRequest* frameRequest);
    void (*OnFrameHeaderRequest)(void* controller, const SilKit_Experimental_NetSim_LinFrameHeaderRequest* frameHeaderRequest);
    void (*OnWakeupPulse)(void* controller, const SilKit_Experimental_NetSim_LinWakeupPulse* wakeupPulse);
    void (*OnControllerConfig)(void* controller, const SilKit_Experimental_NetSim_LinControllerConfig* controllerConfig);
    void (*OnFrameResponseUpdate)(void* controller, const SilKit_Experimental_NetSim_LinFrameResponseUpdate* frameResponseUpdate);
    void (*OnControllerStatusUpdate)(void* controller, const SilKit_Experimental_NetSim_LinControllerStatusUpdate* statusUpdate);
};

// --------------------------------
// NetworkSimulator
// --------------------------------

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_NetworkSimulator_Create(SilKit_Experimental_NetworkSimulator** outNetworkSimulator,
                                                                      SilKit_Participant* participant);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_NetworkSimulator_Create_t)(SilKit_Experimental_NetworkSimulator** outNetworkSimulator,
                                                                        SilKit_Participant* participant);

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_NetworkSimulator_SimulateNetwork(
    SilKit_Experimental_NetworkSimulator* networkSimulator, const char* networkName, SilKit_Experimental_SimulatedNetworkType networkType,
    void* simulatedNetwork, const SilKit_Experimental_SimulatedNetworkFunctions* simulatedNetworkFunctions);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_NetworkSimulator_SimulateNetwork_t)(
    SilKit_Experimental_NetworkSimulator* networkSimulator, const char* networkName, SilKit_Experimental_SimulatedNetworkType networkType,
    void* simulatedNetwork, const SilKit_Experimental_SimulatedNetworkFunctions* simulatedNetworkFunctions);

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_NetworkSimulator_Start(SilKit_Experimental_NetworkSimulator* networkSimulator);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_Experimental_NetworkSimulator_Start_t)(SilKit_Experimental_NetworkSimulator* networkSimulator);

SILKIT_END_DECLS

#pragma pack(pop)
