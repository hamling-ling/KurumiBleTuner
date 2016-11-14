/***************************************************************************
 *
 * PURPOSE
 *   RLduino78 framework MCU depend header file.
 *
 * TARGET DEVICE
 *   RL78/G13
 *
 * AUTHOR
 *   Renesas Solutions Corp.
 *
 * $Date:: 2013-03-07 08:42:10 +0900#$
 *
 ***************************************************************************
 * Copyright (C) 2012 Renesas Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See file LICENSE.txt for further informations on licensing terms.
 ***************************************************************************/
/**
 * @file  RLduino78_mcu_depend.h
 * @brief RLduino78フレームワークMCU依存ヘッダファイル
 */
#ifndef RLDUINO78_MCU_DEPEND_H
#define RLDUINO78_MCU_DEPEND_H
/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include "pins_arduino.h"
#include "iodefine.h"
#include "iodefine_ext.h"


/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define	ASM			asm
#define INTERRUPT
#define NOP()		asm("nop;")
#define HALT()		asm("halt;");
#define STOP()		asm("stop;");

/* Port define. */
#define PORT_0				0
#define PORT_1				1
#define PORT_2				2
#define PORT_3				3
#define PORT_4				4
#define PORT_5				5
#define PORT_6				6
#define PORT_7				7
#define PORT_8				8
#define PORT_9				9
#define PORT_10				10
#define PORT_11				11
#define PORT_12				12
#define PORT_13				13
#define PORT_14				14
#define PORT_15				15

/* Software PWM 定義 */
#define SOFTWARE_PWM		0xF0
#define SW_PWM_TIMER        6

/* SFR register map. */
#define ADDR_PORT_REG			0xFFFFFF00
#define ADDR_PORT_MODE_REG		0xFFFFFF20
#define ADDR_PORT_PULL_UP_REG	0xFFFF0030
#define	ADDR_PORT_IN_MODE_REG	0xFFFF0040
#define ADDR_PORT_OUT_MODE_REG	0xFFFF0050

#ifndef configCPU_CLOCK_HZ
#define configCPU_CLOCK_HZ 					(32000000)	//!< CPUの動作周波数（RTOS未使用時に定義）
#endif

#define clockCyclesPerMicrosecond()			(configCPU_CLOCK_HZ / 1000000)				//!< CPUの動作周波数を1000000で割ったもの
#define clockCyclesToMicroseconds(cycle)	((cycle) / clockCyclesPerMicrosecond())		//!< サイクルあたりの時間[us]
#define microsecondsToClockCycles(us)		((us) * clockCyclesPerMicrosecond())		//!< 時間当たりのサイクル数


/***************************************************************************/
/*    GR-KURUMI（RL78/G13 48ピン）のコンフィグレーション                   */
/***************************************************************************/
#if defined(REL_GR_KURUMI)

/* External interrupt define. */
#define EXTERNAL_INTERRUPT_0	3
#define EXTERNAL_INTERRUPT_1	5

/* Port of digital pin define. */
#define DIGITAL_PIN_0		PORT_1	// P11
#define DIGITAL_PIN_1		PORT_1	// P12
#define DIGITAL_PIN_2		PORT_3	// P30
#define DIGITAL_PIN_3		PORT_1	// P16
#define DIGITAL_PIN_4		PORT_3	// P31
#define DIGITAL_PIN_5		PORT_1	// P15
#define DIGITAL_PIN_6		PORT_1	// P10
#define DIGITAL_PIN_7		PORT_0	// P00
#define DIGITAL_PIN_8		PORT_0	// P01
#define DIGITAL_PIN_9		PORT_1	// P13
#define DIGITAL_PIN_10		PORT_1	// P14
#define DIGITAL_PIN_11		PORT_7	// P72
#define DIGITAL_PIN_12		PORT_7	// P71
#define DIGITAL_PIN_13		PORT_7	// P70
#define DIGITAL_PIN_14		PORT_2	// P22
#define DIGITAL_PIN_15		PORT_2	// P23
#define DIGITAL_PIN_16		PORT_2	// P24
#define DIGITAL_PIN_17		PORT_2	// P25
#define DIGITAL_PIN_18		PORT_2	// P26
#define DIGITAL_PIN_19		PORT_2	// P27
#define DIGITAL_PIN_20		PORT_14	// P147
#define DIGITAL_PIN_21		PORT_12	// P120
#define DIGITAL_PIN_22		PORT_1	// P17
#define DIGITAL_PIN_23		PORT_5	// P51
#define DIGITAL_PIN_24		PORT_5	// P50

/* Bit mask of digital pin define. */
#define DIGITAL_PIN_MASK_0	0x02	// P11
#define DIGITAL_PIN_MASK_1	0x04	// P12
#define DIGITAL_PIN_MASK_2	0x01	// P30
#define DIGITAL_PIN_MASK_3	0x40	// P16
#define DIGITAL_PIN_MASK_4	0x02	// P31
#define DIGITAL_PIN_MASK_5	0x20	// P15
#define DIGITAL_PIN_MASK_6	0x01	// P10
#define DIGITAL_PIN_MASK_7	0x01	// P00
#define DIGITAL_PIN_MASK_8	0x02	// P01
#define DIGITAL_PIN_MASK_9	0x08	// P13
#define DIGITAL_PIN_MASK_10	0x10	// P14
#define DIGITAL_PIN_MASK_11	0x04	// P72
#define DIGITAL_PIN_MASK_12	0x02	// P71
#define DIGITAL_PIN_MASK_13	0x01	// P70
#define DIGITAL_PIN_MASK_14	0x04	// P22
#define DIGITAL_PIN_MASK_15	0x08	// P23
#define DIGITAL_PIN_MASK_16	0x10	// P24
#define DIGITAL_PIN_MASK_17	0x20	// P25
#define DIGITAL_PIN_MASK_18	0x40	// P25
#define DIGITAL_PIN_MASK_19	0x80	// P27
#define DIGITAL_PIN_MASK_20	0x80	// P147
#define DIGITAL_PIN_MASK_21	0x01	// P120
#define DIGITAL_PIN_MASK_22	0x80	// P17
#define DIGITAL_PIN_MASK_23	0x02	// P51
#define DIGITAL_PIN_MASK_24	0x01	// P50

//#define PWM_PIN_0			0xE#	// Software PWM#
//#define PWM_PIN_1			0xE#	// Software PWM#
//#define PWM_PIN_2			0xE#	// Software PWM#
#define PWM_PIN_3			1		// TO1
//#define PWM_PIN_4			0xE#	// Software PWM#
#define PWM_PIN_5			2		// TO2
#define PWM_PIN_6			7		// TO7
//#define PWM_PIN_7			0xE#	// Software PWM#
//#define PWM_PIN_8			0xE#	// Software PWM#
#define PWM_PIN_9			4		// TO4
#define PWM_PIN_10			3		// TO3
#define PWM_PIN_11			0xE0	// Software PWM0
//#define PWM_PIN_12		0xE#	// Software PWM#
//#define PWM_PIN_13		0xE#	// Software PWM#
//#define PWM_PIN_14		0xE#	// Software PWM#
//#define PWM_PIN_15		0xE#	// Software PWM#
//#define PWM_PIN_16		0xE#	// Software PWM#
//#define PWM_PIN_17		0xE#	// Software PWM#
//#define PWM_PIN_18		0xE#	// Software PWM#
//#define PWM_PIN_19		0xE#	// Software PWM#
//#define PWM_PIN_20		0xE#	// Software PWM#
//#define PWM_PIN_21		0xE#	// Software PWM#
#define PWM_PIN_22			0xE1	// Software PWM1
#define PWM_PIN_23			0xE2	// Software PWM2
#define PWM_PIN_24			0xE3	// Software PWM3

#define SWPWM_PIN			0xE0

#define ANALOG_PIN_0		2		// ANI2
#define ANALOG_PIN_1		3		// ANI3
#define ANALOG_PIN_2		4		// ANI4
#define ANALOG_PIN_3		5		// ANI5
#define ANALOG_PIN_4		6		// ANI6
#define ANALOG_PIN_5		7		// ANI7
#define ANALOG_PIN_6		18		// ANI18
#define ANALOG_PIN_7		19		// ANI19

#define ANALOG_ADPC_OFFSET	4

#define SPI_CHANNEL 		21		// CSI21
#define SERVO_CHANNEL 		4		// TM04
#define I2C_CHANNEL 		0		// IICA0
#define UART_CHANNEL 		0		// UART0(Serial0)
#define UART1_CHANNEL       1       // UART1(Serial1)
#define UART2_CHANNEL       2       // UART2(Serial2)
#define HOOK_TIMER_CHANNEL  1

/***************************************************************************/
/*    RL78 Stick（RL78/G13 68ピン）のコンフィグレーション                  */
/***************************************************************************/
#elif defined(REL_GR_KURUMI_PROTOTYPE)

/* External interrupt define. */
#define EXTERNAL_INTERRUPT_0	3
#define EXTERNAL_INTERRUPT_1	4

/* Port of digital pin define. */
#define DIGITAL_PIN_0		PORT_1	// P14
#define DIGITAL_PIN_1		PORT_1	// P13
#define DIGITAL_PIN_2		PORT_3	// P30
#define DIGITAL_PIN_3		PORT_3	// P31
#define DIGITAL_PIN_4		PORT_4	// P43
#define DIGITAL_PIN_5		PORT_4	// P41
#define DIGITAL_PIN_6		PORT_4	// P42
#define DIGITAL_PIN_7		PORT_0	// P04
#define DIGITAL_PIN_8		PORT_0	// P05
#define DIGITAL_PIN_9		PORT_1	// P16
#define DIGITAL_PIN_10		PORT_1	// P17
#define DIGITAL_PIN_11		PORT_1	// P12
#define DIGITAL_PIN_12		PORT_1	// P11
#define DIGITAL_PIN_13		PORT_1	// P10
#define DIGITAL_PIN_14		PORT_7	// P75
#define DIGITAL_PIN_15		PORT_7	// P74
#define DIGITAL_PIN_16		PORT_7	// P73
#define DIGITAL_PIN_17		PORT_7	// P72
#define DIGITAL_PIN_18		PORT_7	// P71
#define DIGITAL_PIN_19		PORT_7	// P70

/* Bit mask of digital pin define. */
#define DIGITAL_PIN_MASK_0	0x10	// P14
#define DIGITAL_PIN_MASK_1	0x08	// P13
#define DIGITAL_PIN_MASK_2	0x01	// P30
#define DIGITAL_PIN_MASK_3	0x02	// P31
#define DIGITAL_PIN_MASK_4	0x08	// P43
#define DIGITAL_PIN_MASK_5	0x02	// P41
#define DIGITAL_PIN_MASK_6	0x04	// P42
#define DIGITAL_PIN_MASK_7	0x10	// P04
#define DIGITAL_PIN_MASK_8	0x20	// P05
#define DIGITAL_PIN_MASK_9	0x40	// P16
#define DIGITAL_PIN_MASK_10	0x80	// P17
#define DIGITAL_PIN_MASK_11	0x04	// P12
#define DIGITAL_PIN_MASK_12	0x02	// P11
#define DIGITAL_PIN_MASK_13	0x01	// P10
#define DIGITAL_PIN_MASK_14	0x20	// P75
#define DIGITAL_PIN_MASK_15	0x10	// P74
#define DIGITAL_PIN_MASK_16	0x08	// P73
#define DIGITAL_PIN_MASK_17	0x04	// P72
#define DIGITAL_PIN_MASK_18	0x02	// P71
#define DIGITAL_PIN_MASK_19	0x01	// P70

#define PWM_PIN_3			3		// TO3
#define PWM_PIN_5			7		// TO7
#define PWM_PIN_6			4		// TO4
#define PWM_PIN_9			1		// TO1
#define PWM_PIN_10			2		// TO2
//////////////////////////////////////////////
#define PWM_PIN_11			-1		// TO5
//////////////////////////////////////////////

#define ANALOG_PIN_0		0x01	// ANI1
#define ANALOG_PIN_1		0x02	// ANI2
#define ANALOG_PIN_2		0x03	// ANI3
#define ANALOG_PIN_3		0x04	// ANI4
#define ANALOG_PIN_4		0x05	// ANI5
#define ANALOG_PIN_5		0x06	// ANI6

#define ANALOG_ADPC_OFFSET	3

#define SPI_CHANNEL 		0		// CSI00
#define SERVO_CHANNEL 		1		// TM01
#define I2C_CHANNEL 		0		// IICA0
#define UART_CHANNEL 		2		// UART2

#endif


/* Timer clock setting. */
#define TIMER_CLOCK			(0x0050)	// CK0 = 32MHz, CK1 = 1MHz, CK2 = 16MHz, CK3 = 125kHz
#define CLK_SOURCE_XT1 1
#define CLK_SOURCE_FIL 2
#define RTC_CLK_SOURCE CLK_SOURCE_XT1
//#define RTC_CLK_SOURCE CLK_SOURCE_FIL

#if (RTC_CLK_SOURCE == CLK_SOURCE_XT1)
#define INTERVAL		(33 - 1)	//!< インターバルタイマ割り込み周期（1 / 32.768kHz x 33   =   1.0070 ms)
#elif (RTC_CLK_SOURCE == CLK_SOURCE_FIL)
#define INTERVAL		(15 - 1)	//!< インターバルタイマ割り込み周期（1 / 15kHz x 15    =   1 ms)
#endif

#define MICROSECONDS_PER_TIMER05_OVERFLOW 0xffff //1us * 2 ^ 16
#define MILLISECONDS_PER_TIMER05_OVERFLOW (MICROSECONDS_PER_TIMER05_OVERFLOW / 1000)
#define INTERVAL_MICRO_TDR	(MICROSECONDS_PER_TIMER05_OVERFLOW - 1)
#define WAIT_XT1_CLOCK	1850000
/* Interval Micro Timer setting. */
#define INTERVAL_MICRO_MODE       (0x8000)    // Select CK1, Using Interrupt Handler

/* PWM setting. */
//#define PWM_MASTER_CLOCK    (32000000)
#define PWM_MASTER_CLOCK    (1000000)
#if PWM_MASTER_CLOCK == 32000000
#define PWM_MASTER_MODE		(0x0801)    // CK0を選択
#define PWM_SLAVE_MODE		(0x0409)
#elif PWM_MASTER_CLOCK == 1000000
#define PWM_MASTER_MODE		(0x8801)    // CK1を選択
#define PWM_SLAVE_MODE		(0x8409)
#endif
#define PWM_DEFAULT_FREQ    (490)       // 490Hz
#define PWM_TDR00			(PWM_MASTER_CLOCK / PWM_DEFAULT_FREQ - 1)

/* Tone setting. */
#define TONE_TIMER			PWM_PIN_3
#define TONE_MODE			(0xC000)    // CK3を選択

/* Output clock setting. */
#define OUTPUT_CLOCK_CKx	(32000000)
#define OUTPUT_CLOCK_MODE 	(0x0000)    // CK0を選択
#define OUTPUT_CLOCK_MIN	(245)
#define OUTPUT_CLOCK_MAX	(16000000)


/* SPI register define. */
#if ( SPI_CHANNEL == 0 )
#define SPI_SAUxEN			SAU0EN
#define SPI_SPSx			SPS0.sps0
#define SPI_STx 			ST0.st0
#define SPI_SSx 			SS0.ss0
#define SPI_CHx 			0x0001
#define SPI_CSIMKxx 		CSIMK00
#define SPI_CSIIFxx 		CSIIF00
#define SPI_CSIPR1xx		CSIPR100
#define SPI_CSIPR0xx		CSIPR000
#define SPI_SIRxx			SIR00.sir00
#define SPI_SMRxx			SMR00.smr00
#define SPI_SCRxx			SCR00.scr00
#define SPI_SDRxx			SDR00.sdr00
#define SPI_SOx 			SO0.so0
#define SPI_SOEx			SOE0.soe0
#define SPI_SIOxx			SIO00.sio00

#elif ( SPI_CHANNEL == 21 )
#define SPI_SAUxEN			SAU1EN
#define SPI_SPSx			SPS1.sps1
#define SPI_STx 			ST1.st1
#define SPI_SSx 			SS1.ss1
#define SPI_CHx 			0x0002
#define SPI_CSIMKxx 		CSIMK21
#define SPI_CSIIFxx 		CSIIF21
#define SPI_CSIPR1xx		CSIPR121
#define SPI_CSIPR0xx		CSIPR021
#define SPI_SIRxx			SIR11.sir11
#define SPI_SMRxx			SMR11.smr11
#define SPI_SCRxx			SCR11.scr11
#define SPI_SDRxx			SDR11.sdr11
#define SPI_SOx 			SO1.so1
#define SPI_SOEx			SOE1.soe1
#define SPI_SIOxx			SIO21.sio21

#endif


#ifdef WORKAROUND_READ_MODIFY_WRITE

#define SFR_ADM0	0xfff30

#define SFR_RTCC0	0xfff9d
#define SFR_RTCC1	0xfff9d

#define SFR_IF2		0xfffd0
#define SFR_IF2L	0xfffd0
#define SFR_IF2H	0xfffd1
#define SFR_IF3L	0xfffd2
#define SFR_MK2		0xfffd4
#define SFR_MK2L	0xfffd4
#define SFR_MK2H	0xfffd5
#define SFR_MK3L	0xfffd6
#define SFR_PR02	0xfffd8
#define SFR_PR02L	0xfffd8
#define SFR_PR02H	0xfffd9
#define SFR_PR03L	0xfffda
#define SFR_PR12	0xfffdc
#define SFR_PR12L	0xfffdd
#define SFR_PR12H	0xfffdd
#define SFR_PR13L	0xfffde
#define SFR_IF0		0xfffe0
#define SFR_IF0L	0xfffe0
#define SFR_IF0H	0xfffe1
#define SFR_IF1		0xfffe2
#define SFR_IF1L	0xfffe2
#define SFR_IF1H	0xfffe3
#define SFR_MK0		0xfffe4
#define SFR_MK0L	0xfffe4
#define SFR_MK0H    0xfffe5
#define SFR_MK1		0xfffe6
#define SFR_MK1L	0xfffe6
#define SFR_MK1H	0xfffe7
#define SFR_PR00	0xfffe8
#define SFR_PR00L	0xfffe8
#define SFR_PR00H	0xfffe9
#define SFR_PR01	0xfffea
#define SFR_PR01L	0xfffea
#define SFR_PR01H	0xfffeb
#define SFR_PR10	0xfffec
#define SFR_PR10L	0xfffec
#define SFR_PR10H	0xfffed
#define SFR_PR11	0xfffee
#define SFR_PR11L	0xfffee
#define SFR_PR11H	0xfffef

/*
#define SFR_MDAL    0xffff0
#define SFR_MDAH    0xffff2
#define SFR_MDBL    0xffff4
#define SFR_MDBH    0xffff6
#define SFR_PMC        0xffffe
*/

#define SFR2_ADM2	0xf0010

#define SFR2_PMC12	0xf006c
#define SFR2_PMC14	0xf006e

#define SFR2_PER0	0xf00f0

#define	SFR2_SS0	0xf0122
#define	SFR2_ST0	0xf0124
#define	SFR2_SO0L	0xf0128
#define	SFR2_SO0H	0xf0129
#define	SFR2_SOE0	0xf012a
#define	SFR2_SS1	0xf0162
#define	SFR2_ST1	0xf0164
#define	SFR2_SO1L	0xf0168
#define	SFR2_SO1H	0xf0169
#define	SFR2_SOE1	0xf016a

#define	SFR2_TSR02L	0xf01a4
#define	SFR2_TSR03L	0xf01a6
#define	SFR2_TSR04L	0xf01a8
#define	SFR2_TSR05L	0xf01aa
#define	SFR2_TSR06L	0xf01ac
#define	SFR2_TSR07L	0xf01ae
#define	SFR2_TE0L	0xf01b0
#define	SFR2_TS0L	0xf01b2
#define	SFR2_TT0L	0xf01b4
#define	SFR2_TPS0	0xf01b6
#define	SFR2_TO0L	0xf01b8
#define	SFR2_TOE0L	0xf01ba
#define	SFR2_TOL0L	0xf01bc
#define	SFR2_TOM0L	0xf01be

#define SFR_BIT_RTCE		7

#define SFR_BIT_ADCE		0
#define SFR_BIT_ADCS		7

#define SFR2_BIT_TAU0EN		0
#define SFR2_BIT_TAU1EN		1
#define SFR2_BIT_SAU0EN		2
#define SFR2_BIT_SAU1EN		3
#define SFR2_BIT_IICA0EN	4
#define SFR2_BIT_ADCEN		5
#define SFR2_BIT_IICA1EN	6
#define SFR2_BIT_RTCEN		7


#if ( SPI_CHANNEL == 0 )
#define	SFR2_STx 			SFR2_ST0
#define	SFR2_SSx 			SFR2_SS0
#define	SFR_MKxx 			SFR_MK0H
#define	SFR_IFxx 			SFR_IF0H
#define	SFR_PR1xx			SFR_PR10H
#define	SFR_PR0xx			SFR_PR00H
#define	SFR2_SOxL 			SFR2_SO0L
#define	SFR2_SOxH			SFR2_SO0H
#define	SFR2_SOEx			SFR2_SOE0

#define	SFR2_BIT_SAUxEN		SFR2_BIT_SAU0EN
#define	SFR_BIT_CSIMKxx		5
#define	SFR_BIT_CSIIFxx		5
#define	SFR_BIT_CSIPR1xx	5
#define	SFR_BIT_CSIPR0xx	5
#define	SFR2_BIT_STx		0
#define	SFR2_BIT_SSx		0
#define	SFR2_BIT_SOx		0
#define	SFR2_BIT_SOEx		0

#elif ( SPI_CHANNEL == 21 )
#define	SFR2_STx 			SFR2_ST1
#define	SFR2_SSx 			SFR2_SS1
#define	SFR_MKxx 			SFR_MK0H
#define	SFR_IFxx 			SFR_IF0H
#define	SFR_PR1xx			SFR_PR10H
#define	SFR_PR0xx			SFR_PR00H
#define	SFR2_SOxL 			SFR2_SO1L
#define	SFR2_SOxH			SFR2_SO1H
#define	SFR2_SOEx			SFR2_SOE1

#define	SFR2_BIT_SAUxEN		SFR2_BIT_SAU1EN
#define	SFR_BIT_CSIMKxx		1
#define	SFR_BIT_CSIIFxx		1
#define	SFR_BIT_CSIPR1xx	1
#define	SFR_BIT_CSIPR0xx	1
#define	SFR2_BIT_STx		1
#define	SFR2_BIT_SSx		1
#define	SFR2_BIT_SOx		1
#define	SFR2_BIT_SOEx		1

#endif


#define CBI(sfr, bit) \
do { \
    __asm __volatile( \
        "        clr1    %u0.%u1 \n" \
        : \
        : "i"(sfr), "i"(bit) \
    ); \
} while (0)
#define SBI(sfr, bit) \
do { \
    __asm __volatile( \
        "        set1    %u0.%u1 \n" \
        : \
        : "i"(sfr), "i"(bit) \
    ); \
} while (0)

#define CBI2(sfr, bit) \
do { \
    __asm __volatile( \
        "        clr1    !%u0.%u1 \n" \
        : \
        : "i"(sfr), "i"(bit) \
    ); \
} while (0)
#define SBI2(sfr, bit) \
do { \
    __asm __volatile( \
        "        set1    !%u0.%u1 \n" \
        : \
        : "i"(sfr), "i"(bit) \
    ); \
} while (0)

#endif

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/


/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/


/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
#endif /* RLDUINO78_MCU_DEPEND_H */
