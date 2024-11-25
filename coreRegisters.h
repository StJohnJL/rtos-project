/*
 * changeAsp.h
 *
 *  Created on: Sep 19, 2024
 *      Author: jared
 */

#ifndef COREREGISTERS_H_
#define COREREGISTERS_H_

extern void setASP(void);
extern void clearASP(void);
extern void setTMPL(void);
extern void clearTMPL(void);
extern void setPSP(uint32_t i);
extern uint32_t readMSP(void);
extern uint32_t readPSP(void);
extern void callSV(uint8_t);
extern uint8_t readR0(void);
extern void saveContext(void);
extern void loadContext(void);
extern void setPC(uint32_t);
extern void changeVals(void);
extern void returnFromException(void);

#endif /* COREREGISTERS_H_ */
