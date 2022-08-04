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
#pragma warning(disable: 5105 4204)
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

void MacToBytes(uint8_t* outBytes, const char* mac)
{
    char macCopy[18];
    memcpy(macCopy, mac, sizeof(macCopy)-1);

    char* ptrMac = macCopy;
    uint8_t* ptrBytes = outBytes;
    for (int i = 0; i < 18; i++) {
        if (macCopy[i] == ':' || i==17)
        {
            macCopy[i] = '\0';
            *ptrBytes = (uint8_t)strtol(ptrMac, NULL, 16);
            ptrMac += 3;
            ptrBytes += 1;
        }
    }
}

SilKit_Participant* participant;
SilKit_EthernetController* ethernetController1;
SilKit_EthernetController* ethernetController2;

char* participantName;
uint8_t ethernetFrameCounter = 0;
uint8_t buffer[100];

#define SOURCE_MAC_SIZE 6
#define DESTINATION_MAC_SIZE 6
#define ETHERTYPE_MAC_SIZE 2
#define PAYLOAD_OFFSET (SOURCE_MAC_SIZE + DESTINATION_MAC_SIZE + ETHERTYPE_MAC_SIZE)

typedef struct  {
    uint32_t someInt;
} TransmitContext;

TransmitContext transmitContext;

void SilKitCALL FrameTransmitHandler(void* context, SilKit_EthernetController* controller,
                          struct SilKit_EthernetFrameTransmitEvent* frameTransmitEvent)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    TransmitContext* tc = (TransmitContext*)frameTransmitEvent->userContext;
    printf(">> %i for Ethernet frame with transmitId=%i, timestamp=%" PRIu64 "\n", frameTransmitEvent->status,
           tc->someInt, frameTransmitEvent->timestamp);
}

void SilKitCALL FrameHandler(void* context, SilKit_EthernetController* controller, SilKit_EthernetFrameEvent* frameEvent)
{
    UNUSED_ARG(controller);

    TransmitContext* txContext = (TransmitContext*)(context);
    unsigned int i;
    printf(">> Ethernet FrameEvent: timestamp=%" PRIu64 "\n", frameEvent->timestamp);
    if (txContext != NULL)
    {
        printf("transmitContext=%d ", txContext->someInt);
    }

    printf(": ");

    for (i = PAYLOAD_OFFSET; i < frameEvent->ethernetFrame->raw.size; i++)
    {
        char ch = frameEvent->ethernetFrame->raw.data[i];
        if (isalnum(ch))
        {
            printf("%c", ch);
        }
        else
        {
            printf("<%x>", frameEvent->ethernetFrame->raw.data[i]);
        }
    }
    printf("\n");
}

void SendFrame()
{
    // set destination mac
    uint8_t destinationMac[6] = { 0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC1 };
    memcpy(&(buffer[0]), destinationMac, sizeof(destinationMac));

    // set source mac
    uint8_t sourceMac[6] = { 0xF6, 0x04, 0x68, 0x71, 0xAA, 0xC2 };
    memcpy(&(buffer[6]), sourceMac, sizeof(sourceMac));

    // set ethertype
    buffer[12] = 0x00;
    buffer[13] = 0x08;

    // set payload
    ethernetFrameCounter += 1;
    int payloadSize = snprintf((char*)buffer + PAYLOAD_OFFSET, sizeof(buffer) - PAYLOAD_OFFSET, 
        "This is the demonstration Ethernet frame number %i.", ethernetFrameCounter);

    if (payloadSize <= 0)
    {
        fprintf(stderr, "Error: SendFrame cannot create payload. snprintf returned %d\n", payloadSize);
        exit(-2);
    }

    SilKit_EthernetFrame ef;
    SilKit_Struct_Init(SilKit_EthernetFrame, ef);
    ef.raw.data = (const uint8_t*)buffer;
    ef.raw.size = PAYLOAD_OFFSET + payloadSize;

    transmitContext.someInt = ethernetFrameCounter;
    SilKit_EthernetController_SendFrame(ethernetController1, &ef, (void*)&transmitContext);
    
    printf("Ethernet frame sent \n");
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("usage: SilKitDemoCEthernet <ConfigJsonFile> <ParticipantName> [RegistryUri]\n");
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
    if (returnCode) 
    {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfiguration);
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    returnCode = SilKit_EthernetController_Create(&ethernetController1, participant, "ETH0", "Ethernet1");
    returnCode = SilKit_EthernetController_Create(&ethernetController2, participant, "ETH1", "Ethernet1");

    returnCode = SilKit_EthernetController_Activate(ethernetController1);
    returnCode = SilKit_EthernetController_Activate(ethernetController2);

    SilKit_HandlerId frameTransmitHandlerId;
    SilKit_EthernetController_AddFrameTransmitHandler(ethernetController1, NULL, &FrameTransmitHandler,
        SilKit_EthernetTransmitStatus_Transmitted | SilKit_EthernetTransmitStatus_ControllerInactive
            | SilKit_EthernetTransmitStatus_LinkDown | SilKit_EthernetTransmitStatus_Dropped
            | SilKit_EthernetTransmitStatus_InvalidFrameFormat,
        &frameTransmitHandlerId);
    SilKit_HandlerId frameHandlerId;
    SilKit_EthernetController_AddFrameHandler(ethernetController2, NULL, &FrameHandler, SilKit_Direction_Receive,
                                              &frameHandlerId);

    for (int i = 0; i < 10; i ++) 
    {
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
