/* Copyright (c) Vector Informatik GmbH. All rights reserved. */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "ib/capi/IntegrationBus.h"

#ifdef WIN32
#include "Windows.h"
#   define SleepMs(X) Sleep(X)
#else
#   include "unistd.h"
#define SleepMs(X) usleep((X)*1000)
#endif

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

ib_SimulationParticipant* participant;
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

void InitCallback(void* context, ib_SimulationParticipant* participant, struct ib_ParticipantCommand* command)
{
    ParticipantHandlerContext* tc = (ParticipantHandlerContext*)context;
    printf(">> InitCallback of kind=%i with context=%i\n", command->kind, tc->someInt);

    /* Set baud rate and start the controllers. We omitted the return value check for brevity.*/
    (void)ib_Can_Controller_SetBaudRate(canController, 10000u, 1000000u);
    (void)ib_Can_Controller_SetBaudRate(canController2, 10000u, 1000000u);
    (void)ib_Can_Controller_Start(canController);
    (void)ib_Can_Controller_Start(canController2);
}

void StopCallback(void* context, ib_SimulationParticipant* participant)
{
    ParticipantHandlerContext* tc = (ParticipantHandlerContext*)context;
    printf(">> StopCallback with context=%i\n", tc->someInt);
}

void ShutdownCallback(void* context, ib_SimulationParticipant* participant)
{
    ParticipantHandlerContext* tc = (ParticipantHandlerContext*)context;
    printf(">> ShutdownCallback with context=%i\n", tc->someInt);
}

void AckCallback(void* context, ib_Can_Controller* controller, struct ib_Can_TransmitAcknowledge* cAck)
{
    //TransmitContext* tc = (TransmitContext*) cAck->userContext;
    printf(">> %i for CAN Message with timestamp=%llu\n", cAck->status, cAck->timestamp);
}

void ReceiveMessage(void* context, ib_Can_Controller* controller, ib_Can_Message* message)
{
    TransmitContext* txContext = (TransmitContext*)(context);
    unsigned int i;
    printf(">> CAN Message: canId=%i timestamp=%llu ",
        message->canFrame->id, message->timestamp);
    if (txContext != NULL)
    {
        printf("transmitContext=%d ", txContext->someInt);
    }

    printf(": ");

    for (i = 0; i < message->canFrame->data.size; i++)
    {
        char ch = message->canFrame->data.data[i];
        if (isalnum(ch))
        {
            printf("%c", ch);
        }
        else
        {
            printf("<%x>", message->canFrame->data.data[i]);
        }
    }
    printf("\n");
}

void SendCanMessage()
{

    ib_Can_Frame msg;
    msg.id = 17;
    msg.flags = ib_Can_FrameFlag_brs;

    static int msgId = 0;
    char payload[64];
    canMessageCounter += 1;
    int payloadSize = snprintf(payload, sizeof(payload), "CAN %i", canMessageCounter);

    msg.data.data = &payload[0];
    msg.data.size = payloadSize;
    msg.dlc = payloadSize;

    transmitContext.someInt = 1234;
    ib_Can_Controller_SendFrame(canController, &msg, (void*)&transmitContext);
    printf("CAN Message sent with transmitId=%i\n", transmitContext.someInt);
}

void SimTask(void* context, ib_SimulationParticipant* participant, ib_NanosecondsTime now)
{
    SimTaskContext* tc = (SimTaskContext*)context;
    printf(">> Simulation task now=%llu with context=%i\n", now, tc->someInt);

    SendCanMessage();
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
    returnCode = ib_SimulationParticipant_Create(&participant, jsonString, participantName, domainId);
    if (returnCode) {
        printf("%s\n", ib_GetLastErrorString());
        return 2;
    }
    printf("Creating Participant %s for simulation '%s'\n", participantName, domainId);

    participantHandlerContext.someInt = 123;
    ib_SimulationParticipant_SetInitHandler(participant, (void*)&participantHandlerContext, &InitCallback);
    ib_SimulationParticipant_SetStopHandler(participant, (void*)&participantHandlerContext, &StopCallback);
    ib_SimulationParticipant_SetShutdownHandler(participant, (void*)&participantHandlerContext, &ShutdownCallback);

    const char* canNetworkName = "CAN1";
    const char* canControllerName = "CAN1";
    returnCode = ib_Can_Controller_Create(&canController, participant, canControllerName, canNetworkName);

    const char* canController2Name = "CAN2";
    returnCode = ib_Can_Controller_Create(&canController2, participant, canController2Name, canNetworkName);

    ib_Can_Controller_RegisterTransmitStatusHandler(
        canController, (void*)&transmitContext, &AckCallback,
        ib_Can_TransmitStatus_Transmitted | ib_Can_TransmitStatus_Canceled | ib_Can_TransmitStatus_TransmitQueueFull);
    ib_Can_Controller_RegisterReceiveMessageHandler(canController2, (void*)&transmitContext, &ReceiveMessage,
                                                    ib_Direction_SendReceive);
    simTaskContext.someInt = 456;
    ib_SimulationParticipant_SetPeriod(participant, 1000000);
    ib_SimulationParticipant_SetSimulationTask(participant, (void*)&simTaskContext, &SimTask);

    // Non-Blocking variant 
    ib_SimulationParticipant_RunAsync(participant);
    ib_ParticipantState outFinalParticipantState;
    ib_SimulationParticipant_WaitForRunAsyncToComplete(participant, &outFinalParticipantState);

    // Blocking variant 
    // ib_ParticipantState outFinalParticipantState;
    //ib_ParticipantState finalState = ib_SimulationParticipant_Run(participant, &outFinalParticipantState);

    ib_SimulationParticipant_Destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }

    return EXIT_SUCCESS;
}
