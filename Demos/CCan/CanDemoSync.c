/* Copyright (c) Vector Informatik GmbH. All rights reserved. */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>

#include "ib/capi/IntegrationBus.h"

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

ib_Participant* participant;
ib_Can_Controller* canController;
ib_Can_Controller* canController2;

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

void InitCallback(void* context, ib_Participant* cbParticipant)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbParticipant);
    ParticipantHandlerContext* tc = (ParticipantHandlerContext*)context;
    printf(">> InitCallback of with context=%i\n", tc->someInt);

    /* Set baud rate and start the controllers. We omitted the return value check for brevity.*/
    (void)ib_Can_Controller_SetBaudRate(canController, 10000u, 1000000u);
    (void)ib_Can_Controller_SetBaudRate(canController2, 10000u, 1000000u);
    (void)ib_Can_Controller_Start(canController);
    (void)ib_Can_Controller_Start(canController2);
}

void StopCallback(void* context, ib_Participant* cbParticipant)
{
    UNUSED_ARG(cbParticipant);
    ParticipantHandlerContext* tc = (ParticipantHandlerContext*)context;
    printf(">> StopCallback with context=%i\n", tc->someInt);
}

void ShutdownCallback(void* context, ib_Participant* cbParticipant)
{
    UNUSED_ARG(cbParticipant);
    ParticipantHandlerContext* tc = (ParticipantHandlerContext*)context;
    printf(">> ShutdownCallback with context=%i\n", tc->someInt);
}

void FrameTransmitHandler(void* context, ib_Can_Controller* controller, struct ib_Can_FrameTransmitEvent* cAck)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);
    //TransmitContext* tc = (TransmitContext*) cAck->userContext;
    printf(">> %i for CAN Message with timestamp=%"PRIu64"\n", cAck->status, cAck->timestamp);
}

void FrameHandler(void* context, ib_Can_Controller* controller, ib_Can_FrameEvent* frameEvent)
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

    ib_Can_Frame canFrame;
    canFrame.id = 17;
    canFrame.flags = ib_Can_FrameFlag_brs;

    char payload[64];
    canMessageCounter += 1;
    uint8_t payloadSize = (uint8_t)snprintf(payload, sizeof(payload), "CAN %i", canMessageCounter);

    canFrame.data.data = (uint8_t*)&payload[0];
    canFrame.data.size = payloadSize;
    canFrame.dlc = payloadSize;

    transmitContext.someInt = 1234;
    ib_Can_Controller_SendFrame(canController, &canFrame, (void*)&transmitContext);
    printf("CAN Message sent with transmitId=%i\n", transmitContext.someInt);
}

void SimTask(void* context, ib_Participant* cbParticipant, ib_NanosecondsTime now)
{
    UNUSED_ARG(cbParticipant);
    SimTaskContext* tc = (SimTaskContext*)context;
    printf(">> Simulation task now=%"PRIu64" with context=%i\n", now, tc->someInt);

    SendFrame();
    SleepMs(100);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("usage: IbDemoCCan <ConfigJsonFile> <ParticipantName> [<DomainId>]\n");
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
    if (returnCode) {
        printf("%s\n", ib_GetLastErrorString());
        return 2;
    }
    printf("Creating participant '%s' for simulation '%s'\n", participantName, domainId);

    participantHandlerContext.someInt = 123;
    ib_Participant_SetInitHandler(participant, (void*)&participantHandlerContext, &InitCallback);
    ib_Participant_SetStopHandler(participant, (void*)&participantHandlerContext, &StopCallback);
    ib_Participant_SetShutdownHandler(participant, (void*)&participantHandlerContext, &ShutdownCallback);

    const char* canNetworkName = "CAN1";
    const char* canControllerName = "CAN1";
    returnCode = ib_Can_Controller_Create(&canController, participant, canControllerName, canNetworkName);

    const char* canController2Name = "CAN2";
    returnCode = ib_Can_Controller_Create(&canController2, participant, canController2Name, canNetworkName);

    ib_HandlerId frameTransmitHandlerId;
    ib_Can_Controller_AddFrameTransmitHandler(
        canController, (void*)&transmitContext, &FrameTransmitHandler,
        ib_Can_TransmitStatus_Transmitted | ib_Can_TransmitStatus_Canceled | ib_Can_TransmitStatus_TransmitQueueFull,
        &frameTransmitHandlerId);

    ib_HandlerId frameHandlerId;
    ib_Can_Controller_AddFrameHandler(canController2, (void*)&transmitContext, &FrameHandler, ib_Direction_SendReceive,
                                      &frameHandlerId);
    simTaskContext.someInt = 456;
    ib_Participant_SetPeriod(participant, 1000000);
    ib_Participant_SetSimulationTask(participant, (void*)&simTaskContext, &SimTask);

    ib_ReturnCode result;
    result = ib_Participant_ExecuteLifecycleWithSyncTime(participant, ib_True, ib_True, ib_True);
    if(result != ib_ReturnCode_SUCCESS)
    {
        printf("Error: ib_Participant_ExecuteLifecycleWithSyncTime failed: %s\n", ib_GetLastErrorString());
        exit(1);
    }
    ib_ParticipantState outFinalParticipantState;
    result = ib_Participant_WaitForLifecycleToComplete(participant, &outFinalParticipantState);
    if(result != ib_ReturnCode_SUCCESS)
    {
        printf("Error: ib_Participant_WaitForLifecycleToComplete failed: %s\n", ib_GetLastErrorString());
        exit(1);
    }


    ib_Participant_Destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }

    return EXIT_SUCCESS;
}
