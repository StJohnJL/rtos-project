// Shell functions
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
#include "faults.h"
#include "tasks.h"
#include "coreRegisters.h"
#include "stringUtils.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// REQUIRED: If these were written in assembly
//           omit this file and add a faults.s file

// REQUIRED: code this function
void mpuFaultIsr(void)
{
    char stringBuffer[20] = {0};

    ORANGE_LED = 1;

    /*
    putsUart0("Bus fault in process ");
    itoa(pid, stringBuffer);
    putsUart0(stringBuffer);
    putsUart0(" \n");
    putsUart0("MSP: ");
    itoh(readMSP(), stringBuffer);
    putsUart0(stringBuffer);
    */

    volatile uint32_t* pspStackPointer = (volatile uint32_t*)(readPSP() + 32);

    putsUart0("\nPSP: ");
    itoh(readPSP(), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nMFault Flags: ");
    itoh(NVIC_FAULT_STAT_R, stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nxPSR: ");
    itoh(*(pspStackPointer - 1), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nPC: ");
    itoh(*(pspStackPointer - 2), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR12: ");
    itoh(*(pspStackPointer - 3), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR3: ");
    itoh(*(pspStackPointer - 4), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR2: ");
    itoh(*(pspStackPointer - 5), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR1: ");
    itoh(*(pspStackPointer - 6), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR0: ");
    itoh(*(pspStackPointer - 7), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nMemory Fault Address: ");
    itoh(NVIC_MM_ADDR_R, stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\n\n");

    NVIC_SYS_HND_CTRL_R &= ~(NVIC_SYS_HND_CTRL_MEMA);
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;
}

// REQUIRED: code this function
void hardFaultIsr(void)
{
    char stringBuffer[20] = {0};

    RED_LED = 1;

    /*
    putsUart0("Hard fault in process ");
    itoa(pid, stringBuffer);
    putsUart0(stringBuffer);
    putsUart0(" \n");
    putsUart0("MSP: ");
    itoh(readMSP(), stringBuffer);
    putsUart0(stringBuffer);
    */

    volatile uint32_t* pspStackPointer = (volatile uint32_t*)(readPSP() + 32);

    putsUart0("\nPSP: ");
    itoh(readPSP(), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nMFault Flags: ");
    itoh(NVIC_FAULT_STAT_R, stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nxPSR: ");
    itoh(*(pspStackPointer - 1), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nPC: ");
    itoh(*(pspStackPointer - 2), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR12: ");
    itoh(*(pspStackPointer - 3), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR3: ");
    itoh(*(pspStackPointer - 4), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR2: ");
    itoh(*(pspStackPointer - 5), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR1: ");
    itoh(*(pspStackPointer - 6), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nR0: ");
    itoh(*(pspStackPointer - 7), stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\nMemory Fault Address: ");
    itoh(NVIC_MM_ADDR_R, stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\n\n");
    while(1);
}

// REQUIRED: code this function
void busFaultIsr(void)
{
    char stringBuffer[20] = {0};

    GREEN_LED = 1;

    /*
    putsUart0("Bus fault in process ");
    itoa(pid, stringBuffer);
    putsUart0(stringBuffer);
    putsUart0("\n\n");
    */
    while(1);
}

// REQUIRED: code this function
void usageFaultIsr(void)
{
    char stringBuffer[20] = {0};

    if(!BUTTON_1) {
        YELLOW_LED = 1;

        /*
        putsUart0("Usage fault in process ");
        itoa(pid, stringBuffer);
        putsUart0(stringBuffer);
        putsUart0("\n\n");
        */
    }
    else if(!BUTTON_2) {
        *(volatile uint32_t*)0x400043FD = (uint32_t)1;
    }
    while(1);
}

