/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4100 5105 4204)
#include "windows.h"
#   define SleepMs(X) Sleep(X)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <unistd.h>
#define SleepMs(X) usleep((X)*1000)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ib/capi/IntegrationBus.h"

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

typedef struct
{
    uint32_t someInt;
} TransmitContext;
TransmitContext transmitContext;

ib_SimulationParticipant* participant;
ib_Data_Publisher* dataPublisher1;
ib_Data_Publisher* dataPublisher2;
ib_Data_Subscriber* dataSubscriber;

uint8_t payload[1] = {0};

uint8_t publishCount = 0;
int receiveCount = 0;
const int numPublications = 10;

char* participantName;

void NewDataSourceHandler(void* context, ib_Data_Subscriber* cbDataSubscriber, const char* topic,
                          const ib_Data_ExchangeFormat* dataExchangeFormat, const ib_KeyValueList* labelList)
{
    printf("<< Received new data source: topic=\"%s\", mediaType=\"%s\", labels={", topic,
           dataExchangeFormat->mediaType);
    for (uint32_t i = 0; i < labelList->numLabels; i++)
    {
        printf("{%s, %s}", labelList->labels[i].key, labelList->labels[i].value);
    }
    printf("}\n");

}

void SpecificDataHandler(void* context, ib_Data_Subscriber* subscriber, const ib_ByteVector* data)
{
    receiveCount += 1;
    printf("<< [SpecificDataHandler] Data received: ");

    for (size_t i = 0; i < data->size; i++)
    {
        char ch = data->data[i];
        printf("%c", ch);
    }
    printf("\n");
}

void DefaultDataHandler(void* context, ib_Data_Subscriber* subscriber, const ib_ByteVector* data)
{
    receiveCount += 1;
    printf("<< [DefaultDataHandler] Data received: ");

    for (size_t i = 0; i < data->size; i++)
    {
        char ch = data->data[i];
        printf("%c", ch);
    }
    printf("\n");
}

void PublishMessage()
{
    publishCount += 1;
    publishCount = publishCount % 10;
    payload[0] = publishCount + '0';
    ib_ByteVector dataBlob = {payload, 1};

    ib_Data_Publisher_Publish(dataPublisher1, &dataBlob);
    ib_Data_Publisher_Publish(dataPublisher2, &dataBlob);
    printf(">> Data Message published: %i\n", publishCount);
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
    // Arguments
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

    // Participant
    ib_ReturnCode returnCode;
    returnCode = ib_SimulationParticipant_Create(&participant, jsonString, participantName, domainId, ib_False);
    if (returnCode) {
        printf("%s\n", ib_GetLastErrorString());
        return 2;
    }
    printf("Creating Participant %s for simulation '%s'\n", participantName, domainId);

    if (strcmp(participantName, "Subscriber1") == 0)
    {
        ib_Data_ExchangeFormat subDataExchangeFormat = { ib_InterfaceIdentifier_DataExchangeFormat, "text/plain" };
        
        // For subscriber labels:
        // The key must appear in the publisher's labels for communication to take place.
        // No labels at all is wildcard.
        // The label's value for a given key must match, empty value is wildcard.
        ib_KeyValueList* subLabelList;
        size_t numSubLabels = 1;
        ib_KeyValuePair subLabels[1] = { {"KeyA", "ValA"} };
        Create_Labels(&subLabelList, subLabels, numSubLabels);
        transmitContext.someInt = 1234;

        returnCode = ib_Data_Subscriber_Create(&dataSubscriber, participant, "TopicA", &subDataExchangeFormat,
                                               subLabelList, (void*)&transmitContext, &DefaultDataHandler,
                                               (void*)&transmitContext, &NewDataSourceHandler);

        // This redirects publications by dataPublisher2 (label {"KeyA", "ValA"}, {"KeyB", "ValB"}) 
        // to the SpecificDataHandler (empty value is wildcard)
        ib_KeyValueList* specificLabelList;
        numSubLabels = 2;
        ib_KeyValuePair specificLabels[2] = { {"KeyA", ""}, {"KeyB", ""} };
        Create_Labels(&specificLabelList, specificLabels, numSubLabels);
        ib_Data_Subscriber_RegisterSpecificDataHandler(dataSubscriber, &subDataExchangeFormat, specificLabelList,
                                                       (void*)&transmitContext, &SpecificDataHandler);

        if (returnCode)
        {
            printf("%s\n", ib_GetLastErrorString());
            return 2;
        }

        // Run
        while (receiveCount < numPublications * 2)
        {
            SleepMs(100);
        }
        Labels_Destroy(subLabelList);
        Labels_Destroy(specificLabelList);
    }
    else if (strcmp(participantName, "Publisher1") == 0)
    {
        ib_Data_ExchangeFormat pubDataExchangeFormat = { ib_InterfaceIdentifier_DataExchangeFormat, "text/plain" };
        uint8_t history = 1;

        ib_KeyValueList* pubLabelList1;
        size_t numPubLabels1 = 1;
        ib_KeyValuePair pubLabels1[1] = { {"KeyA", "ValA"} };
        Create_Labels(&pubLabelList1, pubLabels1, numPubLabels1);
        returnCode = ib_Data_Publisher_Create(&dataPublisher1, participant, "TopicA", &pubDataExchangeFormat,
                                             pubLabelList1, history);

        ib_KeyValueList* pubLabelList2;
        size_t numPubLabels2 = 2;
        ib_KeyValuePair pubLabels2[2] = { {"KeyA", "ValA"}, {"KeyB", "ValB"} };
        Create_Labels(&pubLabelList2, pubLabels2, numPubLabels2);

        returnCode = ib_Data_Publisher_Create(&dataPublisher2, participant, "TopicA", &pubDataExchangeFormat,
                                             pubLabelList2, history);
        if (returnCode)
        {
            printf("%s\n", ib_GetLastErrorString());
            return 2;
        }

        // Run
        for (int i = 0; i < numPublications; i++)
        {
            PublishMessage();
            SleepMs(1000);
        }
        Labels_Destroy(pubLabelList1);
        Labels_Destroy(pubLabelList2);
    }

    if (jsonString)
    {
        free(jsonString);
    }
    ib_SimulationParticipant_Destroy(participant);

    return EXIT_SUCCESS;
}

#ifndef WIN32
#pragma GCC diagnostic pop
#endif
