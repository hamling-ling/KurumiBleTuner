﻿/***********************************************************************/
/*  													               */
/*      PROJECT NAME :  RLduino78                                      */
/*      FILE         :  interrupt_handlers.h                           */
/*      DESCRIPTION  :  Interrupt Handler Declarations                 */
/*      CPU SERIES   :  RL78 - G13                                     */
/*      CPU TYPE     :  R5F100LE                                       */
/*  													               */
/*      This file is generated by e2studio.                        */
/*  													               */
/***********************************************************************/                                                                       
                                                                                   
#ifndef INTERRUPT_HANDLERS_H
#define INTERRUPT_HANDLERS_H

extern void (*INT_TM_HOOK)();

//0x4
void INT_WDTI(void) __attribute__ ((interrupt));
//0x6
void INT_LVI(void) __attribute__ ((interrupt));
//0x8
void INT_P0(void) __attribute__ ((interrupt));
//0xA
void INT_P1(void) __attribute__ ((interrupt));
//0xC
void INT_P2(void) __attribute__ ((interrupt));
//0xE
void INT_P3(void) __attribute__ ((interrupt));
//0x10
void INT_P4(void) __attribute__ ((interrupt));
//0x12
void INT_P5(void) __attribute__ ((interrupt));
//0x14
void INT_ST2(void) __attribute__ ((interrupt));
//0x16
void INT_SR2(void) __attribute__ ((interrupt));
//0x18
void INT_SRE2(void) __attribute__ ((interrupt));
//0x1A
void INT_DMA0(void) __attribute__ ((interrupt));
//0x1C
void INT_DMA1(void) __attribute__ ((interrupt));
//0x1E
void INT_ST0(void) __attribute__ ((interrupt));
//0x20
void INT_SR0(void) __attribute__ ((interrupt));
//0x22
void INT_TM01H(void) __attribute__ ((interrupt));
//0x24
void INT_ST1(void) __attribute__ ((interrupt));
//0x26
void INT_SR1(void) __attribute__ ((interrupt));
//0x28
void INT_TM03H(void) __attribute__ ((interrupt));
//0x2A
void INT_IICA0(void) __attribute__ ((interrupt));
//0x2C
void INT_TM00(void) __attribute__ ((interrupt, weak));
//0x2E
void INT_TM01(void) __attribute__ ((interrupt));
//0x30
void INT_TM02(void) __attribute__ ((interrupt));
//0x32
void INT_TM03(void) __attribute__ ((interrupt));
//0x34
void INT_AD(void) __attribute__ ((interrupt));
//0x36
void INT_RTC(void) __attribute__ ((interrupt));
//0x38
void INT_IT(void) __attribute__ ((interrupt));
//0x3A
void INT_KR(void) __attribute__ ((interrupt));
//0x42
void INT_TM04(void) __attribute__ ((interrupt));
//0x44
void INT_TM05(void) __attribute__ ((interrupt));
//0x46
void INT_TM06(void) __attribute__ ((interrupt));
//0x48
void INT_TM07(void) __attribute__ ((interrupt));
//0x4A
void INT_P6(void) __attribute__ ((interrupt));
//0x4C
void INT_P7(void) __attribute__ ((interrupt));
//0x4E
void INT_P8(void) __attribute__ ((interrupt));
//0x50
void INT_P9(void) __attribute__ ((interrupt));
//0x52
void INT_P10(void) __attribute__ ((interrupt));
//0x54
void INT_P11(void) __attribute__ ((interrupt));
//0x5E
void INT_MD(void) __attribute__ ((interrupt));
//0x62
void INT_FL(void) __attribute__ ((interrupt));
//0x7E
void INT_BRK_I(void) __attribute__ ((interrupt));

//Hardware Vectors
//0x0
void PowerON_Reset(void) __attribute__ ((interrupt));
#endif
