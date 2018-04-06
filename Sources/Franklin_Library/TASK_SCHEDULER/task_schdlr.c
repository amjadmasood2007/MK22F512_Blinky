//***********************************************************************************
// Module Name:         task_schdlr.c
// Application:
// Platform:            TM4C129x MCU's, ARM Compiler TI v5.1.13
// Author(s):           Derrick Simeon
// Notes:
// Description:
// // *** Example layout of allocated memory for arbritrary task list ***
// ===========================
// | Empty                    | <= High address
// ===========================
// | Empty                    |
// ===========================
// | ...                      |
// | ...                      |
// | ...                      |
// ===========================
// | Empty                    |
// ===========================
// | Task #6 Idle Priority    |
// ===========================
// | Task #5 Idle Priority    |
// ===========================
// | Task #4 Low Priority     |
// ===========================
// | Task #3 Normal Priority  |
// ===========================
// | Task #2 Normal Priority  |
// ===========================
// | Task #1 High Priority    | <= Low address
// ===========================
//
// Reordering of priority tasks in memory is done each time a task is added or removed
// from the task list. These tasks are arranged by descending priority starting from the
// lowest memory address, allowing the scheduler to run without comparing each task's
// priority level at execution time.
//
// Usage instructions:
// The task list size should be optimized for the application by setting the define macro,
// m_nMaxTasks. The scheduler's main initialization function should be implemented
// to initialize a hardware timer that will increment the scheduler's clock tick counter at
// the frequency specified by the input argument.
// Copyright (c) 2016, Franklin Control Systems, all rights reserved
//***********************************************************************************

//**************
// Includes
//**************

// Standard C libraries
#include <stdio.h>

// Project-specific modules
#include "task_schdlr.h"

//**************
// Defines
//**************

//**************
// Local Typedefs
//**************

//**************
// Local Variables
//**************

static FCS_Task_t *m_pTaskLst = NULL;   // Active task list

static uint8_t m_nClkTcks = 0;      // Elapsed clock ticks for dispatcher processing
static uint8_t m_nMaxTasks = 0;     // Maximum number of tasks supported by task list
static uint8_t m_nPrtyTasks = 0;    // Number of active priority tasks
static uint8_t m_nIdleTasks = 0;    // Number of active idle tasks

#ifdef _DEBUG_ENABLE
static void (*m_pfnTimingStart)(void) = NULL;
static void (*m_pfnTimingStop)(void) = NULL;
#endif

//**************
// Global Variables
//**************
// Get array size.
#define ARRAY_COUNT(array)          (sizeof(array) / sizeof((array)[0]))
// Main system task list
static FCS_Task_t _m_taskLst[64];


void Scheduler_Init()
{
  FCS_TASK_SCHDLR_init(_m_taskLst, ARRAY_COUNT(_m_taskLst));
}

//**************
// Local Functions
//**************

static void _FCS_TASK_SCHDLR_initTask(FCS_Task_t *pTask);
static bool _FCS_TASK_SCHDLR_validateArguments(FCS_TaskCode_t taskCode, void *pTaskArg);
static FCS_Task_t *_FCS_TASK_SCHDLR_getTask(FCS_TaskCode_t taskCode, void *pTaskArg);

//***************************************************************************
// Function Name:   _FCS_TASK_SCHDLR_initTask
// Returns:         void
// Param1:          void
// Description:
//***************************************************************************
static void _FCS_TASK_SCHDLR_initTask(FCS_Task_t *pTask)
{
    pTask->code.pfnNoArg = NULL;
    pTask->pArg = NULL;
    pTask->priority = FCS_TASKPRIORITY_Idle;
    pTask->nIntvlTcks = 0;
    pTask->nElapsTcks = 0;
}

//***************************************************************************
// Function Name:   _FCS_TASK_SCHDLR_validateArguments
// Returns:         void
// Param1:          void
// Description:
//***************************************************************************
static bool _FCS_TASK_SCHDLR_validateArguments(FCS_TaskCode_t taskCode, void *pTaskArg)
{
    if(((taskCode.pfnHasArg == NULL) || (pTaskArg == NULL))
            && (taskCode.pfnNoArg == NULL))
    {
        return false;
    }
    else
    {
        return true;
    }
}

//***************************************************************************
// Function Name:   _FCS_TASK_SCHDLR_getTask
// Returns:         void
// Param1:          void
// Description:
//***************************************************************************
static FCS_Task_t *_FCS_TASK_SCHDLR_getTask(FCS_TaskCode_t taskCode, void *pTaskArg)
{
    FCS_Task_t *pTask;
    uint8_t idx;

    if(!_FCS_TASK_SCHDLR_validateArguments(taskCode, pTaskArg))
    {
        // Invalid task parameters.
        pTask = NULL;
    }
    else
    {
        // Initialize task as not yet found.
        pTask = NULL;

        // Search the task list.
        for(idx = 0; (idx < (m_nPrtyTasks + m_nIdleTasks)) && (pTask == NULL); idx++)
        {
            if(pTaskArg == NULL)
            {
                // No task argument.
                if(m_pTaskLst[idx].code.pfnNoArg == taskCode.pfnNoArg)
                {
                    // Task found in the current task list.
                    pTask = &m_pTaskLst[idx];
                }
                else
                {
                    // Task not found.
                    (void) 0;
                }
            }
            else
            {
                // Task has argument.
                if((m_pTaskLst[idx].code.pfnHasArg == taskCode.pfnHasArg)
                        && (m_pTaskLst[idx].pArg == pTaskArg))
                {
                    // Task found in the current task list.
                    pTask = &m_pTaskLst[idx];
                }
                else
                {
                    // Task not found.
                    (void) 0;
                }
            }
        }
    }

    return pTask;
}

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
void FCS_TASK_SCHDLR_init(FCS_Task_t *pTaskLst, uint8_t nMaxTasks)
{
    uint8_t idx;

    // Initialize scheduler.
    m_nClkTcks = 0;
    m_nMaxTasks = nMaxTasks;
    m_nPrtyTasks = 0;
    m_nIdleTasks = 0;
    m_pTaskLst = pTaskLst;

    // Initialize task list.
    for(idx = 0; idx < m_nMaxTasks; idx++)
    {
        _FCS_TASK_SCHDLR_initTask(&m_pTaskLst[idx]);
    }

#ifdef _DEBUG_ENABLE
    m_pfnTimingStart = NULL;
    m_pfnTimingStop = NULL;
#endif
}

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_clockTick
// Returns:         void
// Param1:          void
// Description:     Increments the scheduler's clock tick counter. A hardware
// timer is required to call this function at the desired tick frequency.
//***************************************************************************
void FCS_TASK_SCHDLR_clockTick(void)
{
    // Increment the clock tick counter.
    m_nClkTcks++;
}

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
bool FCS_TASK_SCHDLR_addTask(FCS_TaskCode_t taskCode, void *pTaskArg,
        uint32_t intvlTicks, FCS_TaskPriority_e priority)
{
    FCS_Task_t *pTask;
    FCS_Task_t tempTask;
    uint32_t nTempTicks;
    uint8_t idx;

    if(!_FCS_TASK_SCHDLR_validateArguments(taskCode, pTaskArg))
    {
        // Invalid arguments.
        return false;
    }

    // Search for existing task with same code and argument.
    if(_FCS_TASK_SCHDLR_getTask(taskCode, pTaskArg) != NULL)
    {
        // Task already exists.
        return false;
    }

    // Check if task list is already full.
    if((m_nPrtyTasks + m_nIdleTasks) >= m_nMaxTasks)
    {
        // Task list is full. Failed to add new task.
        return false;
    }

    // Update existing priority tasks' elapsed times before adding the new task.
    if(m_nClkTcks > 0)
    {
        // Save tick count so each task is processed the same.
        nTempTicks = m_nClkTcks;
        m_nClkTcks -= nTempTicks;

        for(idx = 0; idx < m_nPrtyTasks; idx++)
        {
            // Calculate task's elapsed time.
            m_pTaskLst[idx].nElapsTcks += nTempTicks;
        }
    }

    // Get next available task structure in the task list.
    pTask = &m_pTaskLst[m_nPrtyTasks + m_nIdleTasks];

    // Update task count.
    switch(priority)
    {
        case FCS_TASKPRIORITY_Idle:
            m_nIdleTasks++;
            break;

        case FCS_TASKPRIORITY_Low:
        case FCS_TASKPRIORITY_Normal:
        case FCS_TASKPRIORITY_High:
        case FCS_TASKPRIORITY_Critical:
            m_nPrtyTasks++;
            break;

        default:
            // Unexpected priority assignment.
            return false;
    }

    // Timed tasks are added going from lowest to highest index in the tasks array.
    pTask->code = taskCode;
    pTask->pArg = pTaskArg;
    pTask->nIntvlTcks = intvlTicks;
    pTask->nElapsTcks = 0;
    pTask->priority = priority;

    // Reorganize tasks. Start from high address and work to low address.
    for((void) 0; (pTask > m_pTaskLst) && (pTask->priority > (pTask - 1)->priority); pTask--)
    {
        // This task has higher priority. Swap their positions.
        tempTask = *pTask;
        *pTask = *(pTask - 1);
        *(pTask - 1) = tempTask;

        // If elapsed tick counter was previously used as either task's argument, preserve it.
        if(pTask->pArg == &(pTask - 1)->nElapsTcks)
        {
            pTask->pArg = &pTask->nElapsTcks;
        }

        if((pTask - 1)->pArg == &pTask->nElapsTcks)
        {
            (pTask - 1)->pArg = &(pTask - 1)->nElapsTcks;
        }
    }

    // Task successfully added.
    return true;
}

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_removeTask
// Returns:         Success status of task removal.
// Param1:          task - Task to remove.
// Description:     Removes a task from the task list.
//***************************************************************************
bool FCS_TASK_SCHDLR_removeTask(FCS_Task_t task)
{
    FCS_Task_t *pTask;

    // Search for task in the task list.
    pTask = _FCS_TASK_SCHDLR_getTask(task.code, task.pArg);

    if(pTask == NULL)
    {
        // Task not found.
        return false;
    }

    // Update task count.
    switch(pTask->priority)
    {
        case FCS_TASKPRIORITY_Idle:
            m_nIdleTasks--;
            break;

        case FCS_TASKPRIORITY_Low:
        case FCS_TASKPRIORITY_Normal:
        case FCS_TASKPRIORITY_High:
        case FCS_TASKPRIORITY_Critical:
            m_nPrtyTasks--;
            break;

        default:
            // Unexpected priority assignment.
            return false;
    }

    // Remove task by initializing the structure.
    _FCS_TASK_SCHDLR_initTask(pTask);

    // Shift the task list if necessary.
    for((void) 0; pTask < &m_pTaskLst[m_nPrtyTasks + m_nIdleTasks]; pTask++)
    {
        // Shift task down.
        *pTask = *(pTask + 1);

        // If elapsed tick counter was previously used as the task's argument, preserve it.
        if(pTask->pArg == &(pTask + 1)->nElapsTcks)
        {
            pTask->pArg = &pTask->nElapsTcks;
        }
    }

    // Task successfully removed.
    return true;
}

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_updateTask
// Returns:         Success status of task update.
// Param1:          task - Copy of original task structure.
// Param2:          intvlTicks - Dispatch interval.
// Param3:          priority - Priority level.
// Description:     Updates an existing task's dispatch interval and priority level.
//***************************************************************************
bool FCS_TASK_SCHDLR_updateTask(FCS_Task_t task, uint32_t intvlTicks, FCS_TaskPriority_e priority)
{
    FCS_Task_t *pTask;

    // Search for task in the task list.
    pTask = _FCS_TASK_SCHDLR_getTask(task.code, task.pArg);

    if(pTask != NULL)
    {
        // Task found. Update the priority and interval ticks.
        pTask->priority = priority;
        pTask->nIntvlTcks = intvlTicks;
        pTask->nElapsTcks = 0;

        // Task successfully updated.
        return true;
    }
    else
    {
        // Task not found.
        return false;
    }
}

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_getTask
// Returns:         Copy of task structure.
// Param1:          taskCode - Points to code associated with the task.
// Param2:          *pTaskArg - Optional argument passed to task's code.
// Description:     Retrieves a copy of the task structure associated with the code
// and optional argument.
//***************************************************************************
FCS_Task_t FCS_TASK_SCHDLR_getTask(FCS_TaskCode_t taskCode, void *pTaskArg)
{
    return *_FCS_TASK_SCHDLR_getTask(taskCode, pTaskArg);
}

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_dispatcher
// Returns:         void
// Param1:          void
// Description:     Main loop to process the task list. The scheduler's clock tick
// counter is monitored and tasks are dispatched at their defined intervals in order
// of priority level.
//***************************************************************************
#pragma TASK(FCS_TASK_SCHDLR_dispatcher);
void FCS_TASK_SCHDLR_dispatcher(void)
{
    static uint8_t idx;
    static uint32_t ticks;

#ifdef _DEBUG_ENABLE
    if((m_pfnTimingStart == NULL) || (m_pfnTimingStop == NULL))
    {
        // Hang the dispatcher.
        while(true) {(void) 0;}
    }
#endif

    // Initialize main clock tick counter.
    m_nClkTcks = 0;

    // Run scheduler loop indefinitely.
    while(true)
    {
        if(m_nClkTcks > 0)
        {
            // Save current tick count so each task is processed the same.
            ticks = m_nClkTcks;
            m_nClkTcks -= ticks;

            // Update all priority tasks.
            for(idx = 0; idx < m_nPrtyTasks; idx++)
            {
                m_pTaskLst[idx].nElapsTcks += ticks;

                if(m_pTaskLst[idx].nElapsTcks >= m_pTaskLst[idx].nIntvlTcks)
                {
#ifdef _DEBUG_ENABLE
                    m_pfnTimingStart();
#endif

                    // Interval has expired. Run the task.
                    if(m_pTaskLst[idx].pArg != NULL)
                    {
                        // Pass the stored argument.
                        m_pTaskLst[idx].code.pfnHasArg(m_pTaskLst[idx].pArg);
                    }
                    else
                    {
                        // Pass pointer to the elapsed time value.
                        m_pTaskLst[idx].code.pfnHasArg(&m_pTaskLst[idx].nElapsTcks);
                    }

#ifdef _DEBUG_ENABLE
                    m_pfnTimingStop();
#endif

                    // Reset ticks elapsed.
                    m_pTaskLst[idx].nElapsTcks = 0;
                }
            }
        }
        else
        {
            // Process idle tasks while priority tasks are waiting.
            for(idx = m_nPrtyTasks; idx < (m_nPrtyTasks + m_nIdleTasks); idx++)
            {
                // Run the idle task.
                if(m_pTaskLst[idx].pArg != NULL)
                {
                    m_pTaskLst[idx].code.pfnHasArg(m_pTaskLst[idx].pArg);
                }
                else
                {
                    m_pTaskLst[idx].code.pfnNoArg();
                }
            }
        }
    }
}

//***************************************************************************
// Function Name:   FCS_TASK_SCHDLR_registerTimingCbs
// Returns:         void
// Param1:          void
// Description:
//***************************************************************************
#ifdef _DEBUG_ENABLE
extern void FCS_TASK_SCHDLR_registerTimingCbs(void (*pfnTimingStart)(void),
        void (*pfnTimingStop)(void))
{
    m_pfnTimingStart = pfnTimingStart;
    m_pfnTimingStop = pfnTimingStop;
}
#endif
