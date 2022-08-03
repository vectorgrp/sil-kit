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

#define _CRT_SECURE_NO_WARNINGS
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "silkit/capi/SilKit.h"

#ifdef WIN32
#    include "windows.h"
#    define SleepMs(X) Sleep(X)
#else
#    include <unistd.h>
#    define SleepMs(X) usleep((X)*1000)
#endif
#define UNUSED_ARG(X) (void)(X)

void AbortOnFailedAllocation(const char* failedAllocStrucName)
{
    fprintf(stderr, "Error: Allocation of \"%s\" failed, aborting...", failedAllocStrucName);
    abort();
}

typedef uint8_t MasterState;
#define MasterState_Ignore ((MasterState)0)
#define MasterState_PerformWakeup ((MasterState)1)
#define MasterState_WaitForWakeup ((MasterState)2)
#define MasterState_WakeupDone ((MasterState)3)

static inline SilKit_NanosecondsTime milliseconds(unsigned long long msValue)
{
    return msValue * 1000000ULL;
}

struct FlexrayNode
{
    SilKit_FlexrayController* _controller;
    SilKit_FlexrayControllerConfig* _controllerConfig;
    SilKit_FlexrayPocStatusEvent _oldPocStatus;
    SilKit_Bool _configureCalled;
    SilKit_NanosecondsTime _startupDelay;
    MasterState _busState;
};
typedef struct FlexrayNode FlexrayNode;

FlexrayNode* FlexrayNode_Create(SilKit_FlexrayController* controller, SilKit_FlexrayControllerConfig* config);
void FlexrayNode_Init(FlexrayNode* flexrayNode);
void FlexrayNode_SetStartupDelay(FlexrayNode* flexrayNode, SilKit_NanosecondsTime delay);
void FlexrayNode_DoAction(FlexrayNode* flexrayNode, SilKit_NanosecondsTime now);
void FlexrayNode_PocReady(struct FlexrayNode* flexrayNode, SilKit_NanosecondsTime now);
void FlexrayNode_TxBufferUpdate(FlexrayNode* flexrayNode, SilKit_NanosecondsTime now);
void FlexrayNode_ReconfigureTxBuffers(struct FlexrayNode* flexrayNode);
void FlexrayNode_PocStatusHandler(void* context, SilKit_FlexrayController* controller,
                                  const SilKit_FlexrayPocStatusEvent* pocStatus);
void FlexrayNode_WakeupHandler(void* context, SilKit_FlexrayController* controller, const SilKit_FlexraySymbolEvent* symbol);

void print_header(FILE* out, SilKit_FlexrayHeader* header)
{
    fprintf(out, "FlexrayHeader{f=[%s%s%s%s],s=%d,l=%d,crc=0x%04x,c=%d}",
            ((header->flags & SilKit_FlexrayHeader_SuFIndicator) != 0) ? "U" : "-",
            ((header->flags & SilKit_FlexrayHeader_SyFIndicator) != 0) ? "Y" : "-",
            ((header->flags & SilKit_FlexrayHeader_NFIndicator) != 0) ? "-" : "N",
            ((header->flags & SilKit_FlexrayHeader_PPIndicator) != 0) ? "P" : "-", header->frameId, header->payloadLength,
            header->headerCrc, header->cycleCount);
}

void print_hexbytes(FILE* out, const uint8_t* bytes, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++)
    {
        if (i == 0)
            fprintf(out, "%02x", bytes[i]);
        else
            fprintf(out, " %02x", bytes[i]);
    }
    fprintf(out, " \"");
    for (i = 0; i < size; i++)
    {
        if (isprint(bytes[i]))
            putc(bytes[i], out);
        else
            putc('.', out);
    }
    fprintf(out, "\"");
}

void print_channel(FILE* out, SilKit_FlexrayChannel channel)
{
    switch (channel)
    {
    case SilKit_FlexrayChannel_A: fprintf(out, "A"); break;
    case SilKit_FlexrayChannel_B: fprintf(out, "B"); break;
    case SilKit_FlexrayChannel_AB: fprintf(out, "AB"); break;
    default: fprintf(out, "?"); break;
    }
}
void print_flexrayframeevent(FILE* out, const SilKit_FlexrayFrameEvent* message)
{
    fprintf(out, "FlexrayFrameEvent ch=");
    print_channel(out, message->channel);
    fprintf(out, ",");
    print_header(out, message->frame->header);
    if ((message->frame->header->flags & SilKit_FlexrayHeader_NFIndicator) != 0)
    {
        fprintf(out, ", payload=");
        print_hexbytes(out, message->frame->payload.data, message->frame->payload.size);
    }
    fprintf(out, " @%" PRIu64 "\n", message->timestamp);
    fflush(out);
}

void print_flexrayframetransmitevent(FILE* out, const SilKit_FlexrayFrameTransmitEvent* message)
{
    fprintf(out, "FlexrayFrameTransmitEvent ch=");
    print_channel(out, message->channel);
    fprintf(out, ",");
    print_header(out, message->frame->header);
    fprintf(out, ", txBuffer=%d  @%" PRIu64 "\n", message->txBufferIndex, message->timestamp);
    fflush(out);
}

void ReceiveFrame(void* context, SilKit_FlexrayController* controller, const SilKit_FlexrayFrameEvent* message)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    fprintf(stdout, ">> ");
    print_flexrayframeevent(stdout, message);
}

void ReceiveFrameTransmit(void* context, SilKit_FlexrayController* controller,
                          const SilKit_FlexrayFrameTransmitEvent* message)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    fprintf(stdout, ">> ");
    print_flexrayframetransmitevent(stdout, message);
}

void ReceiveWakeup(void* context, SilKit_FlexrayController* controller, const SilKit_FlexrayWakeupEvent* message)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    fprintf(stdout, ">> ");
    fprintf(stdout, "FlexrayWakeupEvent channel=%d symbol=%d\n", message->channel, message->pattern);
}

void ReceiveSymbol(void* context, SilKit_FlexrayController* controller, const SilKit_FlexraySymbolEvent* message)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    fprintf(stdout, ">> ");
    fprintf(stdout, "FlexraySymbolEvent channel=%d symbol=%d\n", message->channel, message->pattern);
}

void ReceiveSymbolTransmit(void* context, SilKit_FlexrayController* controller,
                           const SilKit_FlexraySymbolTransmitEvent* message)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    fprintf(stdout, ">> ");
    fprintf(stdout, "FlexraySymbolTransmitEvent channel=%d symbol=%d\n", message->channel, message->pattern);
}

void ReceiveCycleStart(void* context, SilKit_FlexrayController* controller, const SilKit_FlexrayCycleStartEvent* message)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    fprintf(stdout, ">> ");
    fprintf(stdout, "FlexrayCycleStartEvent cycleCounter=%d\n", message->cycleCounter);
}

void AppendTxBufferConfig(SilKit_FlexrayControllerConfig** inOutControllerConfig,
                          const SilKit_FlexrayTxBufferConfig* txBufferConfig)
{
    uint32_t newNumTxBufferConfigs = (*inOutControllerConfig)->numBufferConfigs + 1;
    size_t newSize = newNumTxBufferConfigs * sizeof(SilKit_FlexrayTxBufferConfig);
    SilKit_FlexrayTxBufferConfig* result =
        (SilKit_FlexrayTxBufferConfig*)realloc((*inOutControllerConfig)->bufferConfigs, newSize);
    if (result == NULL)
    {
        AbortOnFailedAllocation("SilKit_FlexrayTxBufferConfig");
        return;
    }
    memcpy(&result[newNumTxBufferConfigs - 1], txBufferConfig, sizeof(SilKit_FlexrayTxBufferConfig));
    (*inOutControllerConfig)->numBufferConfigs = newNumTxBufferConfigs;
    (*inOutControllerConfig)->bufferConfigs = result;
}

FlexrayNode* FlexrayNode_Create(SilKit_FlexrayController* controller, SilKit_FlexrayControllerConfig* config)
{
    struct FlexrayNode* flexrayNode = malloc(sizeof(FlexrayNode));
    if (flexrayNode == NULL)
    {
        AbortOnFailedAllocation("FlexrayNode");
        return NULL;
    }
    flexrayNode->_controller = controller;
    flexrayNode->_controllerConfig = config;
    flexrayNode->_oldPocStatus.state = SilKit_FlexrayPocState_DefaultConfig;
    flexrayNode->_configureCalled = SilKit_False;
    flexrayNode->_startupDelay = 0;
    flexrayNode->_busState = MasterState_Ignore;
    return flexrayNode;
}

void FlexrayNode_SetStartupDelay(FlexrayNode* flexrayNode, SilKit_NanosecondsTime delay)
{
    flexrayNode->_startupDelay = delay;
}

void FlexrayNode_Init(FlexrayNode* flexrayNode)
{
    SilKit_ReturnCode returnCode;
    if (flexrayNode->_configureCalled)
        return;

    returnCode = SilKit_FlexrayController_Configure(flexrayNode->_controller, flexrayNode->_controllerConfig);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_Configure => %s\n", SilKit_GetLastErrorString());
    }
    flexrayNode->_configureCalled = SilKit_True;
}

void FlexrayNode_DoAction(FlexrayNode* flexrayNode, SilKit_NanosecondsTime now)
{
    if (now < flexrayNode->_startupDelay)
        return;
    switch (flexrayNode->_oldPocStatus.state)
    {
    case SilKit_FlexrayPocState_DefaultConfig: FlexrayNode_Init(flexrayNode); break;
    case SilKit_FlexrayPocState_Ready: FlexrayNode_PocReady(flexrayNode, now); break;
    case SilKit_FlexrayPocState_NormalActive:
        if (now == milliseconds(100) + milliseconds(flexrayNode->_startupDelay))
        {
            FlexrayNode_ReconfigureTxBuffers(flexrayNode);
        }
        else
        {
            FlexrayNode_TxBufferUpdate(flexrayNode, now);
        }
        break;
    case SilKit_FlexrayPocState_Config:
    case SilKit_FlexrayPocState_Startup:
    case SilKit_FlexrayPocState_Wakeup:
    case SilKit_FlexrayPocState_NormalPassive:
    case SilKit_FlexrayPocState_Halt: return;
    }
}

void FlexrayNode_PocReady(FlexrayNode* flexrayNode, SilKit_NanosecondsTime now)
{
    UNUSED_ARG(now);

    switch (flexrayNode->_busState)
    {
    case MasterState_PerformWakeup:
        SilKit_FlexrayController_ExecuteCmd(flexrayNode->_controller, SilKit_FlexrayChiCommand_WAKEUP);
        break;
    case MasterState_WaitForWakeup: break;
    case MasterState_WakeupDone:
        SilKit_FlexrayController_ExecuteCmd(flexrayNode->_controller, SilKit_FlexrayChiCommand_ALLOW_COLDSTART);
        SilKit_FlexrayController_ExecuteCmd(flexrayNode->_controller, SilKit_FlexrayChiCommand_RUN);
        break;
    default: break;
    }
}

void FlexrayNode_TxBufferUpdate(FlexrayNode* flexrayNode, SilKit_NanosecondsTime now)
{
    UNUSED_ARG(now);

    if (flexrayNode->_controllerConfig->numBufferConfigs == 0)
        return;

    static uint16_t msgNumber = 0;

    uint16_t bufferIdx = msgNumber % (uint16_t)(flexrayNode->_controllerConfig->numBufferConfigs);

    // prepare a friendly message as payload
    char payloadString[128];
    sprintf(payloadString, "FlexrayFrameEvent#%d sent from buffer %d", msgNumber, bufferIdx);

    SilKit_FlexrayTxBufferUpdate update;
    SilKit_Struct_Init(SilKit_FlexrayTxBufferUpdate, update);
    update.payload.size = strlen(payloadString + 1);
    update.payload.data = (uint8_t*)payloadString;
    update.payloadDataValid = SilKit_True;
    update.txBufferIndex = bufferIdx;

    SilKit_ReturnCode returnCode = SilKit_FlexrayController_UpdateTxBuffer(flexrayNode->_controller, &update);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_UpdateTxBuffer => %s\n", SilKit_GetLastErrorString());
    }

    msgNumber++;
}

// Reconfigure buffers: Swap Channels A and B
void FlexrayNode_ReconfigureTxBuffers(struct FlexrayNode* flexrayNode)
{
    printf("Reconfiguring TxBuffers. Swapping FlexrayChannel::A and FlexrayChannel::B\n");
    uint16_t idx;
    for (idx = 0; idx < flexrayNode->_controllerConfig->numBufferConfigs; idx++)
    {
        SilKit_FlexrayTxBufferConfig* bufferConfig = &(flexrayNode->_controllerConfig->bufferConfigs[idx]);
        switch (bufferConfig->channels)
        {
        case SilKit_FlexrayChannel_A:
            bufferConfig->channels = SilKit_FlexrayChannel_B;
            SilKit_FlexrayController_ReconfigureTxBuffer(flexrayNode->_controller, idx, bufferConfig);
            break;
        case SilKit_FlexrayChannel_B:
            bufferConfig->channels = SilKit_FlexrayChannel_A;
            SilKit_FlexrayController_ReconfigureTxBuffer(flexrayNode->_controller, idx, bufferConfig);
            break;
        default: break;
        }
    }
}

void FlexrayNode_PocStatusHandler(void* context, SilKit_FlexrayController* controller,
                                  const SilKit_FlexrayPocStatusEvent* pocStatus)
{
    UNUSED_ARG(controller);

    FlexrayNode* flexrayNode = (FlexrayNode*)context;
    printf(">> POC=%d freeze=%d wakeupStatus=%d slotMode=%d T=%" PRIu64 "\n", pocStatus->state, pocStatus->freeze,
           pocStatus->wakeupStatus, pocStatus->slotMode, pocStatus->timestamp);

    if (flexrayNode->_oldPocStatus.state == SilKit_FlexrayPocState_Wakeup && pocStatus->state == SilKit_FlexrayPocState_Ready)
    {
        printf("   Wakeup finished...\n");
        flexrayNode->_busState = MasterState_WakeupDone;
    }

    flexrayNode->_oldPocStatus = *pocStatus;
}

void FlexrayNode_WakeupHandler(void* context, SilKit_FlexrayController* controller, const SilKit_FlexraySymbolEvent* symbol)
{
    UNUSED_ARG(context);

    printf(">> WAKEUP! (%d)\n", symbol->pattern);
    SilKit_FlexrayController_ExecuteCmd(controller, SilKit_FlexrayChiCommand_ALLOW_COLDSTART);
    SilKit_FlexrayController_ExecuteCmd(controller, SilKit_FlexrayChiCommand_RUN);
}

void FlexrayNode_SimulationStep(void* context, SilKit_TimeSyncService* timeSyncService, SilKit_NanosecondsTime time,
                                SilKit_NanosecondsTime duration)
{
    UNUSED_ARG(timeSyncService);

    FlexrayNode* node = (FlexrayNode*)context;
    uint64_t nowMs = time / 1000000ULL;
    uint64_t durationMs = duration / 1000000ULL;
    printf("now=%" PRIu64 "ms; duration=%" PRIu64 "ms\n", nowMs, durationMs);
    FlexrayNode_DoAction(node, time);
    SleepMs(500);
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

char* LoadFile(char const* path)
{
    size_t length = 0;
    char* result = NULL;
    FILE* f = fopen(path, "rb");

    if (f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* buffer = (char*)malloc((length + 1) * sizeof(char));
        if (buffer)
        {
            size_t num = fread(buffer, sizeof(uint8_t), length, f);
            if (num != length)
            {
                printf("Warning: short read on config file: %zu/%zu\n", num, length);
                exit(1);
            }
            buffer[length] = '\0';
            result = buffer;
        }
        fclose(f);
    }
    return result;
}

int main(int argc, char** argv)
{
    char* participantName;
    SilKit_Participant* participant;

    SilKit_FlexrayClusterParameters clusterParams;
    SilKit_Struct_Init(SilKit_FlexrayClusterParameters, clusterParams);
    clusterParams.gColdstartAttempts = 8;
    clusterParams.gCycleCountMax = 63;
    clusterParams.gdActionPointOffset = 2;
    clusterParams.gdDynamicSlotIdlePhase = 1;
    clusterParams.gdMiniSlot = 5;
    clusterParams.gdMiniSlotActionPointOffset = 2;
    clusterParams.gdStaticSlot = 31;
    clusterParams.gdSymbolWindow = 1;
    clusterParams.gdSymbolWindowActionPointOffset = 1;
    clusterParams.gdTSSTransmitter = 9;
    clusterParams.gdWakeupTxActive = 60;
    clusterParams.gdWakeupTxIdle = 180;
    clusterParams.gListenNoise = 2;
    clusterParams.gMacroPerCycle = 3636;
    clusterParams.gMaxWithoutClockCorrectionFatal = 2;
    clusterParams.gMaxWithoutClockCorrectionPassive = 2;
    clusterParams.gNumberOfMiniSlots = 291;
    clusterParams.gNumberOfStaticSlots = 70;
    clusterParams.gPayloadLengthStatic = 16;
    clusterParams.gSyncFrameIDCountMax = 15;

    SilKit_FlexrayNodeParameters nodeParams;
    SilKit_Struct_Init(SilKit_FlexrayClusterParameters, nodeParams);
    nodeParams.pAllowHaltDueToClock = 1;
    nodeParams.pAllowPassiveToActive = 0;
    nodeParams.pChannels = SilKit_FlexrayChannel_AB;
    nodeParams.pClusterDriftDamping = 2;
    nodeParams.pdAcceptedStartupRange = 212;
    nodeParams.pdListenTimeout = 400162;
    nodeParams.pKeySlotId = 10;
    nodeParams.pKeySlotOnlyEnabled = 0;
    nodeParams.pKeySlotUsedForStartup = 1;
    nodeParams.pKeySlotUsedForSync = 0;
    nodeParams.pLatestTx = 249;
    nodeParams.pMacroInitialOffsetA = 3;
    nodeParams.pMacroInitialOffsetB = 3;
    nodeParams.pMicroInitialOffsetA = 6;
    nodeParams.pMicroInitialOffsetB = 6;
    nodeParams.pMicroPerCycle = 200000;
    nodeParams.pOffsetCorrectionOut = 127;
    nodeParams.pOffsetCorrectionStart = 3632;
    nodeParams.pRateCorrectionOut = 81;
    nodeParams.pWakeupChannel = SilKit_FlexrayChannel_A;
    nodeParams.pWakeupPattern = 33;
    nodeParams.pdMicrotick = SilKit_FlexrayClockPeriod_T25NS;
    nodeParams.pSamplesPerMicrotick = 2;

    if (argc < 3)
    {
        printf("usage: SilKitDemoCFlexray <ConfigJsonFile> <ParticipantName> [RegistryUri]\n");
        return 1;
    }

    char* jsonString = LoadFile(argv[1]);
    if (jsonString == NULL)
    {
        printf("Error: cannot open config file %s\n", argv[1]);
        return 1;
    }
    participantName = argv[2];

    const char* registryUri = "silkit://localhost:8500";
    if (argc >= 4)
    {
        registryUri = argv[3];
    }

    SilKit_ReturnCode returnCode;

    SilKit_ParticipantConfiguration* participantConfiguration = NULL;
    returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfiguration, jsonString);
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    returnCode = SilKit_Participant_Create(&participant, participantConfiguration, participantName, registryUri);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_Participant_Create => %s\n", SilKit_GetLastErrorString());
        return 2;
    }

    returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfiguration);
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    SilKit_LifecycleService* lifecycleService;
    returnCode = SilKit_LifecycleService_Create(&lifecycleService, participant);

    SilKit_TimeSyncService* timeSyncService;
    returnCode = SilKit_TimeSyncService_Create(&timeSyncService, lifecycleService);

    const char* flexrayControllerName = "FlexRay1";
    const char* flexrayNetworkName = "FlexRay1";

    SilKit_FlexrayControllerConfig* config = (SilKit_FlexrayControllerConfig*)malloc(sizeof(SilKit_FlexrayControllerConfig));
    if (config == NULL)
    {
        AbortOnFailedAllocation("SilKit_FlexrayControllerConfig");
        return 2;
    }
    memset(config, 0, sizeof(SilKit_FlexrayControllerConfig));
    SilKit_Struct_Init(SilKit_FlexrayControllerConfig, *config);
    config->clusterParams = &clusterParams;
    config->nodeParams = &nodeParams;

    if (!strcmp(participantName, "Node0"))
    {
        // initialize bufferConfig to send some FrMessages
        SilKit_FlexrayTxBufferConfig cfg;
        SilKit_Struct_Init(SilKit_FlexrayTxBufferConfig, cfg);
        cfg.channels = SilKit_FlexrayChannel_AB;
        cfg.slotId = 10;
        cfg.offset = 0;
        cfg.repetition = 1;
        cfg.hasPayloadPreambleIndicator = SilKit_False;
        cfg.headerCrc = 5;
        cfg.transmissionMode = SilKit_FlexrayTransmissionMode_SingleShot;
        AppendTxBufferConfig(&config, &cfg);

        cfg.channels = SilKit_FlexrayChannel_A;
        cfg.slotId = 20;
        AppendTxBufferConfig(&config, &cfg);

        cfg.channels = SilKit_FlexrayChannel_B;
        cfg.slotId = 30;
        AppendTxBufferConfig(&config, &cfg);
    }
    else if (!strcmp(participantName, "Node1"))
    {
        config->nodeParams->pKeySlotId = 11;

        // initialize bufferConfig to send some FrMessages
        SilKit_FlexrayTxBufferConfig cfg;
        SilKit_Struct_Init(SilKit_FlexrayTxBufferConfig, cfg);
        cfg.channels = SilKit_FlexrayChannel_AB;
        cfg.slotId = 11;
        cfg.offset = 0;
        cfg.repetition = 1;
        cfg.hasPayloadPreambleIndicator = SilKit_False;
        cfg.headerCrc = 5;
        cfg.transmissionMode = SilKit_FlexrayTransmissionMode_SingleShot;
        AppendTxBufferConfig(&config, &cfg);

        cfg.channels = SilKit_FlexrayChannel_A;
        cfg.slotId = 21;
        AppendTxBufferConfig(&config, &cfg);

        cfg.channels = SilKit_FlexrayChannel_B;
        cfg.slotId = 31;
        AppendTxBufferConfig(&config, &cfg);
    }
    else
    {
        printf("Wrong participant name provided. Use either \"Node0\" or \"Node1\".\n");
        return 1;
    }

    SilKit_FlexrayController* controller;
    returnCode = SilKit_FlexrayController_Create(&controller, participant, flexrayControllerName, flexrayNetworkName);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_Create => %s\n", SilKit_GetLastErrorString());
        return 2;
    }
    FlexrayNode* frNode = FlexrayNode_Create(controller, config);
    if (!strcmp(participantName, "Node0"))
    {
        frNode->_busState = MasterState_PerformWakeup;
    }
    if (!strcmp(participantName, "Node1"))
    {
        frNode->_busState = MasterState_PerformWakeup;
        FlexrayNode_SetStartupDelay(frNode, 0);
    }

    SilKit_HandlerId pocStatusHandlerId;
    returnCode = SilKit_FlexrayController_AddPocStatusHandler(controller, frNode, &FlexrayNode_PocStatusHandler,
                                                           &pocStatusHandlerId);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_AddPocStatusHandler => %s\n", SilKit_GetLastErrorString());
        return 2;
    }
    SilKit_HandlerId frameHandlerId;
    returnCode = SilKit_FlexrayController_AddFrameHandler(controller, frNode, &ReceiveFrame, &frameHandlerId);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_AddFrameHandler => %s\n", SilKit_GetLastErrorString());
        return 2;
    }
    SilKit_HandlerId frameTransmitHandlerId;
    returnCode = SilKit_FlexrayController_AddFrameTransmitHandler(controller, frNode, &ReceiveFrameTransmit,
                                                               &frameTransmitHandlerId);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_AddFrameTransmitHandler => %s\n", SilKit_GetLastErrorString());
        return 2;
    }
    SilKit_HandlerId wakeupHandlerId;
    returnCode =
        SilKit_FlexrayController_AddWakeupHandler(controller, frNode, &FlexrayNode_WakeupHandler, &wakeupHandlerId);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_AddWakeupHandler => %s\n", SilKit_GetLastErrorString());
        return 2;
    }
    SilKit_HandlerId symbolHandlerId;
    returnCode = SilKit_FlexrayController_AddSymbolHandler(controller, frNode, &ReceiveSymbol, &symbolHandlerId);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_AddSymbolHandler => %s\n", SilKit_GetLastErrorString());
        return 2;
    }
    SilKit_HandlerId symbolTransmitHandlerId;
    returnCode = SilKit_FlexrayController_AddSymbolTransmitHandler(controller, frNode, &ReceiveSymbolTransmit,
                                                                &symbolTransmitHandlerId);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_AddSymbolTransmitHandler => %s\n", SilKit_GetLastErrorString());
        return 2;
    }
    SilKit_HandlerId cycleStartHandlerId;
    returnCode =
        SilKit_FlexrayController_AddCycleStartHandler(controller, frNode, &ReceiveCycleStart, &cycleStartHandlerId);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_FlexrayController_AddCycleStartHandler => %s\n", SilKit_GetLastErrorString());
        return 2;
    }

    returnCode = SilKit_TimeSyncService_SetSimulationStepHandler(timeSyncService, frNode, &FlexrayNode_SimulationStep, 1000000);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_TimeSyncService_SetSimulationStepHandler => %s\n", SilKit_GetLastErrorString());
        return 2;
    }

    SilKit_ParticipantState finalState;
    SilKit_LifecycleConfiguration startConfig;
    SilKit_Struct_Init(SilKit_LifecycleConfiguration, startConfig);
    startConfig.operationMode = SilKit_OperationMode_Coordinated;

    returnCode = SilKit_LifecycleService_StartLifecycle(lifecycleService, &startConfig);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_LifecycleService_StartLifecycle => %s\n", SilKit_GetLastErrorString());
        return 2;
    }
    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(lifecycleService, &finalState);
    if (returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("SilKit_LifecycleService_WaitForLifecycleToComplete => %s\n", SilKit_GetLastErrorString());
        return 3;
    }
    printf("Simulation stopped. Final State: %d\n", finalState);
    printf("Press enter to stop the process...\n");
    char line[2];
    char* result = fgets(line, 2, stdin);
    (void)result;

    SilKit_Participant_Destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }
}
