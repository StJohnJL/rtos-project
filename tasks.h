// Tasks
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef TASKS_H_
#define TASKS_H_

#define PB0 PORTC,4
#define PB1 PORTC,5
#define PB2 PORTC,6
#define PB3 PORTC,7
#define PB4 PORTD,6
#define PB5 PORTD,7
#define LED0 PORTF,2
#define LED1 PORTA,2
#define LED2 PORTA,3
#define LED3 PORTA,4
#define LED4 PORTE,0
#define UART0_RX PORTA,0
#define UART0_TX PORTA,1

#define BLUE_LED (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 2*4)))
#define ORANGE_LED (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 2*4)))
#define YELLOW_LED (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 3*4)))
#define GREEN_LED (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 4*4)))
#define RED_LED (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 0*4)))
#define BUTTON_0 (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 4*4)))
#define BUTTON_1 (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 5*4)))
#define BUTTON_2 (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 6*4)))
#define BUTTON_3 (*((volatile uint32_t *)(0x42000000 + (0x400063FC-0x40000000)*32 + 7*4)))
#define BUTTON_4 (*((volatile uint32_t *)(0x42000000 + (0x400073FC-0x40000000)*32 + 6*4)))
#define BUTTON_5 (*((volatile uint32_t *)(0x42000000 + (0x400073FC-0x40000000)*32 + 7*4)))

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initHw(void);

void idle(void);
void flash4Hz(void);
void oneshot(void);
void partOfLengthyFn(void);
void lengthyFn(void);
void readKeys(void);
void debounce(void);
void uncooperative(void);
void errant(void);
void important(void);

#endif
