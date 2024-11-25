;Author: Jared St. John

	.def setASP

.thumb
.const

setASP:
	MOV R0, #2
	MRS R1, CONTROL
	ORR R0, R0, R1
	MSR CONTROL, R0
	ISB
	BX  LR

	.def clearASP

clearASP:
	MOV R0, #2
	MVN R0, R0
	MRS R1, CONTROL
	AND R0, R0, R1
	MSR CONTROL, R0
	ISB
	BX  LR

	.def setTMPL

setTMPL:
	MOV R0, #1
	MRS R1, CONTROL
	ORR R0, R0, R1
	MSR CONTROL, R0
	ISB
	BX  LR


	.def clearTMPL

clearTMPL:
	MOV R0, #1
	MVN R0, R0
	MRS R1, CONTROL
	and R0, R0, R1
	MSR CONTROL, R0
	ISB
	BX  LR

	.def setPSP

setPSP:
	MSR PSP, R0
	BX  LR

	.def readMSP

readMSP:
	MRS R0, MSP
	BX  LR

	.def readPSP

readPSP:
	MRS R0, PSP
	BX  LR

	.def callSV

callSV:
	SVC #0
	BX  LR

	.def saveContext

saveContext:
	MRS R0, PSP
	STM R0, {R4-R11}
	MSR PSP, R0
	BX  LR

	.def loadContext

loadContext:
	MRS R0, PSP
	LDM R0, {R4-R11}
	MSR PSP, R0
	BX  LR

	.def setPC

setPC:
	MOV PC, R0
	BX  LR

	.def changeVals

changeVals:
	MOV R4, #1
	MOV R5, #2
	MOV R6, #3
	MOV R7, #4
	MOV R8, #5
	MOV R9, #6
	MOV R10, #7
	MOV R11, #8
	BX  LR

	.def returnFromException

returnFromException:
	MOVW R0, #0xFFFD
	MOVT R0, #0xFFFF
	MOV  LR, R0
	BX  LR
