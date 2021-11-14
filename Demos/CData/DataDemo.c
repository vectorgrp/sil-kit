/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
ib_DataPublisher* dataPublisher;
ib_DataSubscriber* dataSubscriber;

int dataMessageCounter = 0;

char* participantName;

void ReceiveMessage(void* context, ib_DataSubscriber* subscriber, const ib_ByteVector* data)
{
    printf(">> Data received: ");

    for (int i = 0; i < data->size; i++)
    {
        char ch = data->pointer[i];
        if (isalnum(ch))
        {
            printf("%c", ch);
        }
        else
        {
            printf("<%x>", data->pointer[i]);
        }
    }
    printf("\n");
}

void PublishMessage()
{
    char payload[64];
    dataMessageCounter += 1;
    int payloadSize = snprintf(payload, sizeof(payload), "Data Message %i", dataMessageCounter);

    ib_ByteVector dataBlob = {(const uint8_t* const)&payload[0], payloadSize};

    ib_DataPublisher_Publish(dataPublisher, &dataBlob);
    printf("Data Message published: %i\n", dataMessageCounter);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("usage: IbDemoCData <ConfigJsonFile> <ParticipantName> [<DomainId>]\n");
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
    returnCode = ib_SimulationParticipant_create(&participant, jsonString, participantName, domainId);
    if (returnCode) {
        printf("%s\n", ib_GetLastErrorString());
        return 2;
    }
    printf("Creating Participant %s for simulation '%s'\n", participantName, domainId);

    ib_DataExchangeFormat dataExchangeFormat = { ib_InterfaceIdentifier_DataExchangeFormat, "" };
    returnCode = ib_DataPublisher_create(&dataPublisher, participant, "testTopic", &dataExchangeFormat, 0);
    returnCode = ib_DataSubscriber_create(&dataSubscriber, participant, "testTopic", &dataExchangeFormat, NULL, &ReceiveMessage);

    for (int i = 0; i < 10; i++) {
        PublishMessage();
        SleepMs(1000);
    }

    ib_SimulationParticipant_destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }

    return EXIT_SUCCESS;
}
