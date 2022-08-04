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


#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 5105 4204)
#include "windows.h"
#   define SleepMs(X) Sleep(X)
#else
#include <unistd.h>
#define SleepMs(X) usleep((X)*1000)
#endif
#define UNUSED_ARG(X) (void)(X)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "silkit/capi/SilKit.h"

void AbortOnFailedAllocation(const char* failedAllocStrucName)
{
    fprintf(stderr, "Error: Allocation of \"%s\" failed, aborting...", failedAllocStrucName);
    abort();
}

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

SilKit_Participant* participant;
SilKit_DataPublisher* dataPublisher1;
SilKit_DataPublisher* dataPublisher2;
SilKit_DataSubscriber* dataSubscriber;

uint8_t payload[1] = {0};

uint8_t publishCount = 0;
int receiveCount = 0;
const int numPublications = 30;

char* participantName;

void SilKitCALL DefaultDataHandler(void* context, SilKit_DataSubscriber* subscriber, const SilKit_DataMessageEvent* dataMessageEvent)
{
    UNUSED_ARG(context);
    UNUSED_ARG(subscriber);

    receiveCount += 1;
    printf("<< [DefaultDataHandler] Data received: ");

    for (size_t i = 0; i < dataMessageEvent->data.size; i++)
    {
        char ch = dataMessageEvent->data.data[i];
        printf("%c", ch);
    }
    printf("\n");
}

void PublishMessage()
{
    publishCount += 1;
    publishCount = publishCount % 10;
    payload[0] = publishCount + '0';
    SilKit_ByteVector dataBlob = {payload, 1};

    SilKit_DataPublisher_Publish(dataPublisher1, &dataBlob);
    SilKit_DataPublisher_Publish(dataPublisher2, &dataBlob);
    printf(">> Data Message published: %i\n", publishCount);
}

void Copy_Label(SilKit_Label* dst, const SilKit_Label* src)
{
    dst->key = malloc(strlen(src->key) + 1);
    dst->value = malloc(strlen(src->value) + 1);
    if (dst->key == NULL || dst->value == NULL)
    {
        AbortOnFailedAllocation("SilKit_KeyValuePair");
        return;
    }
    strcpy((char*)dst->key, src->key);
    strcpy((char*)dst->value, src->value);
    dst->kind = src->kind;
}

void Create_Labels(SilKit_LabelList** outLabelList, const SilKit_Label* labels, size_t numLabels)
{
    SilKit_LabelList* newLabelList;
    newLabelList = (SilKit_LabelList*)malloc(sizeof(SilKit_LabelList));
    if (newLabelList == NULL)
    {
        AbortOnFailedAllocation("SilKit_LabelList");
        return;
    }
    newLabelList->numLabels = numLabels;
    newLabelList->labels = (SilKit_Label*)malloc(numLabels * sizeof(SilKit_Label));
    if (newLabelList->labels == NULL)
    {
        AbortOnFailedAllocation("SilKit_Label");
        return;
    }
    for (size_t i = 0; i < numLabels; i++)
    {
        Copy_Label(&newLabelList->labels[i], &labels[i]);
    }
    *outLabelList = newLabelList;
}

void Labels_Destroy(SilKit_LabelList* labelList)
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
        printf("usage: SilKitDemoCData <ConfigJsonFile> <ParticipantName> [RegistryUri]\n");
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

    // Participant
    SilKit_ReturnCode returnCode;

    SilKit_ParticipantConfiguration* participantConfiguration = NULL;
    returnCode = SilKit_ParticipantConfiguration_FromString(&participantConfiguration, jsonString);
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    returnCode = SilKit_Participant_Create(&participant, participantConfiguration, participantName, registryUri);
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfiguration);
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    if (strcmp(participantName, "Subscriber1") == 0)
    {
        const char* subMediaType = "text/plain";

        // For subscriber labels:
        // The key must appear in the publisher's labels for communication to take place.
        // No labels at all is wildcard.
        // The label's value for a given key must match, empty value is wildcard.
        SilKit_LabelList* subLabelList;
        size_t numSubLabels = 1;
        SilKit_Label subLabels[1] = {{"KeyA", "ValA", SilKit_LabelKind_Preferred}};
        Create_Labels(&subLabelList, subLabels, numSubLabels);

        SilKit_DataSpec dataSpec;
        SilKit_Struct_Init(SilKit_DataSpec, dataSpec);
        dataSpec.topic = "TopicA";
        dataSpec.mediaType = subMediaType;
        dataSpec.labelList.numLabels = subLabelList->numLabels;
        dataSpec.labelList.labels = subLabelList->labels;

        transmitContext.someInt = 1234;

        returnCode = SilKit_DataSubscriber_Create(&dataSubscriber, participant, "SubCtrl1", &dataSpec,
                                                  (void*)&transmitContext, &DefaultDataHandler);

        if (returnCode)
        {
            printf("%s\n", SilKit_GetLastErrorString());
            return 2;
        }

        // Run
        while (receiveCount < numPublications * 2)
        {
            SleepMs(100);
        }
        Labels_Destroy(subLabelList);
    }
    else if (strcmp(participantName, "Publisher1") == 0)
    {
        const char * pubMediaType = "text/plain";
        uint8_t history = 1;

        SilKit_LabelList* pubLabelList1;
        size_t numPubLabels1 = 1;
        SilKit_Label pubLabels1[1] = { {"KeyA", "ValA", SilKit_LabelKind_Mandatory} };
        Create_Labels(&pubLabelList1, pubLabels1, numPubLabels1);

        SilKit_DataSpec dataSpec;
        SilKit_Struct_Init(SilKit_DataSpec, dataSpec);
        dataSpec.topic = "TopicA";
        dataSpec.mediaType = pubMediaType;
        dataSpec.labelList.numLabels = pubLabelList1->numLabels;
        dataSpec.labelList.labels = pubLabelList1->labels;

        returnCode = SilKit_DataPublisher_Create(&dataPublisher1, participant, "PubCtrl1", &dataSpec, history);

        SilKit_LabelList* pubLabelList2;
        size_t numPubLabels2 = 2;
        SilKit_Label pubLabels2[2] = {{"KeyA", "ValA", SilKit_LabelKind_Mandatory},
                                          {"KeyB", "ValB", SilKit_LabelKind_Mandatory}};
        Create_Labels(&pubLabelList2, pubLabels2, numPubLabels2);
        SilKit_DataSpec dataSpec2;
        SilKit_Struct_Init(SilKit_DataSpec, dataSpec2);
        dataSpec2.topic = "TopicA";
        dataSpec2.mediaType = pubMediaType;
        dataSpec2.labelList.numLabels = pubLabelList2->numLabels;
        dataSpec2.labelList.labels = pubLabelList2->labels;

        returnCode = SilKit_DataPublisher_Create(&dataPublisher2, participant, "PubCtrl2", &dataSpec2, history);
        if (returnCode)
        {
            printf("%s\n", SilKit_GetLastErrorString());
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
    else
    {
        printf("Wrong participant name provided. Use either \"Subscriber1\" or \"Publisher1\".\n");
        return 1;
    }

    if (jsonString)
    {
        free(jsonString);
    }
    SilKit_Participant_Destroy(participant);

    return EXIT_SUCCESS;
}
