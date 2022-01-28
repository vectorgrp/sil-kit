/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#include "Windows.h"
#define SleepMs(X) Sleep(X)
#else
#   include "unistd.h"
#define SleepMs(X) usleep((X)*1000)
#endif

#include "ib/capi/IntegrationBus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
ib_Rpc_Client* client;
ib_Rpc_Server* server;

uint8_t callCounter = 0;

char* participantName;

void PrintByteVector(const ib_ByteVector* data)
{
    for (int i = 0; i < data->size; i++)
    {
        printf("%i", data->pointer[i]);
        if (i < data->size - 1)
        {
            printf(", ");
        }
    }
    printf("\n");
}

void CallHandler(void* context, ib_Rpc_Server* server, const ib_Rpc_CallHandle* callHandle,
                        const ib_ByteVector* argumentData)
{
    uint8_t* tmp = (uint8_t*)malloc(argumentData->size * sizeof(uint8_t));
    printf("[server] Call received: ");
    PrintByteVector(argumentData);
    for (int i = 0; i < argumentData->size; i++)
    {
        tmp[i] = argumentData->pointer[i] + (uint8_t)100;
    }

    ib_ByteVector returnData = { tmp, argumentData->size };
    ib_Rpc_Server_SubmitResult(server, callHandle, &returnData);
    free(tmp);
}

void ResultHandler(void* context, ib_Rpc_Client* client, const ib_Rpc_CallHandle* callHandle,
                       ib_Rpc_CallStatus callStatus, const ib_ByteVector* returnData)
{
    if (callStatus == ib_Rpc_CallStatus_SUCCESS)
    {
        printf("[client] Call returned: ");
        PrintByteVector(returnData);
    }
    else
    {
        printf("[client] Call failed with error code %i\n", callStatus);
    }
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
    returnCode = ib_SimulationParticipant_Create(&participant, jsonString, participantName, domainId);
    if (returnCode) {
        printf("%s\n", ib_GetLastErrorString());
        return 2;
    }
    printf("Creating Participant %s for simulation '%s'\n", participantName, domainId);

    ib_Rpc_ExchangeFormat exchangeFormat = { ib_InterfaceIdentifier_RpcExchangeFormat, "*" };
    returnCode = ib_Rpc_Server_Create(&server, participant, "TestFunc", &exchangeFormat, NULL, &CallHandler);
    returnCode = ib_Rpc_Client_Create(&client, participant, "TestFunc", &exchangeFormat, NULL, &ResultHandler);
    
    for (int i = 0; i < 10; i++) 
    {
        uint8_t buffer[3] = { callCounter,callCounter,callCounter };
        ib_ByteVector argumentData = {&buffer[0], 3};
        printf("[client] Call detached: ");
        PrintByteVector(&argumentData);
        ib_Rpc_CallHandle* callHandle;
        ib_Rpc_Client_Call(client, &callHandle, &argumentData);
        callCounter += (uint8_t)1;
        SleepMs(1000);
    }

    ib_SimulationParticipant_Destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }

    return EXIT_SUCCESS;
}
