/*******************************/
/*           test.cpp           */
/*******************************/

#include <stdio.h>
#include <stdlib.h>

#include "sys.h"
#include "rtos_api.h"
#include "defs.h"

// Declare tasks with RMA priorities (lower number = higher rate = higher priority)
DeclareTask(TaskIdle, 16);     // Lowest priority
DeclareTask(TaskHigh, 1);      // Highest priority
DeclareTask(TaskMedium, 5);
DeclareTask(TaskLow, 10);

DeclareResource(Res1, 12);
DeclareResource(Res2, 8);

DeclareEvent(Event1);
DeclareEvent(Event2);

void TestTaskPreemption();
void TestResourceManagement();
void TestEventManagement();
void TestRMA();

extern int SystemTick;
extern int TaskPeriods[MAX_TASK];
extern int TaskDeadlines[MAX_TASK];
extern TTask TaskQueue[MAX_TASK];
extern int RunningTask;
// Main test function
int test(void)
{
    printf("Starting RTOS Test Suite\n");

    // Test 1: Basic task management
    printf("\n=== Test 1: Basic Task Management ===\n");
    char name[] = "TaskIdle";
    StartOS(TaskIdle, TaskIdleprior, name);


    return 0;
}

// Idle task - runs various tests
TASK(TaskIdle)
{
    printf("TaskIdle: Starting tests\n");

    TestTaskPreemption();
    TestResourceManagement();
    TestEventManagement();

    TestRMA();

    printf("TaskIdle: All tests completed\n");

    ShutdownOS();

    TerminateTask();
}
// High priority task
TASK(TaskHigh)
{
    printf("TaskHigh: Running\n");
    printf("TaskHigh: This demonstrates preemption - running before lower priority tasks complete\n");

    if (TaskQueue[RunningTask].ceiling_priority > TaskHighprior)
    {
        char resName[] = "Res1";
        ReleaseResource(Res1, resName);
    }

    TerminateTask();
}

TASK(TaskMedium)
{
    printf("TaskMedium: Running\n");

    char resName[] = "Res1";
    GetResource(Res1, resName);

    printf("TaskMedium: Acquired resource\n");

    char taskName[] = "TaskHigh";
    ActivateTask(TaskHigh, TaskHighprior, taskName);

    printf("TaskMedium: Continuing after TaskHigh\n");

    ReleaseResource(Res1, resName);

    printf("TaskMedium: Resource released\n");

    TerminateTask();
}

TASK(TaskLow)
{
    printf("TaskLow: Running\n");

    printf("TaskLow: Waiting for Event1\n");
    char eventName[] = "Event1";
    WaitEvent(Event1, eventName);

    printf("TaskLow: Event1 received\n");

    TerminateTask();
}

// Test task preemption
void TestTaskPreemption()
{
    printf("\n--- Testing Task Preemption ---\n");

    int highTask = CreateTask(TaskHigh, TaskHighprior, (char*)"TaskHigh");
    int medTask = CreateTask(TaskMedium, TaskMediumprior, (char*)"TaskMedium");
    int lowTask = CreateTask(TaskLow, TaskLowprior, (char*)"TaskLow");

    ResumeTask(lowTask);

    ResumeTask(medTask);


    printf("--- Task Preemption Test Complete ---\n");
}


void TestResourceManagement()
{
    printf("\n--- Testing Resource Management ---\n");

    // Create tasks
    int highTask = CreateTask(TaskHigh, TaskHighprior, (char*)"TaskHigh");
    int medTask = CreateTask(TaskMedium, TaskMediumprior, (char*)"TaskMedium");

    ResumeTask(medTask);

    printf("--- Resource Management Test Complete ---\n");
}


void TestEventManagement()
{
    printf("\n--- Testing Event Management ---\n");

    // Create tasks
    int lowTask = CreateTask(TaskLow, TaskLowprior, (char*)"TaskLow");

    ResumeTask(lowTask);

    printf("Main: Setting Event1\n");
    SetEvent(Event1, (char*)"Event1");

    printf("--- Event Management Test Complete ---\n");
}

// Test Rate Monotonic Algorithm scheduling
void TestRMA()
{
    printf("\n--- Testing RMA Scheduling ---\n");

    int highTask = CreateTask(TaskHigh, TaskHighprior, (char*)"TaskHigh");
    int medTask = CreateTask(TaskMedium, TaskMediumprior, (char*)"TaskMedium");
    int lowTask = CreateTask(TaskLow, TaskLowprior, (char*)"TaskLow");

    SetTaskPeriod(highTask, 2);  // Run every 2 ticks
    SetTaskPeriod(medTask, 5);   // Run every 5 ticks
    SetTaskPeriod(lowTask, 10);  // Run every 10 ticks

    SetTaskDeadline(highTask, 2);
    SetTaskDeadline(medTask, 5);
    SetTaskDeadline(lowTask, 10);

    ResumeTask(highTask);
    ResumeTask(medTask);
    ResumeTask(lowTask);

    printf("Letting periodic tasks run for 20 ticks...\n");
    for (int i = 0; i < 20; i++)
    {
        SystemTick++;
        CheckDeadlines();
    }

    printf("--- RMA Scheduling Test Complete ---\n");
}