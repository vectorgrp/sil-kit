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
#pragma warning(disable : 4100 5105)
#include "windows.h"
#define SleepMs(X) Sleep(X)
#else
#include <unistd.h>
#define SleepMs(X) usleep((X)*1000)
#endif

#define UNUSED_ARG(X) (void)(X)

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

const uint64_t SilKit_NanosecondsTime_Max = ((uint64_t)-1ll);

typedef void (*Timer_Action_t)(SilKit_NanosecondsTime now);
struct Timer
{
    uint8_t            isActive;
    SilKit_NanosecondsTime timeOut;
    Timer_Action_t     action;
};
typedef struct Timer Timer;

void Timer_Set(Timer* timer, SilKit_NanosecondsTime timeOut, Timer_Action_t action)
{
    timer->isActive = 1;
    timer->timeOut = timeOut;
    timer->action = action;
}
void Timer_Clear(Timer* timer)
{
    timer->isActive = 0;
    timer->timeOut = SilKit_NanosecondsTime_Max;
    timer->action = NULL;
}
void Timer_ExecuteAction(Timer* timer, SilKit_NanosecondsTime now)
{
    if (!timer->isActive || (now < timer->timeOut))
        return;

    Timer_Action_t action = timer->action;
    Timer_Clear(timer);
    action(now);
}

typedef void (*Task_Action_t)(SilKit_NanosecondsTime now);
struct Task
{
    SilKit_NanosecondsTime delay;
    Task_Action_t      action;
};
typedef struct Task Task;

struct Schedule
{
    Timer              timer;
    SilKit_NanosecondsTime now;
    uint32_t           numTasks;
    uint32_t           nextTaskIndex;
    Task*              schedule;
};
typedef struct Schedule Schedule;

void Schedule_ScheduleNextTask(Schedule* schedule)
{
    uint32_t currentTaskIndex = schedule->nextTaskIndex++;
    if (schedule->nextTaskIndex == schedule->numTasks)
    {
        schedule->nextTaskIndex = 0;
    }
    Timer_Set(&schedule->timer, schedule->now + schedule->schedule[currentTaskIndex].delay,
              schedule->schedule[currentTaskIndex].action);
}

void Schedule_Reset(Schedule* schedule)
{
    schedule->nextTaskIndex = 0;
    Schedule_ScheduleNextTask(schedule);
}

void Schedule_ExecuteTask(Schedule* schedule, SilKit_NanosecondsTime now)
{
    schedule->now = now;
    Timer_ExecuteAction(&schedule->timer, now);
}

void Schedule_Create(Schedule** outSchedule, Task* tasks, uint32_t numTasks)
{
    Schedule* newSchedule;
    size_t scheduleSize = sizeof(Schedule) + (numTasks * sizeof(Task));
    newSchedule = (Schedule*)malloc(scheduleSize);
    if (newSchedule == NULL)
    {
        AbortOnFailedAllocation("Schedule");
        return;
    }
    newSchedule->numTasks = numTasks;
    newSchedule->nextTaskIndex = 0;
    newSchedule->now = 0;
    newSchedule->timer.action = NULL;
    newSchedule->timer.isActive = 0;
    newSchedule->timer.timeOut = 0;
    memcpy(&newSchedule->schedule, &tasks, numTasks * sizeof(Task));
    Schedule_Reset(newSchedule);
    *outSchedule = newSchedule;
}

void Schedule_Destroy(Schedule* schedule)
{
    if (schedule)
    {
        free(schedule);
    }
}

SilKit_Participant* participant;
SilKit_LinController* linController;
SilKit_LinControllerConfig* controllerConfig;
char* participantName;
Schedule* masterSchedule;
Timer slaveTimer;
SilKit_NanosecondsTime slaveNow;

void StopCallback(void* context, SilKit_LifecycleService* cbLifecycleService)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbLifecycleService);
    printf("Stopping...\n");
}
void ShutdownCallback(void* context, SilKit_LifecycleService* cbLifecycleService)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbLifecycleService);
    printf("Shutting down...\n");
}

void Master_InitCallback(void* context, SilKit_LifecycleService* cbLifecycleService)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbLifecycleService);
    printf("Initializing LinMaster\n");
    controllerConfig = (SilKit_LinControllerConfig*)malloc(sizeof(SilKit_LinControllerConfig));
    if (controllerConfig == NULL)
    {
        AbortOnFailedAllocation("SilKit_LinControllerConfig");
        return;
    }
    memset(controllerConfig, 0, sizeof(SilKit_LinControllerConfig));
    SilKit_Struct_Init(SilKit_LinControllerConfig, *controllerConfig);
    controllerConfig->controllerMode = SilKit_LinControllerMode_Master;
    controllerConfig->baudRate = 20000;
    controllerConfig->numFrameResponses = 0;

    SilKit_LinController_Init(linController, controllerConfig);
}

void Master_doAction(SilKit_NanosecondsTime now)
{

    SilKit_LinControllerStatus status;
    SilKit_LinController_Status(linController, &status);
    if (status != SilKit_LinControllerStatus_Operational)
        return;
    Schedule_ExecuteTask(masterSchedule, now);
}

void Master_SimulationStepHandler(void* context, SilKit_TimeSyncService* cbTimeSyncService, SilKit_NanosecondsTime now,
                    SilKit_NanosecondsTime duration)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbTimeSyncService);
    printf("now%" PRIu64 "ms; duration=%" PRIu64 "ms\n", now / 1000000, duration / 1000000);
    Master_doAction(now);
}

void Master_ReceiveFrameStatus(void* context, SilKit_LinController* controller, const SilKit_LinFrameStatusEvent* frameStatusEvent)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    switch (frameStatusEvent->status)
    {
    case SilKit_LinFrameStatus_LIN_RX_OK:
        break; // good case, no need to warn
    case SilKit_LinFrameStatus_LIN_TX_OK:
        break; // good case, no need to warn
    default:
        printf("WARNING: LIN transmission failed!\n");
    }

    SilKit_LinFrame* frame = frameStatusEvent->frame;
    printf(">> Lin::Frame{id=%d, cs=%d, dl=%d, d={%d %d %d %d %d %d %d %d}} status=%d\n", frame->id,
           frame->checksumModel,
           frame->dataLength, frame->data[0], frame->data[1], frame->data[2], frame->data[3], frame->data[4],
           frame->data[5], frame->data[6], frame->data[7], frameStatusEvent->status);

    Schedule_ScheduleNextTask(masterSchedule);
}

void Master_WakeupHandler(void* context, SilKit_LinController* controller, const SilKit_LinWakeupEvent* wakeUpEvent)
{
    UNUSED_ARG(context);

    SilKit_LinControllerStatus status;
    SilKit_LinController_Status(controller, &status);

    if ( status != SilKit_LinControllerStatus_Sleep)
    {
        printf("WARNING: Received Wakeup pulse while SilKit_LinControllerStatus is %d.\n", status);

    }
    printf(">> Wakeup pulse received @%" PRIu64 "ms; direction=%d\n", wakeUpEvent->timestamp / 1000000,
           wakeUpEvent->direction);
    SilKit_LinController_WakeupInternal(controller);
    Schedule_ScheduleNextTask(masterSchedule);
}

void Master_LinSlaveConfigurationHandler(void* context, SilKit_LinController* controller,
                                         const SilKit_LinSlaveConfigurationEvent* linSlaveConfiguratioEvent)
{
    UNUSED_ARG(context);
    UNUSED_ARG(linSlaveConfiguratioEvent);

    SilKit_LinSlaveConfiguration* linSlaveConfiguration;
    SilKit_LinController_GetSlaveConfiguration(controller, &linSlaveConfiguration);

    printf("Received a new SilKit_LinSlaveConfiguration{respondingLinIds=[");
    if (linSlaveConfiguration->numRespondingLinIds > 0)
    {
        printf("%d", linSlaveConfiguration->respondingLinIds[0]);
        for (size_t i = 1; i < linSlaveConfiguration->numRespondingLinIds; i++)
        {
            printf(", %d", linSlaveConfiguration->respondingLinIds[i]);
        }
    }
    printf("]}\n");
}

void Master_SendFrame_16(SilKit_NanosecondsTime now)
{
    UNUSED_ARG(now);
    SilKit_LinFrame frame;
    SilKit_Struct_Init(SilKit_LinFrame, frame);
    frame.id = 16;
    frame.checksumModel = SilKit_LinChecksumModel_Classic;
    frame.dataLength = 6;
    uint8_t tmp[8] = {1, 6, 1, 6, 1, 6, 1, 6};
    memcpy(frame.data, tmp, sizeof(tmp));

    SilKit_LinController_SendFrame(linController, &frame, SilKit_LinFrameResponseType_MasterResponse);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_17(SilKit_NanosecondsTime now)
{
    UNUSED_ARG(now);
    SilKit_LinFrame frame;
    SilKit_Struct_Init(SilKit_LinFrame, frame);
    frame.id = 17;
    frame.checksumModel = SilKit_LinChecksumModel_Classic;
    frame.dataLength = 6;
    uint8_t tmp[8] = {1, 7, 1, 7, 1, 7, 1, 7};
    memcpy(frame.data, tmp, sizeof(tmp));

    SilKit_LinController_SendFrame(linController, &frame, SilKit_LinFrameResponseType_MasterResponse);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_18(SilKit_NanosecondsTime now)
{
    UNUSED_ARG(now);
    SilKit_LinFrame frame;
    SilKit_Struct_Init(SilKit_LinFrame, frame);
    frame.id = 18;
    frame.checksumModel = SilKit_LinChecksumModel_Enhanced;
    frame.dataLength = 8;
    uint8_t tmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(frame.data, tmp, sizeof(tmp));

    SilKit_LinController_SendFrame(linController, &frame, SilKit_LinFrameResponseType_MasterResponse);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_19(SilKit_NanosecondsTime now)
{
    UNUSED_ARG(now);
    SilKit_LinFrame frame;
    SilKit_Struct_Init(SilKit_LinFrame, frame);
    frame.id = 19;
    frame.checksumModel = SilKit_LinChecksumModel_Classic;
    frame.dataLength = 8;
    uint8_t tmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(frame.data, tmp, sizeof(tmp));

    SilKit_LinController_SendFrame(linController, &frame, SilKit_LinFrameResponseType_MasterResponse);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_34(SilKit_NanosecondsTime now)
{
    UNUSED_ARG(now);
    SilKit_LinFrame frame;
    SilKit_Struct_Init(SilKit_LinFrame, frame);
    frame.id = 34;
    frame.checksumModel = SilKit_LinChecksumModel_Enhanced;
    frame.dataLength = 6;

    SilKit_LinController_SendFrame(linController, &frame, SilKit_LinFrameResponseType_SlaveResponse);
    printf("<< LIN Frame Header sent for ID=%d\n", frame.id);
}

void Master_GoToSleep(SilKit_NanosecondsTime now)
{
    UNUSED_ARG(now);
    printf("<< Sending Go-To-Sleep Command and entering sleep state\n");
    SilKit_LinController_GoToSleep(linController);
}

void Slave_InitCallback(void* context, SilKit_LifecycleService* cbLifecycleService)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbLifecycleService);

    printf("Initializing LinSlave\n");

    // Configure LIN Controller to receive a LinFrameResponse for LIN ID 16
    SilKit_LinFrameResponse response_16;
    SilKit_LinFrame frame16;
    SilKit_Struct_Init(SilKit_LinFrameResponse, response_16);
    SilKit_Struct_Init(SilKit_LinFrame, frame16);

    frame16.id = 16;
    frame16.checksumModel = SilKit_LinChecksumModel_Classic;
    frame16.dataLength = 6;
    response_16.frame = &frame16;
    response_16.responseMode = SilKit_LinFrameResponseMode_Rx;

    // Configure LIN Controller to receive a LinFrameResponse for LIN ID 17
    //  - This SilKit_LinFrameResponseMode_Unused causes the controller to ignore
    //    this message and not trigger a callback. This is also the default.
    SilKit_LinFrameResponse response_17;
    SilKit_LinFrame frame17;
    SilKit_Struct_Init(SilKit_LinFrameResponse, response_17);
    SilKit_Struct_Init(SilKit_LinFrame, frame17);

    frame17.id = 17;
    frame17.checksumModel = SilKit_LinChecksumModel_Classic;
    frame17.dataLength = 6;
    response_17.frame = &frame17;
    response_17.responseMode = SilKit_LinFrameResponseMode_Unused;

    // Configure LIN Controller to receive LIN ID 18
    //  - LinChecksumModel does not match with master --> Receive with LIN_RX_ERROR
    SilKit_LinFrameResponse response_18;
    SilKit_LinFrame frame18;
    SilKit_Struct_Init(SilKit_LinFrameResponse, response_18);
    SilKit_Struct_Init(SilKit_LinFrame, frame18);

    frame18.id = 18;
    frame18.checksumModel = SilKit_LinChecksumModel_Classic;
    frame18.dataLength = 8;
    response_18.frame = &frame18;
    response_18.responseMode = SilKit_LinFrameResponseMode_Rx;

    // Configure LIN Controller to receive LIN ID 19
    //  - dataLength does not match with master --> Receive with LIN_RX_ERROR
    SilKit_LinFrameResponse response_19;
    SilKit_LinFrame frame19;
    SilKit_Struct_Init(SilKit_LinFrameResponse, response_19);
    SilKit_Struct_Init(SilKit_LinFrame, frame19);

    frame19.id = 19;
    frame19.checksumModel = SilKit_LinChecksumModel_Enhanced;
    frame19.dataLength = 1;
    response_19.frame = &frame19;
    response_19.responseMode = SilKit_LinFrameResponseMode_Rx;

    // Configure LIN Controller to send a LinFrameResponse for LIN ID 34
    SilKit_LinFrameResponse response_34;
    SilKit_LinFrame frame34;
    SilKit_Struct_Init(SilKit_LinFrameResponse, response_34);
    SilKit_Struct_Init(SilKit_LinFrame, frame34);

    frame34.id = 34;
    frame34.checksumModel = SilKit_LinChecksumModel_Enhanced;
    frame34.dataLength = 6;
    uint8_t tmp[8] = {3, 4, 3, 4, 3, 4, 3, 4};
    memcpy(frame34.data, tmp, sizeof(tmp));
    response_34.frame = &frame34;
    response_34.responseMode = SilKit_LinFrameResponseMode_TxUnconditional;

    const uint32_t numFrameResponses = 5;
    size_t   controllerConfigSize = sizeof(SilKit_LinControllerConfig);
    controllerConfig = (SilKit_LinControllerConfig*)malloc(controllerConfigSize);
    if (controllerConfig == NULL)
    {
        AbortOnFailedAllocation("Schedule");
        return;
    }
    SilKit_Struct_Init(SilKit_LinControllerConfig, *controllerConfig);
    controllerConfig->controllerMode = SilKit_LinControllerMode_Slave;
    controllerConfig->baudRate = 20000;
    controllerConfig->numFrameResponses = numFrameResponses;
    controllerConfig->frameResponses = (SilKit_LinFrameResponse*)malloc(numFrameResponses * sizeof(SilKit_LinFrameResponse));
    if (controllerConfig->frameResponses == NULL)
    {
        AbortOnFailedAllocation("SilKit_LinFrameResponse");
        return;
    }
    controllerConfig->frameResponses[0] = response_16;
    controllerConfig->frameResponses[1] = response_17;
    controllerConfig->frameResponses[2] = response_18;
    controllerConfig->frameResponses[3] = response_19;
    controllerConfig->frameResponses[4] = response_34;
    SilKit_LinController_Init(linController, controllerConfig);
}

void Slave_DoAction(SilKit_NanosecondsTime now)
{
    slaveNow = now;
    Timer_ExecuteAction(&slaveTimer, now);
}

void Slave_SimulationStepHandler(void* context, SilKit_TimeSyncService* cbTimeSyncService, SilKit_NanosecondsTime now,
                   SilKit_NanosecondsTime duration)
{
    UNUSED_ARG(context);
    UNUSED_ARG(cbTimeSyncService);

    printf("now=%" PRIu64 "ms; duration=%" PRIu64 "ms\n", now / 1000000, duration / 1000000);

    Slave_DoAction(now);
    SleepMs(500);
}

void Slave_UpdateTxBuffer_LinId34()
{
    SilKit_LinFrame frame34;
    SilKit_Struct_Init(SilKit_LinFrame, frame34);
    frame34.id = 34;
    frame34.checksumModel = SilKit_LinChecksumModel_Enhanced;
    frame34.dataLength = 6;
    uint8_t tmp[8] = {rand() % 10, 0, 0, 0, 0, 0, 0, 0};
    memcpy(frame34.data, tmp, sizeof(tmp));
    SilKit_LinController_UpdateTxBuffer(linController, &frame34);
}

void Slave_FrameStatusHandler(void* context, SilKit_LinController* controller, const SilKit_LinFrameStatusEvent* frameStatusEvent)
{
    UNUSED_ARG(context);
    UNUSED_ARG(controller);

    // On a TX acknowledge for ID 34, update the TxBuffer for the next transmission
    if (frameStatusEvent->frame->id == 34)
    {
        Slave_UpdateTxBuffer_LinId34();
    }

    SilKit_LinFrame* frame = frameStatusEvent->frame;
    printf(">> Lin::Frame{id=%d, cs=%d, dl=%d, d={%d %d %d %d %d %d %d %d}} status=%d timestamp=%"PRIu64"ms\n", frame->id,
           frame->checksumModel, frame->dataLength, frame->data[0], frame->data[1], frame->data[2], frame->data[3],
           frame->data[4], frame->data[5], frame->data[6], frame->data[7], frameStatusEvent->status, frameStatusEvent->timestamp/1000000);
}

void Slave_WakeupPulse(SilKit_NanosecondsTime now) 
{
    printf("<< Wakeup pulse @%"PRIu64"ms\n", now/1000000);
    SilKit_LinController_Wakeup(linController);
}

void Slave_GoToSleepHandler(void* context, SilKit_LinController* controller, const SilKit_LinGoToSleepEvent* goToSleepEvent)
{
    UNUSED_ARG(context);
    printf("LIN Slave received go-to-sleep command @%" PRIu64 "ms; entering sleep mode.\n",
           goToSleepEvent->timestamp / 1000000);
    // wakeup in 10 ms
    Timer_Set(&slaveTimer, slaveNow + 10000000, &Slave_WakeupPulse);
    SilKit_LinController_GoToSleepInternal(controller);
}

void Slave_WakeupHandler(void* context, SilKit_LinController* controller, const SilKit_LinWakeupEvent* wakeUpEvent)
{
    UNUSED_ARG(context);
    printf(">> LIN Slave received wakeup pulse @%" PRIu64 "ms; direction=%d; entering normal operation mode.\n",
           wakeUpEvent->timestamp / 1000000, wakeUpEvent->direction);

    if (wakeUpEvent->direction == SilKit_Direction_Receive)
    {
        SilKit_LinController_WakeupInternal(controller);
    }
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("usage: SilKitDemoCLin <ConfigJsonFile> <ParticipantName> [RegistryUri]\n");
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
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    returnCode = SilKit_ParticipantConfiguration_Destroy(participantConfiguration);
    if (returnCode) {
        printf("%s\n", SilKit_GetLastErrorString());
        return 2;
    }

    SilKit_LifecycleConfiguration startConfig;
    SilKit_Struct_Init(SilKit_LifecycleConfiguration, startConfig);
    startConfig.operationMode = SilKit_OperationMode_Coordinated;

    SilKit_LifecycleService* lifecycleService;
    returnCode = SilKit_LifecycleService_Create(&lifecycleService, participant, &startConfig);

    SilKit_TimeSyncService* timeSyncService;
    returnCode = SilKit_TimeSyncService_Create(&timeSyncService, lifecycleService);

    const char* controllerName = "LIN1";
    const char* networkName = "LIN1";
    returnCode = SilKit_LinController_Create(&linController, participant, controllerName, networkName);

    SilKit_LifecycleService_SetStopHandler(lifecycleService, NULL, &StopCallback);
    SilKit_LifecycleService_SetShutdownHandler(lifecycleService, NULL, &ShutdownCallback);
    
    if (strcmp(participantName, "LinMaster") == 0)
    {
        Task tasks[6] = {{0, &Master_SendFrame_16}, {0, &Master_SendFrame_17}, {0, &Master_SendFrame_18},
                         {0, &Master_SendFrame_19}, {0, &Master_SendFrame_34}, {5000000, &Master_GoToSleep}};
        Schedule_Create(&masterSchedule, tasks, 6);

        SilKit_LifecycleService_SetCommunicationReadyHandler(lifecycleService, NULL, &Master_InitCallback);
        SilKit_HandlerId frameStatusHandlerId;
        SilKit_LinController_AddFrameStatusHandler(linController, NULL, &Master_ReceiveFrameStatus, &frameStatusHandlerId);
        SilKit_HandlerId wakeupHandlerId;
        SilKit_LinController_AddWakeupHandler(linController, NULL, &Master_WakeupHandler, &wakeupHandlerId);
        SilKit_HandlerId linSlaveConfigurationHandlerId;
        SilKit_LinController_AddLinSlaveConfigurationHandler(linController, NULL, &Master_LinSlaveConfigurationHandler,
                                                             &linSlaveConfigurationHandlerId);

        SilKit_TimeSyncService_SetSimulationStepHandler(timeSyncService, NULL, &Master_SimulationStepHandler, 1000000);
    }
    else if (strcmp(participantName, "LinSlave") == 0)
    {
        SilKit_LifecycleService_SetCommunicationReadyHandler(lifecycleService, NULL, &Slave_InitCallback);
        SilKit_HandlerId frameStatusHandlerId;
        SilKit_LinController_AddFrameStatusHandler(linController, NULL, &Slave_FrameStatusHandler, &frameStatusHandlerId);
        SilKit_HandlerId goToSleepHandlerId;
        SilKit_LinController_AddGoToSleepHandler(linController, NULL, &Slave_GoToSleepHandler, &goToSleepHandlerId);
        SilKit_HandlerId wakeupHandlerId;
        SilKit_LinController_AddWakeupHandler(linController, NULL, &Slave_WakeupHandler, &wakeupHandlerId);
        SilKit_TimeSyncService_SetSimulationStepHandler(timeSyncService, NULL, &Slave_SimulationStepHandler, 1000000);
    }
    else
    {
        printf("Wrong participant name provided. Use either \"LinMaster\" or \"LinSlave\".\n");
        return 1;
    }

    SilKit_ParticipantState outFinalParticipantState;
    returnCode = SilKit_LifecycleService_StartLifecycle(lifecycleService);
    if(returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("Error: SilKit_LifecycleService_StartLifecycle failed: %s\n", SilKit_GetLastErrorString());
        exit(1);
    }

    returnCode = SilKit_LifecycleService_WaitForLifecycleToComplete(lifecycleService, &outFinalParticipantState);
    if(returnCode != SilKit_ReturnCode_SUCCESS)
    {
        printf("Error: SilKit_LifecycleService_WaitForLifecycleToComplete failed: %s\n", SilKit_GetLastErrorString());
        exit(1);
    }

    printf("Simulation stopped. Final State:%d\n", outFinalParticipantState);

    SilKit_Participant_Destroy(participant);
    if (jsonString)
    {
        free(jsonString);
    }
    if (controllerConfig)
    {
        free(controllerConfig);
    }
    Schedule_Destroy(masterSchedule);

    return EXIT_SUCCESS;
}
