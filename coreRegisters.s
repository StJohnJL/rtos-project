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
	BX LR

	.def clearASP

clearASP:
	MOV R0, #2
	MVN R0, R0
	MRS R1, CONTROL
	AND R0, R0, R1
	MSR CONTROL, R0
	ISB
	BX LR

	.def setTMPL

setTMPL:
	MOV R0, #1
	MRS R1, CONTROL
	ORR R0, R0, R1
	MSR CONTROL, R0
	ISB
	BX LR


	.def clearTMPL

clearTMPL:
	MOV R0, #1
	MVN R0, R0
	MRS R1, CONTROL
	and R0, R0, R1
	MSR CONTROL, R0
	ISB
	BX LR

	.def setPSP

setPSP:
	MSR PSP, R0
	BX LR

	.def readMSP

readMSP:
	MRS R0, MSP
	BX LR

	.def readPSP

readPSP:
	MRS R0, PSP
	BX LR

	.def callPendSV

callPendSV:
	SVC #0
	BX LR
