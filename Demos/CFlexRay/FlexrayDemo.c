// Copyright (c) Vector Informatik GmbH. All rights reserved.

#define _CRT_SECURE_NO_WARNINGS
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "ib/capi/IntegrationBus.h"

#ifdef WIN32
#include "Windows.h"
#define SleepMs(X) Sleep(X)
#else
#include "unistd.h"
#define SleepMs(X) usleep((X)*1000)
#endif

typedef uint8_t MasterState;
#define  MasterState_Ignore        ((MasterState)0)
#define  MasterState_PerformWakeup ((MasterState)1)
#define  MasterState_WaitForWakeup ((MasterState)2)
#define  MasterState_WakeupDone    ((MasterState)3)

static inline ib_NanosecondsTime milliseconds(unsigned long long msValue)
{
  return msValue * 1000000ULL;
}

struct FlexRayNode
{
  ib_FlexRay_Controller*     _controller;
  ib_FlexRay_ControllerConfig* _controllerConfig;
  ib_FlexRay_PocStatus         _oldPocStatus;
  ib_Bool                      _configureCalled;
  ib_NanosecondsTime           _startupDelay;
  MasterState                  _busState;
};
typedef struct FlexRayNode FlexRayNode;

FlexRayNode* FlexRayNode_Create(ib_FlexRay_Controller* controller, ib_FlexRay_ControllerConfig* config);
void FlexRayNode_Init(FlexRayNode* flexRayNode);
void FlexRayNode_SetStartupDelay(FlexRayNode* flexRayNode, ib_NanosecondsTime delay);
void FlexRayNode_pocReady(struct FlexRayNode* flexRayNode, ib_NanosecondsTime now);
void FlexRayNode_txBufferUpdate(FlexRayNode* flexRayNode, ib_NanosecondsTime now);
void FlexRayNode_ReconfigureTxBuffers(struct FlexRayNode* flexRayNode);
void FlexRayNode_PocStatusHandler(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_PocStatus* pocStatus);
void FlexRayNode_WakeupHandler(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_Symbol* symbol);

void print_header(FILE* out, ib_FlexRay_Header* header)
{
    fprintf(out, "Header{f=[%s%s%s%s],s=%d,l=%d,crc=0x%04x,c=%d}"
        , ((header->flags & ib_FlexRay_Header_SuFIndicator) != 0) ? "U" : "-"
        , ((header->flags & ib_FlexRay_Header_SyFIndicator) != 0) ? "Y" : "-"
        , ((header->flags & ib_FlexRay_Header_NFIndicator) != 0) ? "-" : "N"
        , ((header->flags & ib_FlexRay_Header_PPIndicator) != 0) ? "P" : "-"
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

void print_channel(FILE* out, ib_FlexRay_Channel channel)
{
  switch(channel)
  {
    case ib_FlexRay_Channel_A: fprintf(out, "A"); break;
    case ib_FlexRay_Channel_B: fprintf(out, "B"); break;
    case ib_FlexRay_Channel_AB: fprintf(out, "AB"); break;
    default: fprintf(out, "?"); break;
  }
}
void print_frmessage(FILE* out, const ib_FlexRay_Message* message)
{
  fprintf(out, "FrMessage ch=");
  print_channel(out, message->channel);
  fprintf(out, ",");
  print_header(out, &(message->frame->header));
  if ((message->frame->header.flags & ib_FlexRay_Header_NFIndicator) != 0)
  {
    fprintf(out, ", payload=");
    print_hexbytes(out, message->frame->payload.data, message->frame->payload.size);
  }
  fprintf(out, " @%" PRIu64 "\n", message->timestamp);
  fflush(out);
}

void print_frmessageack(FILE* out, const ib_FlexRay_MessageAck* message)
{
  fprintf(out, "FrMessageAck ch=");print_channel(out, message->channel);fprintf(out, ",");
  print_header(out, &(message->frame->header));
  fprintf(out, ", txBuffer=%d  @%" PRIu64 "\n", message->txBufferIndex, message->timestamp);
  fflush(out);
}

void ReceiveMessage(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_Message* message)
{
  fprintf(stdout, ">> ");
  print_frmessage(stdout, message);
}

void ReceiveMessageAck(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_MessageAck* message)
{
  fprintf(stdout, ">> ");
  print_frmessageack(stdout, message);
}

void ReceiveWakeup(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_Symbol* message)
{
  fprintf(stdout, ">> ");
  fprintf(stdout, "Symbol channel=%d symbol=%d\n", message->channel, message->pattern);
}

void ReceiveSymbol(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_Symbol* message)
{
  fprintf(stdout, ">> ");
  fprintf(stdout, "Symbol channel=%d symbol=%d\n", message->channel, message->pattern);
}

void ReceiveSymbolAck(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_SymbolAck* message)
{
  fprintf(stdout, ">> ");
  fprintf(stdout, "SymbolAck channel=%d symbol=%d\n", message->channel, message->pattern);
}

void ReceiveCycleStart(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_CycleStart* message)
{
  fprintf(stdout, ">> ");
  fprintf(stdout, "CycleStart cycleCounter=%d\n", message->cycleCounter);
}

FlexRayNode* FlexRayNode_Create(ib_FlexRay_Controller* controller, ib_FlexRay_ControllerConfig* config)
{
  struct FlexRayNode* flexRayNode = malloc(sizeof(FlexRayNode));
  flexRayNode->_controller = controller;
  flexRayNode->_controllerConfig = config;
  flexRayNode->_oldPocStatus.state = ib_FlexRay_PocState_DefaultConfig;
  flexRayNode->_configureCalled = ib_False;
  flexRayNode->_startupDelay = 0;
  flexRayNode->_busState = MasterState_Ignore;
  return flexRayNode;
}

void FlexRayNode_SetStartupDelay(FlexRayNode* flexRayNode, ib_NanosecondsTime delay)
{
  flexRayNode->_startupDelay = delay;
}

void FlexRayNode_Init(FlexRayNode* flexRayNode)
{
  ib_ReturnCode returnCode;
  if (flexRayNode->_configureCalled)
    return;

  returnCode = ib_FlexRay_Controller_Configure(flexRayNode->_controller, flexRayNode->_controllerConfig);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_Configure => %s\n", ib_GetLastErrorString());
  }
  flexRayNode->_configureCalled = ib_True;
}

void FlexRayNode_doAction(FlexRayNode* flexRayNode, ib_NanosecondsTime now)
{
  if (now < flexRayNode->_startupDelay)
    return;
  switch (flexRayNode->_oldPocStatus.state)
  {
  case ib_FlexRay_PocState_DefaultConfig:
    FlexRayNode_Init(flexRayNode);
    break;
  case ib_FlexRay_PocState_Ready:
    FlexRayNode_pocReady(flexRayNode, now);
    break;
  case ib_FlexRay_PocState_NormalActive:
    if (now == milliseconds(100) + milliseconds(flexRayNode->_startupDelay))
    {
      FlexRayNode_ReconfigureTxBuffers(flexRayNode);
    }
    else
    {
      FlexRayNode_txBufferUpdate(flexRayNode, now);
    }
    break;
  case ib_FlexRay_PocState_Config:
  case ib_FlexRay_PocState_Startup:
  case ib_FlexRay_PocState_Wakeup:
  case ib_FlexRay_PocState_NormalPassive:
  case ib_FlexRay_PocState_Halt:
    return;
  }
}

void FlexRayNode_pocReady(FlexRayNode* flexRayNode, ib_NanosecondsTime now)
{
  switch (flexRayNode->_busState)
  {
  case MasterState_PerformWakeup:
    ib_FlexRay_Controller_ExecuteCmd(flexRayNode->_controller, ib_FlexRay_ChiCommand_WAKEUP);
    break;
  case MasterState_WaitForWakeup:
    break;
  case MasterState_WakeupDone:
    ib_FlexRay_Controller_ExecuteCmd(flexRayNode->_controller, ib_FlexRay_ChiCommand_ALLOW_COLDSTART);
    ib_FlexRay_Controller_ExecuteCmd(flexRayNode->_controller, ib_FlexRay_ChiCommand_RUN);
    break;
  default:
    break;
  }
}

void FlexRayNode_txBufferUpdate(FlexRayNode* flexRayNode, ib_NanosecondsTime now)
{
  if (flexRayNode->_controllerConfig->numBufferConfigs == 0)
    return;

  static int msgNumber = -1;
  msgNumber++;

  int bufferIdx = msgNumber % flexRayNode->_controllerConfig->numBufferConfigs;

  // prepare a friendly message as payload
  char payloadString[128];
  sprintf(payloadString, "FrMessage#%d sent from buffer %d", msgNumber, bufferIdx);

  ib_FlexRay_TxBufferUpdate update;
  update.payload.size = strlen(payloadString + 1);
  update.payload.data = (uint8_t*)payloadString;
  update.payloadDataValid = ib_True;
  update.txBufferIndex = bufferIdx;

  ib_ReturnCode returnCode = ib_FlexRay_Controller_UpdateTxBuffer(flexRayNode->_controller, &update);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_UpdateTxBuffer => %s\n", ib_GetLastErrorString());
  }
}

// Reconfigure buffers: Swap Channels A and B
void FlexRayNode_ReconfigureTxBuffers(struct FlexRayNode* flexRayNode)
{
  printf("Reconfiguring TxBuffers. Swapping Channel::A and Channel::B\n");
  uint16_t idx;
  for (idx = 0; idx < flexRayNode->_controllerConfig->numBufferConfigs; idx++)
  {
    ib_FlexRay_TxBufferConfig* bufferConfig = &(flexRayNode->_controllerConfig->bufferConfigs[idx]);
    switch (bufferConfig->channels)
    {
    case ib_FlexRay_Channel_A:
      bufferConfig->channels = ib_FlexRay_Channel_B;
      ib_FlexRay_Controller_ReconfigureTxBuffer(flexRayNode->_controller, idx, bufferConfig);
      break;
    case ib_FlexRay_Channel_B:
      bufferConfig->channels = ib_FlexRay_Channel_A;
      ib_FlexRay_Controller_ReconfigureTxBuffer(flexRayNode->_controller, idx, bufferConfig);
      break;
    default:
      break;
    }
  }
}

void FlexRayNode_PocStatusHandler(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_PocStatus* pocStatus)
{
  FlexRayNode* flexRayNode = (FlexRayNode*)context;
  printf(">> POC=%d freeze=%d wakeupStatus=%d slotMode=%d T=%" PRIu64 "\n", pocStatus->state, pocStatus->freeze, pocStatus->wakeupStatus,  pocStatus->slotMode, pocStatus->timestamp);

  if (flexRayNode->_oldPocStatus.state == ib_FlexRay_PocState_Wakeup && pocStatus->state == ib_FlexRay_PocState_Ready)
  {
    printf("   Wakeup finished...\n");
    flexRayNode->_busState = MasterState_WakeupDone;
  }

  flexRayNode->_oldPocStatus = *pocStatus;
}

void FlexRayNode_WakeupHandler(void* context, ib_FlexRay_Controller* controller, const ib_FlexRay_Symbol* symbol)
{
  FlexRayNode* flexRayNode = (FlexRayNode*)context;
  printf(">> WAKEUP! (%d)\n", symbol->pattern);
  ib_FlexRay_Controller_ExecuteCmd(controller, ib_FlexRay_ChiCommand_ALLOW_COLDSTART);
  ib_FlexRay_Controller_ExecuteCmd(controller, ib_FlexRay_ChiCommand_RUN);
}

void FlexRayNode_SimulationTask(void* context, ib_SimulationParticipant* participant, ib_NanosecondsTime time)
{
  FlexRayNode* node = (FlexRayNode*)context;
  uint64_t nowMs = time / 1000000ULL;
  printf("now=%" PRIu64 "ms\n", nowMs);
  FlexRayNode_doAction(node, time);
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
   ib_SimulationParticipant* participant;

   ib_FlexRay_ClusterParameters clusterParams;
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

   ib_FlexRay_NodeParameters nodeParams;
   nodeParams.pAllowHaltDueToClock = 1;
   nodeParams.pAllowPassiveToActive = 0;
   nodeParams.pChannels = ib_FlexRay_Channel_AB;
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
   nodeParams.pWakeupChannel = ib_FlexRay_Channel_A;
   nodeParams.pWakeupPattern = 33;
   nodeParams.pdMicrotick = ib_FlexRay_ClockPeriod_T25NS;
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
  returnCode = ib_SimulationParticipant_Create(&participant, jsonString, participantName, ib_True, domainId);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_SimulationParticipant_Create => %s\n", ib_GetLastErrorString());
    return 2;
  }
  printf("Creating Participant %s for simulation '%s'\n", participantName, domainId);
  // NOTE: must know the name, as there is currently no way to enumerate the configured FR controllers
  const char* flexrayControllerName = "FlexRay1";
  const char* flexrayNetworkName = "FlexRay1";

  ib_FlexRay_ControllerConfig* config = (ib_FlexRay_ControllerConfig*)malloc(sizeof(ib_FlexRay_ControllerConfig));
  memset(config, 0, sizeof(ib_FlexRay_ControllerConfig));
  config->clusterParams = clusterParams;
  config->nodeParams = nodeParams;


  if (!strcmp(participantName, "Node0"))
  {
    // initialize bufferConfig to send some FrMessages
    ib_FlexRay_TxBufferConfig cfg;
    cfg.channels = ib_FlexRay_Channel_AB;
    cfg.slotId = 10;
    cfg.offset = 0;
    cfg.repetition = 1;
    cfg.hasPayloadPreambleIndicator = ib_False;
    cfg.headerCrc = 5;
    cfg.transmissionMode = ib_FlexRay_TransmissionMode_SingleShot;
    returnCode = ib_FlexRay_Append_TxBufferConfig(&config, &cfg);
    if (returnCode != ib_ReturnCode_SUCCESS)
    {
      printf("ib_FlexRay_Append_TxBufferConfig => %s\n", ib_GetLastErrorString());
      return 2;
    }

    cfg.channels = ib_FlexRay_Channel_A;
    cfg.slotId = 20;
    returnCode = ib_FlexRay_Append_TxBufferConfig(&config, &cfg);
    if (returnCode != ib_ReturnCode_SUCCESS)
    {
      printf("ib_FlexRay_Append_TxBufferConfig => %s\n", ib_GetLastErrorString());
      return 2;
    }

    cfg.channels = ib_FlexRay_Channel_B;
    cfg.slotId = 30;
    returnCode = ib_FlexRay_Append_TxBufferConfig(&config, &cfg);
    if (returnCode != ib_ReturnCode_SUCCESS)
    {
      printf("ib_FlexRay_Append_TxBufferConfig => %s\n", ib_GetLastErrorString());
      return 2;
    }
  }
  else if (!strcmp(participantName, "Node1"))
  {
    // initialize bufferConfig to send some FrMessages
    ib_FlexRay_TxBufferConfig cfg;
    cfg.channels = ib_FlexRay_Channel_AB;
    cfg.slotId = 11;
    cfg.offset = 0;
    cfg.repetition = 1;
    cfg.hasPayloadPreambleIndicator = ib_False;
    cfg.headerCrc = 5;
    cfg.transmissionMode = ib_FlexRay_TransmissionMode_SingleShot;
    returnCode = ib_FlexRay_Append_TxBufferConfig(&config, &cfg);
    if (returnCode != ib_ReturnCode_SUCCESS)
    {
      printf("ib_FlexRay_Append_TxBufferConfig => %s\n", ib_GetLastErrorString());
      return 2;
    }

    cfg.channels = ib_FlexRay_Channel_A;
    cfg.slotId = 21;
    returnCode = ib_FlexRay_Append_TxBufferConfig(&config, &cfg);
    if (returnCode != ib_ReturnCode_SUCCESS)
    {
      printf("ib_FlexRay_Append_TxBufferConfig => %s\n", ib_GetLastErrorString());
      return 2;
    }

    cfg.channels = ib_FlexRay_Channel_B;
    cfg.slotId = 31;
    returnCode = ib_FlexRay_Append_TxBufferConfig(&config, &cfg);
    if (returnCode != ib_ReturnCode_SUCCESS)
    {
      printf("ib_FlexRay_Append_TxBufferConfig => %s\n", ib_GetLastErrorString());
      return 2;
    }
}


  ib_FlexRay_Controller* controller;
  returnCode = ib_FlexRay_Controller_Create(&controller, participant, flexrayControllerName, flexrayNetworkName);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_Create => %s\n", ib_GetLastErrorString());
    return 2;
  }
  FlexRayNode* frNode = FlexRayNode_Create(controller, config);
  if (!strcmp(participantName, "Node0"))
  {
    frNode->_busState = MasterState_PerformWakeup;
  }
  if (!strcmp(participantName, "Node1"))
  {
    frNode->_busState = MasterState_PerformWakeup;
    FlexRayNode_SetStartupDelay(frNode, 0);
  }

  returnCode = ib_FlexRay_Controller_RegisterPocStatusHandler(controller, frNode, &FlexRayNode_PocStatusHandler);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_RegisterPocStatusHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_FlexRay_Controller_RegisterMessageHandler(controller, frNode, &ReceiveMessage);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_RegisterMessageHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_FlexRay_Controller_RegisterMessageAckHandler(controller, frNode, &ReceiveMessageAck);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_RegisterMessageAckHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_FlexRay_Controller_RegisterWakeupHandler(controller, frNode, &FlexRayNode_WakeupHandler);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_RegisterWakeupHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_FlexRay_Controller_RegisterSymbolHandler(controller, frNode, &ReceiveSymbol);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_RegisterSymbolHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_FlexRay_Controller_RegisterSymbolAckHandler(controller, frNode, &ReceiveSymbolAck);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_RegisterSymbolAckHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }
  returnCode = ib_FlexRay_Controller_RegisterCycleStartHandler(controller, frNode, &ReceiveCycleStart);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_FlexRay_Controller_RegisterCycleStartHandler => %s\n", ib_GetLastErrorString());
    return 2;
  }

  // premature API: to be replaced
  returnCode = ib_SimulationParticipant_SetSimulationTask(participant, frNode, &FlexRayNode_SimulationTask);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_SimulationParticipant_SetSimulationTask => %s\n", ib_GetLastErrorString());
    return 2;
  }

  ib_ParticipantState finalState;
  returnCode = ib_SimulationParticipant_Run(participant, &finalState);
  if (returnCode != ib_ReturnCode_SUCCESS)
  {
    printf("ib_SimulationParticipant_Run => %s\n", ib_GetLastErrorString());
    return 2;
  }
  printf("Simulation stopped. Final State: %d\n", finalState);
  printf("Press enter to stop the process...\n");
  char line[2];
  char* result = fgets(line, 2, stdin);
  (void)result;

  ib_SimulationParticipant_Destroy(participant);
  if (jsonString)
  {
      free(jsonString);
  }
}
