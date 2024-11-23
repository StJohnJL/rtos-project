// Memory manager functions
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
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "mm.h"

#define ALLOCATIONS_1024 16
#define ALLOCATIONS_512 24

static bool alloc_1024[ALLOCATIONS_1024] = {0};
static uint8_t alloc_id_1024[ALLOCATIONS_1024] = {0};
static bool alloc_512[ALLOCATIONS_512] = {0};
static uint8_t alloc_id_512[ALLOCATIONS_512] = {0};
static uint8_t used_alloc_ids[ALLOCATIONS_1024 + ALLOCATIONS_512];

#define REGION_COUNT 6

uint8_t pid = 123;
char stringBuffer[20] = {0};

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// REQUIRED: add your malloc code here and update the SRD bits for the current thread
void * mallocFromHeap(uint32_t size_in_bytes)
{
    static uint8_t allocation_id = 1;

    uint8_t index = 0;
    void* mem_pointer = 0;
    bool allocation_id_free;

    if(size_in_bytes <= 512) {
        for(index = 0; index < ALLOCATIONS_512; index++) {
            if(!alloc_512[index]) {
                alloc_512[index] = 1;
                alloc_id_512[index] = allocation_id;
                mem_pointer = (void*)(0x20001000 + 512*(index));
                goto post_alloc_processing;
            }
        }
        for(index = 0; index < ALLOCATIONS_1024; index++) {
            if(!alloc_1024[index]) {
                alloc_1024[index] = 1;
                alloc_id_1024[index] = allocation_id;
                mem_pointer = (void*)(0x20008000 - 1024*(index+1));
                goto post_alloc_processing;
            }
        }
    }
    else if(size_in_bytes > 512 && size_in_bytes <= 1024) {
        for(index = 0; index < ALLOCATIONS_1024; index++) {
            if(!alloc_1024[index]) {
                alloc_1024[index] = 1;
                alloc_id_1024[index] = allocation_id;
                mem_pointer = (void*)(0x20008000 - 1024*(index+1));
                goto post_alloc_processing;
            }
        }
        for(index = 0; index < ALLOCATIONS_512 - 1; index++) {
            if(!alloc_512[index] && !alloc_512[index+1]) {
                alloc_512[index] = alloc_512[index+1] = 1;
                alloc_id_512[index] = alloc_id_512[index+1] = allocation_id;
                mem_pointer = (void*)(0x20001000 + 512*(index));
                goto post_alloc_processing;
            }
        }
    }
    else if(size_in_bytes > 1024 && size_in_bytes <= 1536) {
        if(!alloc_512[ALLOCATIONS_512-1] && !alloc_1024[ALLOCATIONS_1024-1]) {
            alloc_512[ALLOCATIONS_512 - 1] = 1;
            alloc_id_512[ALLOCATIONS_512 - 1] = allocation_id;
            alloc_1024[ALLOCATIONS_1024 - 1] = 1;
            alloc_id_1024[ALLOCATIONS_1024 - 1] = allocation_id;
            mem_pointer = (void*)(0x20003E00);
            goto post_alloc_processing;
        }
        else {
            for(index = 0; index < ALLOCATIONS_512 - 2; index++) {
                if(!alloc_512[index] && !alloc_512[index + 1] && !alloc_512[index + 2]) {
                    alloc_512[index] = alloc_512[index + 1] = alloc_512[index + 2] = 1;
                    alloc_id_512[index] = alloc_id_512[index + 1] = alloc_id_512[index + 2] = allocation_id;
                    mem_pointer = (void*)(0x20001000 + 512*(index));
                    goto post_alloc_processing;
                }
            }
            for(index = 0; index < ALLOCATIONS_1024 - 1; index++) {
                if(!alloc_1024[index] && !alloc_1024[index + 1]) {
                    alloc_1024[index] = alloc_1024[index + 1] = 1;
                    alloc_id_1024[index] = alloc_id_1024[index + 1] = allocation_id;
                    mem_pointer = (void*)(0x20008000 - 1024*(index+2));
                    goto post_alloc_processing;
                }
            }
        }
    }
    else if(size_in_bytes > 1536) {
        uint8_t j;
        int16_t tempSize;
        uint8_t largeSlotsNeeded = 0;
        uint8_t smallSlotsNeeded = 0;

        tempSize = (int16_t)size_in_bytes;

        while(tempSize > 0) {
            smallSlotsNeeded++;
            tempSize -= 512;
        }

        tempSize = (int16_t)size_in_bytes;

        while(tempSize > 0) {
            largeSlotsNeeded++;
            tempSize -= 1024;
        }
        bool isFree;
        for(index = largeSlotsNeeded - 1; index < ALLOCATIONS_1024; index++) {
            bool isFree = 1;
            for(j = 0; j < largeSlotsNeeded; j++) {
                if(alloc_1024[index - j]) {
                    isFree = 0;
                    break;
                }
            }

            if(isFree) {
                for(j = 0; j < largeSlotsNeeded; j++) {
                    alloc_1024[index - j] = 1;
                    alloc_id_1024[index - j] = allocation_id;
                }
                mem_pointer = (void*)(0x20008000 - 1024*(index + 1));
                goto post_alloc_processing;
            }
        }
        for(index = 0; index < ALLOCATIONS_512; index++) {
            isFree = 1;
            for(j = 0; j < smallSlotsNeeded; j++) {
                if(alloc_512[index + j]) {
                    isFree = 0;
                    break;
                }
            }

            if(isFree) {
                for(j = 0; j < smallSlotsNeeded; j++) {
                    alloc_512[index + j] = 1;
                    alloc_id_512[index + j] = allocation_id;
                }
                mem_pointer = (void*)(0x20001000 + 512*(index));
                goto post_alloc_processing;
            }
        }
    }

post_alloc_processing:
    if(mem_pointer != 0) {
        for(index = 0; index < ALLOCATIONS_512 + ALLOCATIONS_1024; index++) {
            if(used_alloc_ids[index] == 0 ) {
                used_alloc_ids[index] = allocation_id;
                break;
            }
        }
        do {
            allocation_id++;
            allocation_id_free = 1;

            for(index = 0; index < ALLOCATIONS_512 + ALLOCATIONS_1024; index++) {
                if(allocation_id == used_alloc_ids[index]) {
                    allocation_id_free = 0;
                }
            }
        } while(!allocation_id_free);
    }

    return mem_pointer;
}

// REQUIRED: add your free code here and update the SRD bits for the current thread
void freeToHeap(void *pMemory)
{
    uint32_t mem_addr = (uint32_t)pMemory;
    uint8_t target_id;
    uint8_t index;
    uint8_t target_index;

    if(mem_addr >= 0x20001000 && mem_addr < 0x20003E00) {
        target_index = (mem_addr - 0x20001000)/512;
        target_id = alloc_id_512[target_index];
        for(index = target_index; index < ALLOCATIONS_512; index++) {
            if(alloc_id_512[index] == target_id) {
                alloc_512[index] = 0;
                alloc_id_512[index] = 0;
            }
            else {
                break;
            }
        }
    }
    else if(mem_addr < 0x20008000 && mem_addr > 0x20003E00) {
        target_index = (0x20008000 - mem_addr)/1024 - 1;
        target_id = alloc_id_1024[target_index];
        for(index = target_index; index > 0; index--) {
            if(alloc_id_1024[index] == target_id) {
                alloc_1024[index] = 0;
                alloc_id_1024[index] = 0;
            }
            else {
                break;
            }
        }
    }
    else if(mem_addr == 0x20003E00){
        alloc_512[ALLOCATIONS_512 - 1] = alloc_1024[ALLOCATIONS_1024 - 1] = 0;
        alloc_id_512[ALLOCATIONS_512 - 1] = alloc_id_1024[ALLOCATIONS_1024 - 1] = 0;
    }

    for(index = 0; index < ALLOCATIONS_512 + ALLOCATIONS_1024; index++) {
        if(used_alloc_ids[index] == target_id) {
            used_alloc_ids[index] = 0;
        }
    }
}

// REQUIRED: include your solution from the mini project
void allowFlashAccess(void)
{
    NVIC_MPU_NUMBER_R = 0;
    NVIC_MPU_BASE_R = 0x00000000; // Set base address to 0x00000000 (bottom of flash)
    NVIC_MPU_ATTR_R |= 0b011 << 24; // Set AP Field encoding for full RW access in privileged and unprivileged states
    // NVIC_MPU_ATTR_R |= 0b000111 << 16; // Set TEX and SCB bits
    NVIC_MPU_ATTR_R |= 0b11111 << 1; // Set size of MPU region to cover all of memory
    NVIC_MPU_ATTR_R |= 0b1; // Enable region
}

void allowPeripheralAccess(void)
{
}

void setupSramAccess(void)
{
    NVIC_MPU_NUMBER_R = 1; // Set Region Number
    NVIC_MPU_BASE_R = 0x20001000; // Set Base Address
    NVIC_MPU_ATTR_R = 0b011 << 24 | 0xFF << 8 | 0b1011 << 1 | 0b1; // Set Region to RW only in privileged mode, disable subregions, define size to 8*512B and enable region

    NVIC_MPU_NUMBER_R = 2;
    NVIC_MPU_BASE_R = 0x20002000;
    NVIC_MPU_ATTR_R = 0b011 << 24 | 0xFF << 8 | 0b1011 << 1 | 0b1;

    NVIC_MPU_NUMBER_R = 3;
    NVIC_MPU_BASE_R = 0x20003000;
    NVIC_MPU_ATTR_R = 0b011 << 24 | 0xFF << 8 | 0b1011 << 1 | 0b1; // Same as others but set size to 8*512B

    NVIC_MPU_NUMBER_R = 4;
    NVIC_MPU_BASE_R = 0x20004000;
    NVIC_MPU_ATTR_R = 0b011 << 24 | 0xFF << 8 | 0b1100 << 1 | 0b1; // Same as others but set size to 8*1024B

    NVIC_MPU_NUMBER_R = 5;
    NVIC_MPU_BASE_R = 0x20006000;
    NVIC_MPU_ATTR_R = 0b011 << 24 | 0xFF << 8 | 0b1100 << 1 | 0b1;
}

uint64_t createNoSramAccessMask(void)
{
    return 0x0000FFFFFFFFFFFF;
}

void addSramAccessWindow(uint64_t *srdBitMask, uint32_t *baseAddr, uint32_t size_in_bytes)
{
    uint8_t index;
    uint32_t mpuBaseRegion;
    uint8_t mpuBaseSubregion;
    uint8_t requiredSubregions;
    uint64_t tempSrdMask;

    tempSrdMask = *srdBitMask;
    tempSrdMask |= (uint64_t)0xFFFFFFFFFFFFFF << 8; // Clear regions 1-5

    if((uint32_t)baseAddr < 0x20003E00) {
        mpuBaseRegion = ((uint32_t)baseAddr - 0x20001000)/4096 + 1;
        mpuBaseSubregion = (((uint32_t)baseAddr - 0x20001000)/512)%8;
        requiredSubregions = size_in_bytes/512;

        for(index = 0; index < requiredSubregions; index++) {
            if(index != 0 && (mpuBaseSubregion + index)%8 == 0) {
                mpuBaseRegion++;
            }

            tempSrdMask &= ~(1 << (8*mpuBaseRegion + (mpuBaseSubregion + index)%8));
        }
    }
    else if((uint32_t)baseAddr > 0x20003E00) {
        mpuBaseRegion = 5 - (0x20008000 - (uint32_t)baseAddr)/8193;
        mpuBaseSubregion = 7 - ((0x20008000 - (uint32_t)baseAddr)/1024- 1)%8;
        requiredSubregions = size_in_bytes/1024;

        for(index = 0; index < requiredSubregions; index++) {
            if(index != 0 && (mpuBaseSubregion + index)%8 == 0) {
                mpuBaseRegion--;
            }

            tempSrdMask &= ~((uint64_t)1 << (8*mpuBaseRegion + (mpuBaseSubregion + index)%8));
        }
    }
    else if((uint32_t)baseAddr == 0x20003E00) {
        mpuBaseRegion = 3;
        mpuBaseSubregion = 7;
        requiredSubregions = 2;

        tempSrdMask &= ~((uint64_t)0b10000000 << 24 | (uint64_t)0b1 << 32);
    }

    applySramAccessMask(tempSrdMask);

    *srdBitMask = tempSrdMask;
}

void applySramAccessMask(uint64_t srdBitMask)
{
    uint8_t index;

    for(index = 0; index < REGION_COUNT; index++) {
        NVIC_MPU_NUMBER_R = index;
        NVIC_MPU_ATTR_R &= ~1;
        NVIC_MPU_ATTR_R &= ~(0xFF << 8);
        NVIC_MPU_ATTR_R |= ((srdBitMask >> 8*index) & 0xFF) << 8;
        NVIC_MPU_ATTR_R |= 1;
    }
}


