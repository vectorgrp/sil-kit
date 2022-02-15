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

const uint64_t ib_NanosecondsTime_Max = ((uint64_t)-1ll);

typedef void (*Timer_Action_t)(ib_NanosecondsTime now);
struct Timer
{
    uint8_t            isActive;
    ib_NanosecondsTime timeOut;
    Timer_Action_t     action;
};
typedef struct Timer Timer;

void Timer_Set(Timer* timer, ib_NanosecondsTime timeOut, Timer_Action_t action)
{
    timer->isActive = 1;
    timer->timeOut = timeOut;
    timer->action = action;
}
void Timer_Clear(Timer* timer)
{
    timer->isActive = 0;
    timer->timeOut = ib_NanosecondsTime_Max;
    timer->action = NULL;
}
void Timer_ExecuteAction(Timer* timer, ib_NanosecondsTime now)
{
    if (!timer->isActive || (now < timer->timeOut))
        return;

    Timer_Action_t action = timer->action;
    Timer_Clear(timer);
    action(now);
}

typedef void (*Task_Action_t)(ib_NanosecondsTime now);
struct Task
{
    ib_NanosecondsTime delay;
    Task_Action_t      action;
};
typedef struct Task Task;

struct Schedule
{
    Timer              timer;
    ib_NanosecondsTime now;
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

void Schedule_ExecuteTask(Schedule* schedule, ib_NanosecondsTime now)
{
    schedule->now = now;
    Timer_ExecuteAction(&schedule->timer, now);
}

void Schedule_Create(Schedule** outSchedule, Task* tasks, uint32_t numTasks)
{
    Schedule* newSchedule;
    size_t scheduleSize = sizeof(Schedule) + (numTasks * sizeof(Task));
    newSchedule = (Schedule*)malloc(scheduleSize);
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

ib_SimulationParticipant* participant;
ib_Lin_Controller*         linController;
ib_Lin_ControllerConfig*   controllerConfig;
char*                     participantName;
Schedule*                 masterSchedule;
Timer                     slaveTimer;
ib_NanosecondsTime        slaveNow;

void StopCallback(void* context, ib_SimulationParticipant* participant)
{
    printf("Stopping...\n");
}
void ShutdownCallback(void* context, ib_SimulationParticipant* participant)
{
    printf("Shutting down...\n");
}

void Master_InitCallback(void* context, ib_SimulationParticipant* participant, struct ib_ParticipantCommand* command)
{
    printf("Initializing LinMaster\n");
    controllerConfig = (ib_Lin_ControllerConfig*)malloc(sizeof(ib_Lin_ControllerConfig));
    memset(controllerConfig, 0, sizeof(ib_Lin_ControllerConfig));
    controllerConfig->interfaceId = ib_InterfaceIdentifier_LinControllerConfig;
    controllerConfig->controllerMode = ib_Lin_ControllerMode_Master;
    controllerConfig->baudRate = 20000;
    controllerConfig->numFrameResponses = 0;
    
    ib_Lin_Controller_Init(linController, controllerConfig);
}

void Master_doAction(ib_NanosecondsTime now)
{

    ib_Lin_ControllerStatus status;
    ib_Lin_Controller_Status(linController, &status);
    if (status != ib_Lin_ControllerStatus_Operational)
        return;
    Schedule_ExecuteTask(masterSchedule, now);
}

void Master_SimTask(void* context, ib_SimulationParticipant* participant, ib_NanosecondsTime now)
{
    printf("now=%llums\n", now / 1000000);
    Master_doAction(now);
}

void Master_ReceiveFrameStatus(void* context, ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
                               ib_Lin_FrameStatus status, ib_NanosecondsTime timestamp)
{
    switch (status)
    {
    case ib_Lin_FrameStatus_LIN_RX_OK:
        break; // good case, no need to warn
    case ib_Lin_FrameStatus_LIN_TX_OK:
        break; // good case, no need to warn
    default:
        printf("WARNING: LIN transmission failed!\n");
    }

    printf(">> lin::Frame{id=%d, cs=%d, dl=%d, d={%d %d %d %d %d %d %d %d}} status=%d\n", frame->id, frame->checksumModel,
           frame->dataLength, frame->data[0], frame->data[1], frame->data[2], frame->data[3], frame->data[4],
           frame->data[5], frame->data[6], frame->data[7], status);

    Schedule_ScheduleNextTask(masterSchedule);
}

void Master_WakeupHandler(void* context, ib_Lin_Controller* controller)
{
    ib_Lin_ControllerStatus status;
    ib_Lin_Controller_Status(controller, &status);

    if ( status != ib_Lin_ControllerStatus_Sleep)
    {
        printf("WARNING: Received Wakeup pulse while ControllerStatus is %d.\n", status);

    }
    printf(">> Wakeup pulse received\n");
    ib_Lin_Controller_WakeupInternal(controller);
    Schedule_ScheduleNextTask(masterSchedule);
}

void Master_SendFrame_16(ib_NanosecondsTime now)
{
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 16;
    frame.checksumModel = ib_Lin_ChecksumModel_Classic;
    frame.dataLength = 6;
    uint8_t tmp[8] = {1, 6, 1, 6, 1, 6, 1, 6};
    memcpy(frame.data, tmp, sizeof(tmp));

    ib_Lin_Controller_SendFrameWithTimestamp(linController, &frame, ib_Lin_FrameResponseType_MasterResponse, now);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_17(ib_NanosecondsTime now)
{
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 17;
    frame.checksumModel = ib_Lin_ChecksumModel_Classic;
    frame.dataLength = 6;
    uint8_t tmp[8] = {1, 7, 1, 7, 1, 7, 1, 7};
    memcpy(frame.data, tmp, sizeof(tmp));

    ib_Lin_Controller_SendFrameWithTimestamp(linController, &frame, ib_Lin_FrameResponseType_MasterResponse, now);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_18(ib_NanosecondsTime now)
{
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 18;
    frame.checksumModel = ib_Lin_ChecksumModel_Enhanced;
    frame.dataLength = 8;
    uint8_t tmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(frame.data, tmp, sizeof(tmp));

    ib_Lin_Controller_SendFrameWithTimestamp(linController, &frame, ib_Lin_FrameResponseType_MasterResponse, now);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_19(ib_NanosecondsTime now)
{
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 19;
    frame.checksumModel = ib_Lin_ChecksumModel_Classic;
    frame.dataLength = 8;
    uint8_t tmp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(frame.data, tmp, sizeof(tmp));

    ib_Lin_Controller_SendFrameWithTimestamp(linController, &frame, ib_Lin_FrameResponseType_MasterResponse, now);
    printf("<< LIN Frame sent with ID=%d\n", frame.id);
}

void Master_SendFrame_34(ib_NanosecondsTime now)
{
    ib_Lin_Frame frame;
    frame.interfaceId = ib_InterfaceIdentifier_LinFrame;
    frame.id = 34;
    frame.checksumModel = ib_Lin_ChecksumModel_Enhanced;
    frame.dataLength = 6;

    ib_Lin_Controller_SendFrameWithTimestamp(linController, &frame, ib_Lin_FrameResponseType_SlaveResponse, now);
    printf("<< LIN Frame Header sent for ID=%d\n", frame.id);
}

void Master_GoToSleep(ib_NanosecondsTime now)
{
    printf("<< Sending Go-To-Sleep Command and entering sleep state\n");
    ib_Lin_Controller_GoToSleep(linController);
}

void Slave_InitCallback(void* context, ib_SimulationParticipant* participant, struct ib_ParticipantCommand* command)
{
    printf("Initializing LinSlave\n");

    // Configure LIN Controller to receive a FrameResponse for LIN ID 16
    ib_Lin_FrameResponse response_16;
    response_16.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;
    response_16.frame.id = 16;
    response_16.frame.checksumModel = ib_Lin_ChecksumModel_Classic;
    response_16.frame.dataLength = 6;
    response_16.responseMode = ib_Lin_FrameResponseMode_Rx;

    // Configure LIN Controller to receive a FrameResponse for LIN ID 17
    //  - This ib_Lin_FrameResponseMode_Unused causes the controller to ignore
    //    this message and not trigger a callback. This is also the default.
    ib_Lin_FrameResponse response_17;
    response_17.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;
    response_17.frame.id = 17;
    response_17.frame.checksumModel = ib_Lin_ChecksumModel_Classic;
    response_17.frame.dataLength = 6;
    response_17.responseMode = ib_Lin_FrameResponseMode_Unused;

    // Configure LIN Controller to receive LIN ID 18
    //  - ChecksumModel does not match with master --> Receive with LIN_RX_ERROR
    ib_Lin_FrameResponse response_18;
    response_18.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;
    response_18.frame.id = 18;
    response_18.frame.checksumModel = ib_Lin_ChecksumModel_Classic;
    response_18.frame.dataLength = 8;
    response_18.responseMode = ib_Lin_FrameResponseMode_Rx;

    // Configure LIN Controller to receive LIN ID 19
    //  - dataLength does not match with master --> Receive with LIN_RX_ERROR
    ib_Lin_FrameResponse response_19;
    response_19.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;
    response_19.frame.id = 19;
    response_19.frame.checksumModel = ib_Lin_ChecksumModel_Enhanced;
    response_19.frame.dataLength = 1;
    response_19.responseMode = ib_Lin_FrameResponseMode_Rx;

    // Configure LIN Controller to send a FrameResponse for LIN ID 34
    ib_Lin_FrameResponse response_34;
    response_34.interfaceId = ib_InterfaceIdentifier_LinFrameResponse;
    response_34.frame.id = 34;
    response_34.frame.checksumModel = ib_Lin_ChecksumModel_Enhanced;
    response_34.frame.dataLength = 6;
    uint8_t tmp[8] = {3, 4, 3, 4, 3, 4, 3, 4};
    memcpy(response_34.frame.data, tmp, sizeof(tmp));
    response_34.responseMode = ib_Lin_FrameResponseMode_TxUnconditional;

    const uint32_t numFrameResponses = 5;
    size_t   controllerConfigSize = sizeof(ib_Lin_ControllerConfig) + ((numFrameResponses - 1) * sizeof(ib_Lin_FrameResponse));
    controllerConfig = (ib_Lin_ControllerConfig*)malloc(controllerConfigSize);
    controllerConfig->interfaceId = ib_InterfaceIdentifier_LinControllerConfig;
    controllerConfig->controllerMode = ib_Lin_ControllerMode_Slave;
    controllerConfig->baudRate = 20000;
    controllerConfig->numFrameResponses = numFrameResponses;
    controllerConfig->frameResponses[0] = response_16;
    controllerConfig->frameResponses[1] = response_17;
    controllerConfig->frameResponses[2] = response_18;
    controllerConfig->frameResponses[3] = response_19;
    controllerConfig->frameResponses[4] = response_34;

    ib_Lin_Controller_Init(linController, controllerConfig);
}

void Slave_DoAction(ib_NanosecondsTime now)
{
    slaveNow = now;
    Timer_ExecuteAction(&slaveTimer, now);
}


void Slave_SimTask(void* context, ib_SimulationParticipant* participant, ib_NanosecondsTime now)
{
    printf("now=%llums\n", now / 1000000);
    Slave_DoAction(now);
    SleepMs(500);
}

void Slave_FrameStatusHandler(void* context, ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
                              ib_Lin_FrameStatus status, ib_NanosecondsTime timestamp)
{
    printf(">> lin::Frame{id=%d, cs=%d, dl=%d, d={%d %d %d %d %d %d %d %d}} status=%d timestamp=%llums\n", frame->id,
           frame->checksumModel, frame->dataLength, frame->data[0], frame->data[1], frame->data[2], frame->data[3],
           frame->data[4], frame->data[5], frame->data[6], frame->data[7], status, timestamp/1000000);
}

void Slave_WakeupPulse(ib_NanosecondsTime now) 
{
    printf("<< Wakeup pulse @%llums\n", now/1000000);
    ib_Lin_Controller_Wakeup(linController);
}

void Slave_GoToSleepHandler(void* context, ib_Lin_Controller* controller)
{
    printf("LIN Slave received go-to-sleep command; entering sleep mode.\n");
    // wakeup in 10 ms
    Timer_Set(&slaveTimer, slaveNow + 10000000, &Slave_WakeupPulse);
    ib_Lin_Controller_GoToSleepInternal(controller);
}

void Slave_WakeupHandler(void* context, ib_Lin_Controller* controller)
{
    printf(">> LIN Slave received wakeup pulse; entering normal operation mode.\n");
    ib_Lin_Controller_WakeupInternal(controller);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("usage: IbDemoCLin <ConfigJsonFile> <ParticipantName> [<DomainId>]\n");
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

    const char* controllerName = "LIN1";
    const char* networkName = "LIN1";
    returnCode = ib_Lin_Controller_Create(&linController, participant, controllerName, networkName);
    ib_SimulationParticipant_SetStopHandler(participant, NULL, &StopCallback);
    ib_SimulationParticipant_SetShutdownHandler(participant, NULL, &ShutdownCallback);
    ib_SimulationParticipant_SetPeriod(participant, 1000000);
    
    if (strcmp(participantName, "LinMaster") == 0)
    {
        Task tasks[6] = {{0, &Master_SendFrame_16}, {0, &Master_SendFrame_17}, {0, &Master_SendFrame_18},
                         {0, &Master_SendFrame_19}, {0, &Master_SendFrame_34}, {5000000, &Master_GoToSleep}};
        Schedule_Create(&masterSchedule, tasks, 6);

        ib_SimulationParticipant_SetInitHandler(participant, NULL, &Master_InitCallback);
        ib_Lin_Controller_RegisterFrameStatusHandler(linController, NULL, &Master_ReceiveFrameStatus);
        ib_Lin_Controller_RegisterWakeupHandler(linController, NULL, &Master_WakeupHandler);
        ib_SimulationParticipant_SetSimulationTask(participant, NULL, &Master_SimTask);
    }
    else
    {
        ib_SimulationParticipant_SetInitHandler(participant, NULL, &Slave_InitCallback);
        ib_Lin_Controller_RegisterFrameStatusHandler(linController, NULL, &Slave_FrameStatusHandler);
        ib_Lin_Controller_RegisterGoToSleepHandler(linController, NULL, &Slave_GoToSleepHandler);
        ib_Lin_Controller_RegisterWakeupHandler(linController, NULL, &Slave_WakeupHandler);
        ib_SimulationParticipant_SetSimulationTask(participant, NULL, &Slave_SimTask);
    }

    ib_ParticipantState outFinalParticipantState;
    returnCode = ib_SimulationParticipant_Run(participant, &outFinalParticipantState);

    printf("Simulation stopped. Final State:%d\n", outFinalParticipantState);

    ib_SimulationParticipant_Destroy(participant);
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
