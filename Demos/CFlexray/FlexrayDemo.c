// Copyright (c) Vector Informatik GmbH. All rights reserved.

#define _CRT_SECURE_NO_WARNINGS
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "ib/capi/IntegrationBus.h"

#ifdef WIN32
#pragma warning(disable: 4100)
#include "windows.h"
#define SleepMs(X) Sleep(X)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <unistd.h>
#define SleepMs(X) usleep((X)*1000)
#endif

void AbortOnFailedAllocation(const char* failedAllocStrucName)
{
    fprintf(stderr, "Error: Allocation of \"%s\" failed, aborting...", failedAllocStrucName);
    abort();
}

typedef uint8_t MasterState;
#define  MasterState_Ignore        ((MasterState)0)
#define  MasterState_PerformWakeup ((MasterState)1)
#define  MasterState_WaitForWakeup ((MasterState)2)
#define  MasterState_WakeupDone    ((MasterState)3)

static inline ib_NanosecondsTime milliseconds(unsigned long long msValue)
{
    return msValue * 1000000ULL;
}

struct FlexrayNode
{
    ib_Flexray_Controller*       _controller;
    ib_Flexray_ControllerConfig* _controllerConfig;
    ib_Flexray_PocStatusEvent    _oldPocStatus;
    ib_Bool                      _configureCalled;
    ib_NanosecondsTime           _startupDelay;
    MasterState                  _busState;
};
typedef struct FlexrayNode FlexrayNode;

FlexrayNode* FlexrayNode_Create(ib_Flexray_Controller* controller, ib_Flexray_ControllerConfig* config);
void FlexrayNode_Init(FlexrayNode* flexrayNode);
void FlexrayNode_SetStartupDelay(FlexrayNode* flexrayNode, ib_NanosecondsTime delay);
void FlexrayNode_DoAction(FlexrayNode* flexrayNode, ib_NanosecondsTime now);
void FlexrayNode_PocReady(struct FlexrayNode* flexrayNode, ib_NanosecondsTime now);
void FlexrayNode_TxBufferUpdate(FlexrayNode* flexrayNode, ib_NanosecondsTime now);
void FlexrayNode_ReconfigureTxBuffers(struct FlexrayNode* flexrayNode);
void FlexrayNode_PocStatusHandler(void* context, ib_Flexray_Controller* controller, const ib_Flexray_PocStatusEvent* pocStatus);
void FlexrayNode_WakeupHandler(void* context, ib_Flexray_Controller* controller, const ib_Flexray_SymbolEvent* symbol);

void print_header(FILE* out, ib_Flexray_Header* header)
{
    fprintf(out, "FlexrayHeader{f=[%s%s%s%s],s=%d,l=%d,crc=0x%04x,c=%d}"
        , ((header->flags & ib_Flexray_Header_SuFIndicator) != 0) ? "U" : "-"
        , ((header->flags & ib_Flexray_Header_SyFIndicator) != 0) ? "Y" : "-"
        , ((header->flags & ib_Flexray_Header_NFIndicator) != 0) ? "-" : "N"
        , ((header->flags & ib_Flexray_Header_PPIndicator) != 0) ? "P" : "-"
        , header->frameId
        , header->payloadLength
        , header->headerCrc
        , header->cycleCount);
}

void print_hexbytes(FILE* out, const uint8_t* bytes, size_t size)
{
  size_t i;
  for (i=0; i<size; i++)
  {
    if (i == 0)
      fprintf(out, "%02x", bytes[i]);
    else
      fprintf(out, " %02x", bytes[i]);

  }
  fprintf(out, " \"");
  for (i=0; i<size; i++)
  {
    if (isprint(bytes[i]))
      putc(bytes[i], out);
    else
      putc('.', out);
  }
  fprintf(out, "\"");
}

void print_channel(FILE* out, ib_Flexray_Channel channel)
{
  switch(channel)
  {
    case ib_Flexray_Channel_A: fprintf(out, "A"); break;
    case ib_Flexray_Channel_B: fprintf(out, "B"); break;
    case ib_Flexray_Channel_AB: fprintf(out, "AB"); break;
    default: fprintf(out, "?"); break;
  }
}
void print_flexrayframeevent(FILE* out, const ib_Flexray_FrameEvent* message)
{
  fprintf(out, "FlexrayFrameEvent ch=");
  print_channel(out, message->channel);
  fprintf(out, ",");
  print_header(out, message->frame->header);
  if ((message->frame->header->flags & ib_Flexray_Header_NFIndicator) != 0)
  {
    fprintf(out, ", payload=");
    print_hexbytes(out, message->frame->payload.data, message->frame->payload.size);
  }
  fprintf(out, " @%" PRIu64 "\n", message->timestamp);
  fflush(out);
}

void print_flexrayframetransmitevent(FILE* out, const ib_Flexray_FrameTransmitEvent* message)
{
  fprintf(out, "FlexrayFrameTransmitEvent ch=");print_channel(out, message->channel);fprintf(out, ",");
  print_header(out, message->frame->header);
  fprintf(out, ", txBuffer=%d  @%" PRIu64 "\n", message->txBufferIndex, message->timestamp);
  fflush(out);
}

void ReceiveFrame(void* context, ib_Flexray_Controller* controller, const ib_Flexray_FrameEvent* message)
{
  fprintf(stdout, ">> ");
  print_flexrayframeevent(stdout, message);
}

void ReceiveFrameTransmit(void* context, ib_Flexray_Controller* controller, const ib_Flexray_FrameTransmitEvent* message)
{
  fprintf(stdout, ">> ");
  print_flexrayframetransmitevent(stdout, message);
}

void ReceiveWakeup(void* context, ib_Flexray_Controller* controller, const ib_Flexray_WakeupEvent* message)
{
  fprintf(stdout, ">> ");
  fprintf(stdout, "FlexrayWakeupEvent channel=%d symbol=%d\n", message->channel, message->pattern);
}

void ReceiveSymbol(void* context, ib_Flexray_Controller* controller, const ib_Flexray_SymbolEvent* message)
{
  fprintf(stdout, ">> ");
  fprintf(stdout, "FlexraySymbolEvent channel=%d symbol=%d\n", message->channel, message->pattern);
}

void ReceiveSymbolTransmit(void* context, ib_Flexray_Controller* controller, const ib_Flexray_SymbolTransmitEvent* message)
{
  fprintf(stdout, ">> ");
  fprintf(stdout, "FlexraySymbolTransmitEvent channel=%d symbol=%d\n", message->channel, message->pattern);
}

void ReceiveCycleStart(void* context, ib_Flexray_Controller* controller, const ib_Flexray_CycleStartEvent* message)
{
  fprintf(stdout, ">> ");
  fprintf(stdout, "FlexrayCycleStartEvent cycleCounter=%d\n", message->cycleCounter);
}

void AppendTxBufferConfig(ib_Flexray_ControllerConfig** inOutControllerConfig,
                          const ib_Flexray_TxBufferConfig* txBufferConfig)
{
    uint32_t newNumTxBufferConfigs = (*inOutControllerConfig)->numBufferConfigs + 1;
    size_t newSize = newNumTxBufferConfigs * sizeof(ib_Flexray_TxBufferConfig);
    ib_Flexray_TxBufferConfig* result =
        (ib_Flexray_TxBufferConfig*)realloc((*inOutControllerConfig)->bufferConfigs, newSize);
    if (result == NULL)
    {
        AbortOnFailedAllocation("ib_Flexray_TxBufferConfig");
        return;
    }
    memcpy(&result[newNumTxBufferConfigs - 1], txBufferConfig,
           sizeof(ib_Flexray_TxBufferConfig));
    (*inOutControllerConfig)->numBufferConfigs = newNumTxBufferConfigs;
    (*inOutControllerConfig)->bufferConfigs = result;
}

FlexrayNode* FlexrayNode_Create(ib_Flexray_Controller* controller, ib_Flexray_ControllerConfig* config)
{
    struct FlexrayNode* flexrayNode = malloc(sizeof(FlexrayNode));
    if (flexrayNode == NULL)
    {
        AbortOnFailedAllocation("FlexrayNode");
        return NULL;
    }
    flexrayNode->_controller = controller;
    flexrayNode->_controllerConfig = config;
    flexrayNode->_oldPocStatus.state = ib_Flexray_PocState_DefaultConfig;
    flexrayNode->_configureCalled = ib_False;
    flexrayNode->_startupDelay = 0;
    flexrayNode->_busState = MasterState_Ignore;
    return flexrayNode;
}

void FlexrayNode_SetStartupDelay(FlexrayNode* flexrayNode, ib_NanosecondsTime delay)
{
  flexrayNode->_startupDelay = delay;
}

void FlexrayNode_Init(FlexrayNode* flexrayNode)
{
  ib_ReturnCode returnCode;
  if (flexrayNode->_configureCalled)
    return;

  returnCode = ib_Flexray_Controller_Configure(flexrayNode->_controller, flexrayNode->_controllerConfig);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_Configure => %s\n", ib_GetLastErrorString());
  }
  flexrayNode->_configureCalled = ib_True;
}

void FlexrayNode_DoAction(FlexrayNode* flexrayNode, ib_NanosecondsTime now)
{
  if (now < flexrayNode->_startupDelay)
    return;
  switch (flexrayNode->_oldPocStatus.state)
  {
  case ib_Flexray_PocState_DefaultConfig: FlexrayNode_Init(flexrayNode);
    break;
  case ib_Flexray_PocState_Ready: FlexrayNode_PocReady(flexrayNode, now);
    break;
  case ib_Flexray_PocState_NormalActive:
    if (now == milliseconds(100) + milliseconds(flexrayNode->_startupDelay))
    {
      FlexrayNode_ReconfigureTxBuffers(flexrayNode);
    }
    else
    {
        FlexrayNode_TxBufferUpdate(flexrayNode, now);
    }
    break;
  case ib_Flexray_PocState_Config:
  case ib_Flexray_PocState_Startup:
  case ib_Flexray_PocState_Wakeup:
  case ib_Flexray_PocState_NormalPassive:
  case ib_Flexray_PocState_Halt:
    return;
  }
}

void FlexrayNode_PocReady(FlexrayNode* flexrayNode, ib_NanosecondsTime now)
{
  switch (flexrayNode->_busState)
  {
  case MasterState_PerformWakeup:
    ib_Flexray_Controller_ExecuteCmd(flexrayNode->_controller, ib_Flexray_ChiCommand_WAKEUP);
    break;
  case MasterState_WaitForWakeup:
    break;
  case MasterState_WakeupDone:
    ib_Flexray_Controller_ExecuteCmd(flexrayNode->_controller, ib_Flexray_ChiCommand_ALLOW_COLDSTART);
    ib_Flexray_Controller_ExecuteCmd(flexrayNode->_controller, ib_Flexray_ChiCommand_RUN);
    break;
  default:
    break;
  }
}

void FlexrayNode_TxBufferUpdate(FlexrayNode* flexrayNode, ib_NanosecondsTime now)
{
  if (flexrayNode->_controllerConfig->numBufferConfigs == 0)
    return;

  static uint16_t msgNumber = 0;

  uint16_t bufferIdx = msgNumber % (uint16_t)(flexrayNode->_controllerConfig->numBufferConfigs);

  // prepare a friendly message as payload
  char payloadString[128];
  sprintf(payloadString, "FlexrayFrameEvent#%d sent from buffer %d", msgNumber, bufferIdx);

  ib_Flexray_TxBufferUpdate update;
  update.payload.size = strlen(payloadString + 1);
  update.payload.data = (uint8_t*)payloadString;
  update.payloadDataValid = ib_True;
  update.txBufferIndex = bufferIdx;

  ib_ReturnCode returnCode = ib_Flexray_Controller_UpdateTxBuffer(flexrayNode->_controller, &update);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_UpdateTxBuffer => %s\n", ib_GetLastErrorString());
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
    ib_Flexray_TxBufferConfig* bufferConfig = &(flexrayNode->_controllerConfig->bufferConfigs[idx]);
    switch (bufferConfig->channels)
    {
    case ib_Flexray_Channel_A:
      bufferConfig->channels = ib_Flexray_Channel_B;
      ib_Flexray_Controller_ReconfigureTxBuffer(flexrayNode->_controller, idx, bufferConfig);
      break;
    case ib_Flexray_Channel_B:
      bufferConfig->channels = ib_Flexray_Channel_A;
      ib_Flexray_Controller_ReconfigureTxBuffer(flexrayNode->_controller, idx, bufferConfig);
      break;
    default:
      break;
    }
  }
}

void FlexrayNode_PocStatusHandler(void* context, ib_Flexray_Controller* controller, const ib_Flexray_PocStatusEvent* pocStatus)
{
  FlexrayNode* flexrayNode = (FlexrayNode*)context;
  printf(">> POC=%d freeze=%d wakeupStatus=%d slotMode=%d T=%" PRIu64 "\n", pocStatus->state, pocStatus->freeze, pocStatus->wakeupStatus,  pocStatus->slotMode, pocStatus->timestamp);

  if (flexrayNode->_oldPocStatus.state == ib_Flexray_PocState_Wakeup && pocStatus->state == ib_Flexray_PocState_Ready)
  {
    printf("   Wakeup finished...\n");
    flexrayNode->_busState = MasterState_WakeupDone;
  }

  flexrayNode->_oldPocStatus = *pocStatus;
}

void FlexrayNode_WakeupHandler(void* context, ib_Flexray_Controller* controller, const ib_Flexray_SymbolEvent* symbol)
{
  printf(">> WAKEUP! (%d)\n", symbol->pattern);
  ib_Flexray_Controller_ExecuteCmd(controller, ib_Flexray_ChiCommand_ALLOW_COLDSTART);
  ib_Flexray_Controller_ExecuteCmd(controller, ib_Flexray_ChiCommand_RUN);
}

void FlexrayNode_SimulationTask(void* context, ib_Participant* participant, ib_NanosecondsTime time)
{
  FlexrayNode* node = (FlexrayNode*)context;
  uint64_t nowMs = time / 1000000ULL;
  printf("now=%" PRIu64 "ms\n", nowMs);
  FlexrayNode_DoAction(node, time);
  SleepMs(500);
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

char* LoadFile(char const* path)
{
  size_t length = 0;
  char*  result = NULL;
  FILE*  f = fopen(path, "rb");

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
   ib_Participant* participant;

   ib_Flexray_ClusterParameters clusterParams;
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

   ib_Flexray_NodeParameters nodeParams;
   nodeParams.pAllowHaltDueToClock = 1;
   nodeParams.pAllowPassiveToActive = 0;
   nodeParams.pChannels = ib_Flexray_Channel_AB;
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
   nodeParams.pWakeupChannel = ib_Flexray_Channel_A;
   nodeParams.pWakeupPattern = 33;
   nodeParams.pdMicrotick = ib_Flexray_ClockPeriod_T25NS;
   nodeParams.pSamplesPerMicrotick = 2;

  if (argc < 3)
  {
    printf("usage: IbDemoCFlexray <ConfigJsonFile> <ParticipantName> [<DomainId>]\n");
    return 1;
  }

  char* jsonString = LoadFile(argv[1]);
  if (jsonString == NULL)
  {
      printf("Error: cannot open config file %s\n", argv[1]);
      return 1;
  }
  participantName = argv[2];

  const char* domainId = "42";
  if (argc >= 4)
  {
    domainId = argv[3];
  }

  ib_ReturnCode returnCode;
  returnCode = ib_Participant_Create(&participant, jsonString, participantName, domainId, ib_True);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Participant_Create => %s\n", ib_GetLastErrorString());
    return 2;
  }
  printf("Creating participant '%s' for simulation '%s'\n", participantName, domainId);
  const char* flexrayControllerName = "FlexRay1";
  const char* flexrayNetworkName = "FlexRay1";

  ib_Flexray_ControllerConfig* config = (ib_Flexray_ControllerConfig*)malloc(sizeof(ib_Flexray_ControllerConfig));
  if (config == NULL)
  {
      AbortOnFailedAllocation("ib_Flexray_ControllerConfig");
      return 2;
  }
  memset(config, 0, sizeof(ib_Flexray_ControllerConfig));
  config->clusterParams = &clusterParams;
  config->nodeParams = &nodeParams;

  if (!strcmp(participantName, "Node0"))
  {
    // initialize bufferConfig to send some FrMessages
    ib_Flexray_TxBufferConfig cfg;
    cfg.channels = ib_Flexray_Channel_AB;
    cfg.slotId = 10;
    cfg.offset = 0;
    cfg.repetition = 1;
    cfg.hasPayloadPreambleIndicator = ib_False;
    cfg.headerCrc = 5;
    cfg.transmissionMode = ib_Flexray_TransmissionMode_SingleShot;
    AppendTxBufferConfig(&config, &cfg);

    cfg.channels = ib_Flexray_Channel_A;
    cfg.slotId = 20;
    AppendTxBufferConfig(&config, &cfg);

    cfg.channels = ib_Flexray_Channel_B;
    cfg.slotId = 30;
    AppendTxBufferConfig(&config, &cfg);

  }
  else if (!strcmp(participantName, "Node1"))
  {
    config->nodeParams->pKeySlotId = 11;

    // initialize bufferConfig to send some FrMessages
    ib_Flexray_TxBufferConfig cfg;
    cfg.channels = ib_Flexray_Channel_AB;
    cfg.slotId = 11;
    cfg.offset = 0;
    cfg.repetition = 1;
    cfg.hasPayloadPreambleIndicator = ib_False;
    cfg.headerCrc = 5;
    cfg.transmissionMode = ib_Flexray_TransmissionMode_SingleShot;
    AppendTxBufferConfig(&config, &cfg);

    cfg.channels = ib_Flexray_Channel_A;
    cfg.slotId = 21;
    AppendTxBufferConfig(&config, &cfg);

    cfg.channels = ib_Flexray_Channel_B;
    cfg.slotId = 31;
    AppendTxBufferConfig(&config, &cfg);
  }

  ib_Flexray_Controller* controller;
  returnCode = ib_Flexray_Controller_Create(&controller, participant, flexrayControllerName, flexrayNetworkName);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_Create => %s\n", ib_GetLastErrorString());
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

  returnCode = ib_Flexray_Controller_AddPocStatusHandler(controller, frNode, &FlexrayNode_PocStatusHandler);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_AddPocStatusHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_Flexray_Controller_AddFrameHandler(controller, frNode, &ReceiveFrame);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_AddFrameHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_Flexray_Controller_AddFrameTransmitHandler(controller, frNode, &ReceiveFrameTransmit);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_AddFrameTransmitHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_Flexray_Controller_AddWakeupHandler(controller, frNode, &FlexrayNode_WakeupHandler);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_AddWakeupHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_Flexray_Controller_AddSymbolHandler(controller, frNode, &ReceiveSymbol);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_AddSymbolHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_Flexray_Controller_AddSymbolTransmitHandler(controller, frNode, &ReceiveSymbolTransmit);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_AddSymbolTransmitHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_Flexray_Controller_AddCycleStartHandler(controller, frNode, &ReceiveCycleStart);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Flexray_Controller_AddCycleStartHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }

  returnCode = ib_Participant_SetSimulationTask(participant, frNode, &FlexrayNode_SimulationTask);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Participant_SetSimulationTask => %s\n", ib_GetLastErrorString());
    return 2;
  }

  ib_ParticipantState finalState;
  returnCode = ib_Participant_Run(participant, &finalState);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_Participant_Run => %s\n", ib_GetLastErrorString());
    return 2;
  }
  printf("Simulation stopped. Final State: %d\n", finalState);
  printf("Press enter to stop the process...\n");
  char line[2];
  char* result = fgets(line, 2, stdin);
  (void)result;

  ib_Participant_Destroy(participant);
  if (jsonString)
  {
      free(jsonString);
  }
}

#ifndef WIN32
#pragma GCC diagnostic pop
#endif
