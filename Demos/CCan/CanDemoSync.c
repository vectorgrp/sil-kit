/* Copyright (c) Vector Informatik GmbH. All rights reserved. */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>

#include "silkit/capi/SilKit.h"

#ifdef WIN32
#pragma warning(disable : 5105)
#include "windows.h"
#define SleepMs(X) Sleep(X)
#else
#include <unistd.h>
#define SleepMs(X) usleep((X)*1000)
#endif
#define UNUSED_ARG(X) (void)(X)

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
            if(num != length)
            {
                printf("Warning: short read on config file: %zu/%zu",
                    num, length);
                exit(1);
            }
            buffer[length] = '\0';
            result = buffer;
        }
        fclose(f);
    }
    return result;
}

SilKit_Participant* participant;
SilKit_CanController* canController;
SilKit_CanController* canController2;

char* participantName;
uint8_t canMessageCounter = 0;

bool runDone = false;

typedef struct  {
    uint32_t someInt;
} TransmitContext;
TransmitContext transmitContext;

typedef struct {
    uint32_t someInt;
} ParticipantHandlerContext;
ParticipantHandlerContext participantHandlerContext;

typedef struct {
    uint32_t someInt;
} SimTaskContext;
SimTaskContext simTaskContext;

void CommunicationReadyCallback(void* context, SilKit_LifecycleService* cbParticipant)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbParticipant);
    ParticipantHandlerContext* tc = (ParticipantHandlerContext*)context;
    printf(">> CommunicationReadyCallback of with context=%i\n", tc->someInt);

    /* Set baud rate and start the controllers. We omitted the return value check for brevity.*/
    (void)SilKit_CanController_SetBaudRate(canController, 10000u, 1000000u, 2000000u);
    (void)SilKit_CanController_SetBaudRate(canController2, 10000u, 1000000u, 2000000u);
    (void)SilKit_CanController_Start(canController);
    (void)SilKit_CanController_Start(canController2);
}

void StopCallback(void* context, SilKit_LifecycleService* cbLifecycleService)
{
    UNUSED_ARG(cbLifecycleService);
    ParticipantHandlerContext* tc = (ParticipantHandlerContext*)context;
    printf(">> StopCallback with context=%i\n", tc->someInt);
}

void ShutdownCallback(void* context, SilKit_LifecycleService* cbLifecycleService)
{
    UNUSED_ARG(cbLifecycleService);
    ParticipantHandlerContext* tc = (ParticipantHandlerContext*)context;
    printf(">> ShutdownCallback with context=%i\n", tc->someInt);
}

void FrameTransmitHandler(void* context, SilKit_CanController* controller, struct SilKit_CanFrameTransmitEvent* cAck)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);
    //TransmitContext* tc = (TransmitContext*) cAck->userContext;
    printf(">> %i for CAN Message with timestamp=%"PRIu64"\n", cAck->status, cAck->timestamp);
}

void FrameHandler(void* context, SilKit_CanController* controller, SilKit_CanFrameEvent* frameEvent)
{
    UNUSED_ARG(controller);
    TransmitContext* txContext = (TransmitContext*)(context);
    unsigned int i;
    printf(">> CAN frameEvent: canId=%i timestamp=%"PRIu64" ",
        frameEvent->frame->id, frameEvent->timestamp);
    if (txContext != NULL)
    {
        printf("transmitContext=%d ", txContext->someInt);
    }

    printf(": ");

    for (i = 0; i < frameEvent->frame->data.size; i++)
    {
        char ch = frameEvent->frame->data.data[i];
        if (isalnum(ch))
        {
            printf("%c", ch);
        }
        else
        {
            printf("<%x>", frameEvent->frame->data.data[i]);
        }
    }
    printf("\n");
}

void SendFrame()
{

    SilKit_CanFrame canFrame;
    canFrame.id = 17;
    canFrame.flags = SilKit_CanFrameFlag_brs;

    char payload[64];
    canMessageCounter += 1;
    uint8_t payloadSize = (uint8_t)snprintf(payload, sizeof(payload), "CAN %i", canMessageCounter);

    canFrame.data.data = (uint8_t*)&payload[0];
    canFrame.data.size = payloadSize;
    canFrame.dlc = payloadSize;

    transmitContext.someInt = 1234;
    SilKit_CanController_SendFrame(canController, &canFrame, (void*)&transmitContext);
    printf("CAN Message sent with transmitId=%i\n", transmitContext.someInt);
}

void SimTask(void* context, SilKit_TimeSyncService* cbTimeSyncService, SilKit_NanosecondsTime now,
             SilKit_NanosecondsTime duration)
{
    UNUSED_ARG(cbTimeSyncService);
    SimTaskContext* tc = (SimTaskContext*)context;
    printf(">> Simulation task now=%"PRIu64" and duration=%"PRIu64" with context=%i\n", now, duration, tc->someInt);

    SendFrame();
    SleepMs(100);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("usage: SilKitDemoCCan <ConfigJsonFile> <ParticipantName> [RegistryUri]\n");
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
    returnCode = SilKit_Participant_Create(&participant, jsonString, participantName, registryUri, SilKit_True);
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }
    printf("Creating participant '%s' for simulation '%s'\n", participantName, registryUri);

    SilKit_LifecycleService* lifecycleService;
    returnCode = SilKit_LifecycleServiceWithTimeSync_Create(&lifecycleService, participant);

    SilKit_TimeSyncService* timesyncService;
    returnCode = SilKit_TimeSyncService_Create(&timesyncService, lifecycleService);

    participantHandlerContext.someInt = 123;
    SilKit_LifecycleService_SetCommunicationReadyHandler(lifecycleService, (void*)&participantHandlerContext, &CommunicationReadyCallback);
    SilKit_LifecycleService_SetStopHandler(lifecycleService, (void*)&participantHandlerContext, &StopCallback);
    SilKit_LifecycleService_SetShutdownHandler(lifecycleService, (void*)&participantHandlerContext, &ShutdownCallback);

    const char* canNetworkName = "CAN1";
    const char* canControllerName = "CAN1";
    returnCode = SilKit_CanController_Create(&canController, participant, canControllerName, canNetworkName);

    const char* canController2Name = "CAN2";
    returnCode = SilKit_CanController_Create(&canController2, participant, canController2Name, canNetworkName);

    SilKit_HandlerId frameTransmitHandlerId;
    SilKit_CanController_AddFrameTransmitHandler(
        canController, (void*)&transmitContext, &FrameTransmitHandler,
        SilKit_CanTransmitStatus_Transmitted | SilKit_CanTransmitStatus_Canceled | SilKit_CanTransmitStatus_TransmitQueueFull,
        &frameTransmitHandlerId);

    SilKit_HandlerId frameHandlerId;
    SilKit_CanController_AddFrameHandler(canController2, (void*)&transmitContext, &FrameHandler, SilKit_Direction_SendReceive,
                                      &frameHandlerId);
    simTaskContext.someInt = 456;

    SilKit_TimeSyncService_SetSimulationStepHandler(timesyncService, (void*)&simTaskContext, &SimTask, 1000000);

    SilKit_ReturnCode result;
    SilKit_LifecycleConfiguration startConfig;
    startConfig.coordinatedStart = SilKit_True;
    startConfig.coordinatedStop = SilKit_True;
    result = SilKit_LifecycleService_StartLifecycle(lifecycleService, &startConfig);
    if(result != SilKit_ReturnCode_SUCCESS)
    {
        printf("Error: SilKit_LifecycleService_StartLifecycle failed: %s\n", SilKit_GetLastErrorString());
        exit(1);
    }
    SilKit_ParticipantState outFinalParticipantState;
    result = SilKit_LifecycleService_WaitForLifecycleToComplete(lifecycleService, &outFinalParticipantState);
    if(result != SilKit_ReturnCode_SUCCESS)
    {
        printf("Error: SilKit_LifecycleService_WaitForLifecycleToComplete failed: %s\n", SilKit_GetLastErrorString());
        exit(1);
    }


    SilKit_Participant_Destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }

    return EXIT_SUCCESS;
}
