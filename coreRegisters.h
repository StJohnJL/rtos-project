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
extern void callPendSV();

#endif /* COREREGISTERS_H_ */