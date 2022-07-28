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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include "silkit/capi/SilKit.h"

#ifdef WIN32
#pragma warning(disable : 5105)
#include "windows.h"
#   define SleepMs(X) Sleep(X)
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

SilKit_Participant* participant;
SilKit_CanController* canController;
SilKit_CanController* canController2;
SilKit_Logger* logger;

char* participantName;
uint8_t canMessageCounter = 0;

typedef struct {
    uint32_t someInt;
} TransmitContext;

TransmitContext transmitContext;

void FrameTransmitHandler(void* context, SilKit_CanController* controller, struct SilKit_CanFrameTransmitEvent* cAck)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    TransmitContext* tc = (TransmitContext*)cAck->userContext;
    char buffer[256];
    sprintf(buffer, ">> %i for CAN Message with transmitId=%i, timestamp=%" PRIu64 "\n", cAck->status, tc->someInt,
            cAck->timestamp);
    SilKit_Logger_Log(logger, SilKit_LoggingLevel_Info, buffer);
}

void FrameHandler(void* context, SilKit_CanController* controller, SilKit_CanFrameEvent* frameEvent)
{
    UNUSED_ARG(controller);

    TransmitContext* txContext = (TransmitContext*)(context);
    unsigned int i;
    char buffer[512];
    char* position = buffer;
    position += sprintf(position, ">> CAN Message: canId=%i timestamp=%"PRIu64"\n",
        frameEvent->frame->id, frameEvent->timestamp);
    if (txContext != NULL)
    {
        position += sprintf(position, "transmitContext=%d ", txContext->someInt);
    }

    position += sprintf(position, ": ");

    for (i = 0; i < frameEvent->frame->data.size; i++)
    {
        char ch = frameEvent->frame->data.data[i];
        if (isalnum(ch))
        {
            position += sprintf(position, "%c", ch);
        }
        else
        {
            position += sprintf(position, "<%x>", frameEvent->frame->data.data[i]);
        }
    }
    position += sprintf(position, "\n");
    SilKit_Logger_Log(logger, SilKit_LoggingLevel_Info, buffer);
}

void SendFrame()
{
    SilKit_CanFrame canFrame;
    SilKit_Struct_Init(SilKit_CanFrame, canFrame);
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
    char buffer[256];
    sprintf(buffer, "<< CAN Message sent with transmitId=%i\n", transmitContext.someInt);
    SilKit_Logger_Log(logger, SilKit_LoggingLevel_Info, buffer);
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
    returnCode = SilKit_Participant_Create(&participant, jsonString, participantName, registryUri, SilKit_False);
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    const char* canNetworkName = "CAN1";

    const char* canControllerName = "CAN1";
    returnCode = SilKit_CanController_Create(&canController, participant, canControllerName, canNetworkName);
    const char* canController2Name = "CAN2";
    returnCode = SilKit_CanController_Create(&canController2, participant, canController2Name, canNetworkName);

    returnCode = SilKit_CanController_Start(canController);
    returnCode = SilKit_CanController_Start(canController2);

    SilKit_HandlerId frameTransmitHandlerId;
    SilKit_CanController_AddFrameTransmitHandler(
        canController, (void*)&transmitContext, &FrameTransmitHandler,
        SilKit_CanTransmitStatus_Transmitted | SilKit_CanTransmitStatus_Canceled | SilKit_CanTransmitStatus_TransmitQueueFull,
        &frameTransmitHandlerId);

    SilKit_HandlerId frameHandlerId;
    SilKit_CanController_AddFrameHandler(canController2, (void*)&transmitContext, &FrameHandler, SilKit_Direction_SendReceive,
                                      &frameHandlerId);

    SilKit_Participant_GetLogger(&logger, participant);

    for (int i = 0; i < 10; i++) {
        SendFrame();
        SleepMs(1000);
    }

    SilKit_Participant_Destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }

    return EXIT_SUCCESS;
}
