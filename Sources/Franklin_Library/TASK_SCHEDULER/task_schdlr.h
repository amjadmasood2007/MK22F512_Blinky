//***********************************************************************************
// Module Name:         task_schdlr.h
// Application:
// Platform:            TM4C129x MCU's, ARM Compiler TI v5.1.13
// Author(s):           Derrick Simeon
// Notes:
// Description:
// This module implements a simple cooperative task scheduler. Tasks which require immediate
// servicing via interrupts may be serviced outside the scheduler by their own dedicated ISR's.
//
// The scheduler implements:
// - Interval task scheduling
// - Priority levels
// - "Idle" tasks to run when priority-based, interval tasks are not ready/running
// - Run-time adjustments of settings for above features
//
// The scheduler operates on a clock tick counter which is incremented at a rate specified
// at initialization. Each task added to the scheduler's task list is assigned a priority
// level and an interval defining the minimum elapsed ticks between successive calls to run.
// The scheduler constantly checks the tick counter, adds it to each tasks tick counter, and
// queues them in order of priority. A task assigned with an "idle" priority (essentially a
// non-priority task) has no call interval and is only eligible to run when there are no
// priority tasks ready and the tick counter is has a value of 0.
// Copyright (c) 2016, Franklin Control Systems, all rights reserved
//***********************************************************************************

#ifndef TASK_SCHDLR_H_
#define TASK_SCHDLR_H_

//**************
// Includes
//**************

// Standard C libraries
#include <stdint.h>
#include <stdbool.h>

//**************
// Defines
//**************


//**************
// Global Typedefs
//**************

// Unique numeric identifier assigned to each task
typedef uint8_t FCS_TaskID_t;

/*** Enumerations ***/

// Assignable priorities levels for tasks
typedef enum _FCS_TaskPriority_e {
    FCS_TASKPRIORITY_Idle,      // System idle task
    FCS_TASKPRIORITY_Low,       // Low priority
    FCS_TASKPRIORITY_Normal,    // Normal priority
    FCS_TASKPRIORITY_High,      // High priority
    FCS_TASKPRIORITY_Critical,  // Critical priority
//    FCS_TASKPRIORITY_Immediate  // Serviced immediately through ISR
} FCS_TaskPriority_e;

/*** Unions ***/

// Points to task's executable code
typedef union _FCS_TaskCode_t {
    void (*pfnNoArg)(void);     // Execute with no arguments
    void (*pfnHasArg)(void *);  // Execute with pointer argument
} FCS_TaskCode_t;

/*** Structures ***/

// Scheduler task
typedef struct _FCS_Task_t {
    FCS_TaskID_t        id;
    FCS_TaskCode_t      code;           // Task's executable code
    void                *pArg;          // Optional pointer argument passed to task's code
    FCS_TaskPriority_e  priority;       // Scheduling priority level
    uint32_t            nIntvlTcks;     // Interval in ticks to dispatch task for processing
    uint32_t            nElapsTcks;     // Ticks elapsed since task was last processed
} FCS_Task_t;

//**************
// Global Variables
//**************

//**************
// Global Functions
//**************

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_init
// Returns:         void
// Param1:          *pTaskLst - Task list (array) to operate on.
// Param2:          maxTaskCnt - Max amount of tasks supported by task list.
// Description:     Initializes the scheduler and task list.
//***************************************************************************
extern void FCS_TASK_SCHDLR_init(FCS_Task_t *pTaskLst, uint8_t nMaxTasks);

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_clockTick
// Returns:         void
// Param1:          void
// Description:     Increments the scheduler's clock tick counter. A hardware
// timer is required to call this function at the desired tick frequency.
//***************************************************************************
extern void FCS_TASK_SCHDLR_clockTick(void);

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_addTask
// Returns:         Success status of task addition.
// Param1:          taskCode - Points to task's executable code.
// Param2:          *pTaskArg - Optional argument passed to task's code.
// Param3:          intvlTicks - Dispatch interval.
// Param4:          priority - Priority level.
// Description:     Adds a new task to the task list. The function call
// representing the task's executable code will be called with the optional
// argument if one is provided.
//***************************************************************************
extern bool FCS_TASK_SCHDLR_addTask(FCS_TaskCode_t taskCode, void *pTaskArg,
        uint32_t intvlTicks, FCS_TaskPriority_e priority);

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_removeTask
// Returns:         Success status of task removal.
// Param1:          task - Task to remove.
// Description:     Removes a task from the task list.
//***************************************************************************
extern bool FCS_TASK_SCHDLR_removeTask(FCS_Task_t task);

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_updateTask
// Returns:         Success status of task update.
// Param1:          task - Copy of original task structure.
// Param2:          intvlTicks - Dispatch interval.
// Param3:          priority - Priority level.
// Description:     Updates an existing task's dispatch interval and priority level.
//***************************************************************************
extern bool FCS_TASK_SCHDLR_updateTask(FCS_Task_t task, uint32_t intvlTicks,
        FCS_TaskPriority_e priority);

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_getTask
// Returns:         Copy of task structure.
// Param1:          taskCode - Points to code associated with the task.
// Param2:          *pTaskArg - Optional argument passed to task's code.
// Description:     Retrieves a copy of the task structure associated with the code
// and optional argument.
//***************************************************************************
extern FCS_Task_t FCS_TASK_SCHDLR_getTask(FCS_TaskCode_t taskCode, void *pTaskArg);

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_dispatcher
// Returns:         void
// Param1:          void
// Description:     Main loop to process the task list. The scheduler's clock tick
// counter is monitored and tasks are dispatched at their defined intervals in order
// of priority level.
//***************************************************************************
extern void FCS_TASK_SCHDLR_dispatcher(void);

extern void Scheduler_Init();

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_dispatcher
// Returns:         void
// Param1:          void
// Description:
//***************************************************************************
#ifdef _DEBUG_ENABLE
extern void FCS_TASK_SCHDLR_registerTimingCbs(void (*pfnTimingStart)(void),
        void (*pfnTimingStop)(void));
#endif


#endif /* TASK_SCHDLR_H_ */
