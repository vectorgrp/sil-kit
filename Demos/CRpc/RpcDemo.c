/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4100 5105 4204)
#include "Windows.h"
#define SleepMs(X) Sleep(X)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <unistd.h>
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

int receiveCallCount = 0;
const int numCalls = 10;

char* participantName;

uint8_t buffer[3];

void PrintByteVector(const ib_ByteVector* data)
{
    for (size_t i = 0; i < data->size; i++)
    {
        printf("%i", data->data[i]);
        if (i < data->size - 1)
        {
            printf(", ");
        }
    }
    printf("\n");
}

void CallHandler(void* context, ib_Rpc_Server* cbServer, ib_Rpc_CallHandle* callHandle,
                 const ib_ByteVector* argumentData)
{
    receiveCallCount += 1;
    uint8_t* tmp = (uint8_t*)malloc(argumentData->size * sizeof(uint8_t));
    printf("[Server] Call received: ");
    PrintByteVector(argumentData);
    for (size_t i = 0; i < argumentData->size; i++)
    {
        tmp[i] = argumentData->data[i] + (uint8_t)100;
    }

    const ib_ByteVector returnData = { tmp, argumentData->size };
    ib_Rpc_Server_SubmitResult(cbServer, callHandle, &returnData);
    free(tmp);
}

void ResultHandler(void* context, ib_Rpc_Client* cbClient, ib_Rpc_CallHandle* callHandle,
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

void DiscoveryResultHandler(void* context, const ib_Rpc_DiscoveryResultList* discoveryResults)
{
    for (uint32_t i = 0; i < discoveryResults->numResults; i++)
    {
        printf("Discovered RpcServer with functionName=\"%s\", exchangeFormat.mediaType=\"%s\", labels={",
               discoveryResults->results[i].functionName, discoveryResults->results[i].exchangeFormat->mediaType);
        for (uint32_t j = 0; j < discoveryResults->results[i].labelList->numLabels; j++)
        {
            printf("{\"%s\", \"%s\"}", discoveryResults->results[i].labelList->labels[j].key, discoveryResults->results[i].labelList->labels[j].value);
        }
        printf("}\n");
    }
}

void Copy_Label(ib_KeyValuePair* dst, const ib_KeyValuePair* src)
{
    dst->key = malloc(strlen(src->key) + 1);
    dst->value = malloc(strlen(src->value) + 1);
    if (dst->key != NULL && dst->value != NULL)
    {
        strcpy((char*)dst->key, src->key);
        strcpy((char*)dst->value, src->value);
    }
}

void Create_Labels(ib_KeyValueList** outLabelList, const ib_KeyValuePair* labels, size_t numLabels)
{
    ib_KeyValueList* newLabelList;
    size_t labelsSize = numLabels * sizeof(ib_KeyValuePair);
    size_t labelListSize = sizeof(ib_KeyValueList) + labelsSize;
    newLabelList = (ib_KeyValueList*)malloc(labelListSize);
    if (newLabelList != NULL)
    {
        newLabelList->numLabels = numLabels;
        for (size_t i = 0; i < numLabels; i++)
        {
            Copy_Label(&newLabelList->labels[i], &labels[i]);
        }
    }
    *outLabelList = newLabelList;
}

void Labels_Destroy(ib_KeyValueList* labelList)
{
    if (labelList)
    {
        for (size_t i = 0; i < labelList->numLabels; i++)
        {
            free((char*)labelList->labels[i].key);
            free((char*)labelList->labels[i].value);
        }
        free(labelList);
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
    returnCode = ib_SimulationParticipant_Create(&participant, jsonString, participantName, domainId, ib_False);
    if (returnCode) {
        printf("%s\n", ib_GetLastErrorString());
        return 2;
    }
    printf("Creating Participant %s for simulation '%s'\n", participantName, domainId);

    if (strcmp(participantName, "Client") == 0)
    {
        const char* filterFunctionName = "";
        ib_Rpc_ExchangeFormat filterExchangeFormat = {ib_InterfaceIdentifier_RpcExchangeFormat, ""};
        ib_KeyValueList* filterLabelList;
        size_t numLabels = 1;
        ib_KeyValuePair filterLabels[1] = {{"KeyA", "ValA"}};
        Create_Labels(&filterLabelList, filterLabels, numLabels);

        returnCode = ib_Rpc_DiscoverServers(participant, filterFunctionName, &filterExchangeFormat, filterLabelList, NULL, &DiscoveryResultHandler);

        ib_Rpc_ExchangeFormat exchangeFormat = { ib_InterfaceIdentifier_RpcExchangeFormat, "A" };
        ib_KeyValueList* labelList;
        numLabels = 1;
        ib_KeyValuePair labels[1] = { {"KeyA", "ValA"} };
        Create_Labels(&labelList, labels, numLabels);

        returnCode = ib_Rpc_Client_Create(&client, participant, "TestFunc", &exchangeFormat, labelList, NULL, &ResultHandler);

        for (uint8_t i = 0; i < numCalls; i++)
        {
            SleepMs(1000);
            buffer[0] = i;
            buffer[1] = i;
            buffer[2] = i;
            ib_ByteVector argumentData = { &buffer[0], 3 };
            printf("[Client] Call detached: ");
            PrintByteVector(&argumentData);
            ib_Rpc_CallHandle* callHandle;
            ib_Rpc_Client_Call(client, &callHandle, &argumentData);
        }
    }
    else if (strcmp(participantName, "Server") == 0)
    {
        ib_Rpc_ExchangeFormat exchangeFormat = {ib_InterfaceIdentifier_RpcExchangeFormat, "A"};
        ib_KeyValueList* labelList;
        size_t numLabels = 2;
        ib_KeyValuePair labels[2] = {{"KeyA", "ValA"}, {"KeyB", "ValB"}};
        Create_Labels(&labelList, labels, numLabels);

        returnCode = ib_Rpc_Server_Create(&server, participant, "TestFunc", &exchangeFormat, labelList, NULL, &CallHandler);

        while (receiveCallCount < numCalls)
        {
            SleepMs(100);
        }
    }

    ib_SimulationParticipant_Destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }

    return EXIT_SUCCESS;
}

#ifndef WIN32
#pragma GCC diagnostic pop
#endif
