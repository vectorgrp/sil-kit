/* Copyright (c) Vector Informatik GmbH. All rights reserved. */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

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
            if (num != length)
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
ib_Logger* logger;

char* participantName;
uint8_t canMessageCounter = 0;

typedef struct {
    uint32_t someInt;
} TransmitContext;

TransmitContext transmitContext;

void AckCallback(void* context, ib_Can_Controller* controller, struct ib_Can_TransmitAcknowledge* cAck)
{
    TransmitContext* tc = (TransmitContext*)cAck->userContext;
    char buffer[256];
    sprintf(buffer, ">> %i for CAN Message with transmitId=%i, timestamp=%"PRIu64"\n", cAck->status, tc->someInt, cAck->timestamp);
    ib_Logger_Log(logger, ib_LoggingLevel_Info, buffer);
}

void ReceiveMessage(void* context, ib_Can_Controller* controller, ib_Can_Message* message)
{
    TransmitContext* txContext = (TransmitContext*)(context);
    unsigned int i;
    char buffer[512];
    char* position = buffer;
    position += sprintf(position, ">> CAN Message: canId=%i timestamp=%"PRIu64"\n",
        message->canFrame->id, message->timestamp);
    if (txContext != NULL)
    {
        position += sprintf(position, "transmitContext=%d ", txContext->someInt);
    }

    position += sprintf(position, ": ");

    for (i = 0; i < message->canFrame->data.size; i++)
    {
        char ch = message->canFrame->data.data[i];
        if (isalnum(ch))
        {
            position += sprintf(position, "%c", ch);
        }
        else
        {
            position += sprintf(position, "<%x>", message->canFrame->data.data[i]);
        }
    }
    position += sprintf(position, "\n");
    ib_Logger_Log(logger, ib_LoggingLevel_Info, buffer);
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
    char buffer[256];
    sprintf(buffer, "<< CAN Message sent with transmitId=%i\n", transmitContext.someInt);
    ib_Logger_Log(logger, ib_LoggingLevel_Info, buffer);
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
    returnCode = ib_SimulationParticipant_Create(&participant, jsonString, participantName, domainId, ib_False);
    if (returnCode) {
        printf("%s\n", ib_GetLastErrorString());
        return 2;
    }
    printf("Creating Participant %s for simulation '%s'\n", participantName, domainId);

    const char* canNetworkName = "CAN1";

    const char* canControllerName = "CAN1";
    returnCode = ib_Can_Controller_Create(&canController, participant, canControllerName, canNetworkName);
    const char* canController2Name = "CAN2";
    returnCode = ib_Can_Controller_Create(&canController2, participant, canController2Name, canNetworkName);

    ib_Can_Controller_RegisterTransmitStatusHandler(canController, (void*)&transmitContext, &AckCallback, ib_Can_TransmitStatus_Transmitted | ib_Can_TransmitStatus_Canceled | ib_Can_TransmitStatus_TransmitQueueFull);
    ib_Can_Controller_RegisterReceiveMessageHandler(canController2, (void*)&transmitContext, &ReceiveMessage, ib_Direction_SendReceive);

    ib_SimulationParticipant_GetLogger(&logger, participant);

    for (int i = 0; i < 10; i++) {
        SendCanMessage();
        SleepMs(1000);
    }

    ib_SimulationParticipant_Destroy(participant);
    if (jsonString && jsonString != "")
    {
        free(jsonString);
    }

    return EXIT_SUCCESS;
}
