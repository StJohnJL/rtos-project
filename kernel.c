// Kernel functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "mm.h"
#include "coreRegisters.h"
#include "kernel.h"

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// mutex
typedef struct _mutex
{
    bool lock;
    uint8_t queueSize;
    uint8_t processQueue[MAX_MUTEX_QUEUE_SIZE];
    uint8_t lockedBy;
} mutex;
mutex mutexes[MAX_MUTEXES];

// semaphore
typedef struct _semaphore
{
    uint8_t count;
    uint8_t queueSize;
    uint8_t processQueue[MAX_SEMAPHORE_QUEUE_SIZE];
} semaphore;
semaphore semaphores[MAX_SEMAPHORES];

// task states
#define STATE_INVALID           0 // no task
#define STATE_STOPPED           1 // stopped, all memory freed
#define STATE_READY             2 // has run, can resume at any time
#define STATE_DELAYED           3 // has run, but now awaiting timer
#define STATE_BLOCKED_MUTEX     4 // has run, but now blocked by semaphore
#define STATE_BLOCKED_SEMAPHORE 5 // has run, but now blocked by semaphore

// SVC call numbers
#define SVC_YIELD  0
#define SVC_SLEEP  1
#define SVC_LOCK   2
#define SVC_UNLOCK 3
#define SVC_WAIT   4
#define SVC_POST   5

// task
uint8_t taskCurrent = 0;          // index of last dispatched task
uint8_t taskCount = 0;            // total number of valid tasks

// control
bool priorityScheduler = true;    // priority (true) or round-robin (false)
bool priorityInheritance = false; // priority inheritance for mutexes
bool preemption = false;          // preemption (true) or cooperative (false)

// tcb
#define NUM_PRIORITIES   16
struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    void *pid;                     // used to uniquely identify thread (add of task fn)
    void *spInit;                  // original top of stack
    void *sp;                      // current stack pointer
    uint8_t priority;              // 0=highest
    uint8_t currentPriority;       // 0=highest (needed for pi)
    uint32_t ticks;                // ticks until sleep complete
    uint64_t srd;                  // MPU subregion disable bits
    char name[16];                 // name of task used in ps command
    uint8_t mutex;                 // index of the mutex in use or blocking the thread
    uint8_t semaphore;             // index of the semaphore that is blocking the thread
} tcb[MAX_TASKS];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex)
{
    bool ok = (mutex < MAX_MUTEXES);
    if (ok)
    {
        mutexes[mutex].lock = false;
        mutexes[mutex].lockedBy = 0;
    }
    return ok;
}

bool initSemaphore(uint8_t semaphore, uint8_t count)
{
    bool ok = (semaphore < MAX_SEMAPHORES);
    {
        semaphores[semaphore].count = count;
    }
    return ok;
}

// REQUIRED: initialize systick for 1ms system timer
void initRtos(void)
{
    uint8_t i;
    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }
}

// REQUIRED: Implement prioritization to NUM_PRIORITIES
uint8_t rtosScheduler(void)
{
    bool ok;
    static uint8_t task = 0xFF;
    ok = false;
    while (!ok)
    {
        task++;
        if (task >= MAX_TASKS)
            task = 0;
        ok = (tcb[task].state == STATE_READY);
    }
    return task;
}

// REQUIRED: modify this function to start the operating system
// by calling scheduler, set srd bits, setting PSP, ASP bit, call fn with fn add in R0
// fn set TMPL bit, and PC <= fn
void startRtos(void)
{
    while(1) {
        taskCurrent = rtosScheduler();

        applySramAccessMask(tcb[taskCurrent].srd);

        setPSP((uint32_t)tcb[taskCurrent].sp);
        setASP();

        setPC((uint32_t)tcb[taskCurrent].pid);

        //setTMPL();
    }
}

// REQUIRED:
// add task if room in task list
// store the thread name
// allocate stack space and store top of stack in sp and spInit
// set the srd bits based on the memory allocation
// initialize the created stack to make it appear the thread has run before
bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    bool ok = false;
    uint8_t i = 0, j = 0;
    bool found = false;
    if (taskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent reentrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }
        if (!found)
        {
            // find first available tcb record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}
            tcb[i].state = STATE_READY;
            tcb[i].pid = fn;

            tcb[i].sp = mallocFromHeap(stackBytes);
            tcb[i].spInit = tcb[i].sp;

            tcb[i].priority = priority;

            addSramAccessWindow(&tcb[i].srd, (uint32_t*)&tcb[i].spInit, stackBytes);

            do {
                tcb[i].name[j] = name[j];
                j++;
            } while(name[j] != '\0');

            uint32_t* stackPointer = (uint32_t*)tcb[i].sp;

            *(stackPointer - 1) = 0x41000000;    // Write xPSR
            *(stackPointer - 2) = (uint32_t)fn;            // Write PC
            *(stackPointer - 3) = (uint32_t)rtosScheduler; // Write LR
            *(stackPointer - 4) = 0x0000000C;    // Write R12
            *(stackPointer - 5) = 0x00000003;    // Write R3
            *(stackPointer - 6) = 0x00000002;    // Write R2
            *(stackPointer - 7) = 0x00000001;    // Write R1
            *(stackPointer - 8) = 0x00000000;    // Write R0
            *(stackPointer - 9) = 0x00000000;    // Write R4-R11 with zeros
            *(stackPointer - 10) = 0x00000000;
            *(stackPointer - 11) = 0x00000000;
            *(stackPointer - 12) = 0x00000000;
            *(stackPointer - 13) = 0x00000000;
            *(stackPointer - 14) = 0x00000000;
            *(stackPointer - 15) = 0x00000000;
            *(stackPointer - 16) = 0x00000000;

            tcb[i].sp = (void*)(stackPointer - 16);

            // increment task count
            taskCount++;
            ok = true;
        }
    }
    return ok;
}

// REQUIRED: modify this function to restart a thread
void restartThread(_fn fn)
{
}

// REQUIRED: modify this function to stop a thread
// REQUIRED: remove any pending semaphore waiting, unlock any mutexes
void stopThread(_fn fn)
{
}

// REQUIRED: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority)
{
}

// REQUIRED: modify this function to yield execution back to scheduler using pendsv
void yield(void)
{
    callSV(SVC_YIELD);
}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
void sleep(uint32_t tick)
{
    tcb[taskCurrent].ticks = tick;
    if(tcb[taskCurrent].state != STATE_BLOCKED_SEMAPHORE && tcb[taskCurrent].state != STATE_BLOCKED_MUTEX) {
        tcb[taskCurrent].state = STATE_DELAYED;
    }
    callSV(SVC_SLEEP);
}

// REQUIRED: modify this function to lock a mutex using pendsv
void lock(int8_t mutex)
{
    tcb[taskCurrent].mutex = mutex;
    callSV(SVC_LOCK);
}

// REQUIRED: modify this function to unlock a mutex using pendsv
void unlock(int8_t mutex)
{
    tcb[taskCurrent].mutex = mutex;
    callSV(SVC_UNLOCK);
}

// REQUIRED: modify this function to wait a semaphore using pendsv
void wait(int8_t semaphore)
{
    tcb[taskCurrent].semaphore = semaphore;
    callSV(SVC_WAIT);
}

// REQUIRED: modify this function to signal a semaphore is available using pendsv
void post(int8_t semaphore)
{
    tcb[taskCurrent].semaphore = semaphore;
    callSV(SVC_POST);
}

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr(void)
{
    uint8_t index;

    if(preemption) {

    }
    else {
        for(index = 0; index < MAX_TASKS; index++) {
            if(tcb[index].ticks != 0) {
                tcb[index].ticks--;
            }

            if(tcb[index].state == STATE_DELAYED && tcb[index].ticks == 0) {
                if(tcb[index].state != STATE_BLOCKED_MUTEX && tcb[index].state != STATE_BLOCKED_SEMAPHORE) {
                    tcb[index].state = STATE_READY;
                }
            }
        }
    }
}

// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
void pendSvIsr(void)
{
    saveContext();
    tcb[taskCurrent].sp = (void*)readPSP();

    taskCurrent = rtosScheduler();

    setPSP((uint32_t)tcb[taskCurrent].sp);
    loadContext();

    returnFromException();
    NVIC_SYS_HND_CTRL_R &= ~NVIC_SYS_HND_CTRL_PNDSV;
}

// REQUIRED: modify this function to add support for the service call
// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr(void)
{
    uint8_t callNumber = readR0();
    uint8_t index;
    uint8_t mutexId = tcb[taskCurrent].mutex;
    uint8_t semaphoreId = tcb[taskCurrent].semaphore;
    bool idFound = 0;

    switch(callNumber) {
    case SVC_YIELD:
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
        break;

    case SVC_SLEEP:
        NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE; // Enable Systick timer
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
        break;

    case SVC_LOCK:
        if(mutexes[mutexId].lock) {
            if(mutexes[mutexId].queueSize < MAX_MUTEX_QUEUE_SIZE && mutexes[mutexId].lockedBy != taskCurrent) {
                mutexes[mutexId].processQueue[mutexes[mutexId].queueSize] = taskCurrent;
                mutexes[mutexId].queueSize++;
                tcb[taskCurrent].state = STATE_BLOCKED_MUTEX;
                NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
            }
        }
        else {
            tcb[taskCurrent].mutex = 0;
            mutexes[mutexId].lock = 1;
            mutexes[mutexId].lockedBy = taskCurrent;
        }
        break;

    case SVC_UNLOCK:
        if(mutexes[mutexId].lock) {
            if(mutexes[mutexId].queueSize > 0) {
                mutexes[mutexId].lockedBy = mutexes[mutexId].processQueue[mutexes[mutexId].queueSize - 1];

                for(index = 0; index < MAX_MUTEX_QUEUE_SIZE; index++) {
                    if(index != MAX_MUTEX_QUEUE_SIZE - 1) {
                        mutexes[mutexId].processQueue[index] = mutexes[mutexId].processQueue[index + 1];
                    }
                    else {
                        mutexes[mutexId].processQueue[index] = 0;
                    }
                }

                tcb[mutexes[mutexId].lockedBy].mutex = 0;

                if(tcb[mutexes[mutexId].lockedBy].ticks == 0) {
                    tcb[mutexes[mutexId].lockedBy].state = STATE_READY;
                }
                else {
                    tcb[mutexes[mutexId].lockedBy].state = STATE_DELAYED;
                }

                mutexes[mutexId].queueSize--;
            }
            else {
                mutexes[mutexId].lock = 0;
                mutexes[mutexId].lockedBy = 0;
            }
        }
        break;

    case SVC_WAIT:
        if(semaphores[semaphoreId].count == 0 && semaphores[semaphoreId].queueSize != MAX_SEMAPHORE_QUEUE_SIZE) {
            for(index = 0; index < MAX_SEMAPHORE_QUEUE_SIZE; index++) {
                if(semaphores[semaphoreId].processQueue[index] == taskCurrent) {
                    idFound = 1;
                }
            }

            index = 0;
            if(!idFound) {
                while(semaphores[semaphoreId].processQueue[index] != 0) {
                    index++;
                };

                semaphores[semaphoreId].processQueue[index] = taskCurrent;
                semaphores[semaphoreId].queueSize++;
                tcb[taskCurrent].state = STATE_BLOCKED_SEMAPHORE;
                NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
            }

        }
        else {
            for(index = 0; index < MAX_SEMAPHORE_QUEUE_SIZE; index++) {
                if(semaphores[semaphoreId].processQueue[index] == taskCurrent) {
                    idFound = 1;
                }
            }

            if(!idFound) {
                semaphores[semaphoreId].count--;
                tcb[taskCurrent].semaphore = 0;
            }
        }
        break;

    case SVC_POST:
        if(semaphores[semaphoreId].queueSize != 0) {
            if(tcb[semaphores[semaphoreId].processQueue[0]].ticks == 0) {
                tcb[semaphores[semaphoreId].processQueue[0]].state = STATE_READY;
            }
            else {
                tcb[semaphores[semaphoreId].processQueue[0]].state = STATE_DELAYED;
            }

            tcb[semaphores[semaphoreId].processQueue[0]].semaphore = 0;

            for(index = 0; index < MAX_SEMAPHORE_QUEUE_SIZE; index++) {
                if(index != MAX_MUTEX_QUEUE_SIZE - 1) {
                    semaphores[semaphoreId].processQueue[index] = semaphores[semaphoreId].processQueue[index + 1];
                }
                else {
                    semaphores[semaphoreId].processQueue[index] = 0;
                }
            }

            semaphores[semaphoreId].queueSize--;
        }
        else {
            semaphores[semaphoreId].count++;
            tcb[taskCurrent].semaphore = 0;
        }
        break;
    }
}

