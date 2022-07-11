/* Copyright (c) Vector Informatik GmbH. All rights reserved. */
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

void FrameTransmitHandler(void* context, SilKit_EthernetController* controller,
                          struct SilKit_EthernetFrameTransmitEvent* frameTransmitEvent)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    TransmitContext* tc = (TransmitContext*)frameTransmitEvent->userContext;
    printf(">> %i for Ethernet frame with transmitId=%i, timestamp=%" PRIu64 "\n", frameTransmitEvent->status,
           tc->someInt, frameTransmitEvent->timestamp);
}

void FrameHandler(void* context, SilKit_EthernetController* controller, SilKit_EthernetFrameEvent* frameEvent)
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

    SilKit_EthernetFrame ef = {SilKit_InterfaceIdentifier_EthernetFrame, {(const uint8_t*)buffer, PAYLOAD_OFFSET + payloadSize}};

    transmitContext.someInt = ethernetFrameCounter;
    SilKit_EthernetController_SendFrame(ethernetController1, &ef, (void*)&transmitContext);
    
    printf("Ethernet frame sent \n");
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("usage: SilKitDemoCEthernet <ConfigJsonFile> <ParticipantName> [<DomainId>]\n");
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
    if (returnCode) 
    {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }
    printf("Creating participant '%s' for simulation '%s'\n", participantName, registryUri);


    returnCode = SilKit_EthernetController_Create(&ethernetController1, participant, "ETH0", "Ethernet1");
    returnCode = SilKit_EthernetController_Create(&ethernetController2, participant, "ETH1", "Ethernet1");

    returnCode = SilKit_EthernetController_Activate(ethernetController1);
    returnCode = SilKit_EthernetController_Activate(ethernetController2);

    SilKit_HandlerId frameTransmitHandlerId;
    SilKit_EthernetController_AddFrameTransmitHandler(ethernetController1, NULL, &FrameTransmitHandler,
                                                   &frameTransmitHandlerId);
    SilKit_HandlerId frameHandlerId;
    SilKit_EthernetController_AddFrameHandler(ethernetController2, NULL, &FrameHandler, &frameHandlerId);

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
