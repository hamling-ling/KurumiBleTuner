/***************************************************************************
 *
 * PURPOSE
 *   RLduino78 framework header file.
 *
 * TARGET DEVICE
 *   RL78/G13
 *
 * AUTHOR
 *   Renesas Solutions Corp.
 *
 * $Date:: 2013-04-17 13:11:55 +0900#$
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
 * @file  RLduino78.h
 * @brief  Arduino互換の機能を提供するRLduino78ライブラリのヘッダファイルです。
 */
#ifndef RLDUINO78_H
#define RLDUINO78_H
/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include <stdlib.h>
#include <math.h>
#include "Binary.h"
#ifdef __cplusplus
#include "HardwareSerial.h"
#endif


/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define RLDUINO78_VERSION 0x0113	//!< RLduino78ライブラリのバージョン情報

#define true			1
#define false			0

/* PI */
#define PI				3.1415926535897932384626433832795
#define HALF_PI			1.5707963267948966192313216916398
#define TWO_PI			6.283185307179586476925286766559
#define DEG_TO_RAD		0.017453292519943295769236907684886
#define RAD_TO_DEG		57.295779513082320876798154814105

/* Pin mode. */
#define HIGH			0x1
#define LOW				0x0

/* Pin direction mode. */
#define INPUT			0x0
#define OUTPUT			0x1
#define INPUT_PULLUP	0x2
#define INPUT_TTL	0x3
#define INPUT_TTL_PULLUP	0x4
#define OUTPUT_OPENDRAIN	0x5

/* Analog reference mode. */
#define DEFAULT			0x00
#define EXTERNAL		0x01
#define INTERNAL		0x02

#define LSBFIRST 0
#define MSBFIRST 1

/* Interrupt ege mode. */
#define CHANGE			0x01
#define FALLING			0x02
#define RISING			0x03

/* PWM defines. */
#define PWM_MIN			0
#define PWM_MAX			255

/* Power Management Mode. */
#define PM_NORMAL_MODE	0
#define PM_HALT_MODE	1
#define PM_STOP_MODE	2
#define PM_SNOOZE_MODE	3

/* Operation Clock Mode. */
#define CLK_HIGH_SPEED_MODE		0
#define CLK_LOW_SPEED_MODE		1

/* Temperature Mode. */
#define TEMP_MODE_CELSIUS		0
#define TEMP_MODE_FAHRENHEIT	1

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/
typedef unsigned char	uint8_t;
typedef signed char     int8_t;
typedef unsigned short	uint16_t;
typedef signed short    int16_t;
typedef unsigned long   uint32_t;
typedef signed long     int32_t;
typedef uint8_t			byte;
typedef unsigned int	word;
typedef void (*fInterruptFunc_t)(void);
typedef void (*fITInterruptFunc_t)(unsigned long u32timer_millis);
typedef uint8_t			boolean;


/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/
/* RLduino78 extension functions. */
uint16_t getVersion();
uint8_t getPowerManagementMode();
#ifdef __cplusplus
void setPowerManagementMode(uint8_t u8PowerManagementMode, uint16_t u16ADLL = 0, uint16_t u16ADUL = 1023);
#else
void setPowerManagementMode(uint8_t u8PowerManagementMode, uint16_t u16ADLL, uint16_t u16ADUL);
#endif
uint8_t getOperationClockMode();
void setOperationClockMode(uint8_t u8ClockMode);
void attachIntervalTimerHandler(void (*fFunction)(unsigned long u32Milles));
void detachIntervalTimerHandler();
void attachMicroIntervalTimerHandler(void (*fFunction)(void), uint16_t interval);
void detachMicroIntervalTimerHandler();
void attachCyclicHandler(uint8_t u8HandlerNumber, void (*fFunction)(unsigned long u32Milles), uint32_t u32CyclicTime);
void detachCyclicHandler(uint8_t u8HandlerNumber);
void outputClock(uint8_t u8Pin, uint32_t u32Frequency);
int getTemperature(uint8_t u8Mode);
void softwareReset(void);
uint8_t getResetFlag(void);


#ifdef __cplusplus
extern "C" {
#endif
/* Digital Input/Output functions. */
void pinMode(uint8_t u8Pin, uint8_t u8Mode);
void digitalWrite(uint8_t u8Pin, uint8_t u8Value);
int digitalRead(uint8_t u8Pin);

/* Analog Input functions. */
void analogReference(uint8_t u8Type);
int analogRead(uint8_t u8Pin);
void analogWrite(uint8_t u8Pin, int s16Value);
void analogWriteFrequency(uint32_t u32Hz);

/* Other Input/Output functions. */
void shiftOut(uint8_t u8DataPin, uint8_t u8ClockPin, uint8_t u8BitOrder, uint8_t u8Value);
void shiftOutEx(uint8_t u8DataPin, uint8_t u8ClockPin, uint8_t u8BitOrder, uint8_t u8Len, uint32_t u32Value);
uint8_t shiftIn(uint8_t u8DataPin, uint8_t u8ClockPin, uint8_t u8BitOrder);

/* Time functions */
unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long u32ms);
void delayMicroseconds(unsigned int s16us);

/* Mathematics functions. */
/** ************************************************************************
 * @addtogroup group6
 * 
 * @{
 ***************************************************************************/
/**
 * 2つの数値のうち、小さいほうの値を返します。 
 * @param[in] a 1つめ値
 * @param[in] b 2つめ値
 * @return 小さいほうの数値
 * @attention なし
 */
#define min(a,b)						((a)<(b)?(a):(b))

/**
 * 2つの数値のうち、大きいほうの値を返します。 
 * @param[in] a 1つめ値
 * @param[in] b 2つめ値
 * @return 大きいほうの数値
 * @attention なし
 */
#define max(a,b)						((a)>(b)?(a):(b))

/**
 * 数値を指定した範囲のなかに収めます。 
 * @param[in] amt  計算対象の値
 * @param[in] low  範囲の下限
 * @param[in] high 範囲の上限
 * @return 計算の対象値（amt）が範囲の下限（low）以上かつ上限（high)以下のときは対象値（amt）がそのまま返ります。
 * 対象値（amt）が範囲の下限（low）より小さいときは下限（low）、上限（high）より大きいときは上限（high）が返ります。
 * @attention なし
 */
#define constrain(amt,low,high)			((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

/**
 * 指定された値の絶対値を求めます。
 * @param[in] x 計算対象の値
 * @return 絶対値を返します。
 * @attention なし
 */
#define abs(x)							((x) > 0 ? (x) : -(x))

/**
 * 指定された浮動小数点値を最も近い整数値に丸めます。
 * @param[in] x 計算対象の値
 * @return 整数値に丸めた値を返します。
 * @attention なし
 */
#define round(x)						((x) >= 0 ? (long)((x) + 0.5) : (long)((x) - 0.5))

/**
 * 指定されたdegree値をradianに変換します。
 * @param[in] deg 計算対象の値
 * @return radianの値を返します。
 * @attention なし
 */
#define radians(deg)					((deg)*DEG_TO_RAD)

/**
 * 指定されたradian値をdegreeに変換します。
 * @param[in] rad  計算対象の値
 * @return degreeの値を返します。
 * @attention なし
 */
#define degrees(rad)					((rad)*RAD_TO_DEG)

/**
 * 指定された値の2乗を求めます。
 * @param[in] x  計算対象の値
 * @return xの2乗が返ります。
 * @attention なし
 */
#define sq(x)							((x)*(x))

long map(long s32Value, long s32fromLow, long s32fromHigh, long s32toLow, long s32toHigh);
/** @} */


/* Byte & Bit operation functions. */
/** ************************************************************************
 * @addtogroup group9
 * 
 * @{
 ***************************************************************************/
/**
 * 変数の下位1バイトを取り出します。
 * たとえば、lowByte(0xFF01)は1を返します。
 * @param[in] w あらゆる型の変数
 * @return byte
 * @attention なし
 */
#define lowByte(w)						((uint8_t) ((w) & 0xff))

/**
 * 変数の上位1バイトを取り出します(2バイトより大きな型に対しては、下位から2番目のバイトを取り出すことになります)。
 * たとえば、highByte(0xFF01)は255を返します。
 * @param[in] w あらゆる型の変数
 * @return byte
 * @attention なし
 */
#define highByte(w)						((uint8_t) ((w) >> 8))

/**
 * ある数から指定したビットを読み取ります。
 * @param[in] value 読み取る対象となる数
 * @param[in] bit 読み取るビットの位置。右端(LSB)から数えて何ビット目か
 * @return 0または1 
 * @attention なし
 */
#define bitRead(value, bit)				(((value) >> (bit)) & 0x01)

/**
 * ある数の指定したビットに書き込みます。 
 * @param[in] value    書き込む対象の数 
 * @param[in] bit      書き込むビットの位置。右端(LSB)から数えて何ビット目か 
 * @param[in] bitvalue 書き込むビット値(0または1) 
 * @return なし
 * @attention なし
 */
#define bitWrite(value, bit, bitvalue)	(bitvalue ? bitSet(value, bit) : bitClear(value, bit))

/**
 * ある数の指定したビットをセットします。ここでいうセットとは1にすることです。
 * @param[in] value 対象となる数 
 * @param[in] bit   セットするビットの位置。右端(LSB)から数えて何ビット目か
 * @return なし
 * @attention なし
 */
#define bitSet(value, bit)				((value) |= (1 << (bit)))

/**
 * ある数の指定したビットをクリアします。ここでいうクリアとは0にすることです。
 * @param[in] value 対象となる数 
 * @param[in] bit   クリアするビットの位置。右端(LSB)から数えて何ビット目か 
 * @return なし 
 * @attention なし
 */
#define bitClear(value, bit)			((value) &= ~(1 << (bit)))

/**
 * 指定したビットを1にしたときの値を計算します。bit(0)は1、bit(1)は2、bit(2)は4となります。
 * @param[in] b 値を知りたいビット 
 * @return ビットに対応する値数値を指定した範囲のなかに収めます。 
 * @attention なし
 */
#define bit(b)							(1UL << (b))
/** @} */

/* Interrupt operation functions. */
void attachInterrupt(uint8_t u8Interrupt, void (*fFunction)(void), int s16Mode);
void detachInterrupt(uint8_t u8Interrupt);

/** ************************************************************************
 * @addtogroup group11
 * 
 * @{
 ***************************************************************************/
#ifdef USE_RTOS
#define interrupts() 					portENABLE_INTERRUPTS()
#define noInterrupts() 					portDISABLE_INTERRUPTS()
#else
/**
 * noInterrupts()関数によって停止した割り込みを有効にします。
 * 割り込みはデフォルトで有効とされ、バックグラウンドで重要なタスクを処理します。
 * いくつかの機能は割り込みが無効の間は動作しません。たとえば、シリアル通信の
 * 受信データが無視されることがあります。割り込みはコードのタイミングを若干乱すので、
 * クリティカルなセクションでは無効にしたほうがいいかもしれません。 
 * @return なし
 * @attention なし
 */
#define interrupts() 					__asm __volatile("EI\n")

/**
 * 割り込みを無効にします。 interrupts()関数でまた有効にできます。
 * @return なし
 * @attention なし
 */
#define noInterrupts() 					__asm __volatile("DI\n")
#endif
/** @} */

/* Arduino framework functions. */
/**
 * setup()はボードの電源を入れたときやリセットしたときに、一度だけ実行されます。
 * 変数やピンモードの初期化、ライブラリの準備などに使ってください。
 * @return なし
 * @attention setup()は省略できません。
 */
void setup(void);

/**
 * setup()で初期値を設定したら、 loop()に実行したいプログラムを書きます。
 * そのプログラムによってボードの動きをコントロールします。
 * loopという名前のとおり、この部分は繰り返し実行されます。
 * @return なし
 * @attention loop()は省略できません。
 */
void loop(void);

#ifdef __cplusplus
}
#endif

/* Other Input/Output functions. */
#ifdef __cplusplus
unsigned long pulseIn(uint8_t u8Pin, uint8_t u8Value);
#endif
unsigned long pulseIn(uint8_t u8Pin, uint8_t u8Value, unsigned long u32Timeout);

/* Tone functions. */
#ifdef __cplusplus
void tone(uint8_t u8Pin, unsigned int u16Frequency);
#endif
void tone(uint8_t u8Pin, unsigned int u16Frequency, unsigned long u32Duration);
void noTone(uint8_t u8Pin);

/* Random numbers functions. */
void randomSeed(unsigned int u16Seed);
#ifdef __cplusplus
long random(long s32Max);
#endif
long random(long s32Min, long s32Max);

/* itoa() functions. */
void utoa(unsigned int value, char* buff, unsigned char base);
void itoa(int value, char* buff, unsigned char base);
void ltoa(long value, char* buff, unsigned char base);
void ultoa(unsigned long value, char* buff, unsigned char base);

/**
 * 割り込みが有効かを判定します。
 * @return 割り込みが有効だと1、無効だと0
 * @attention なし
 */
#define isInterrupts() (PSW.BIT.ie == 1)

/**
 * 割り込みが禁止かを判定します。
 * @return 割り込みが禁止だと1、有効だと0
 * @attention なし
 */
#define isNoInterrupts() (PSW.BIT.ie == 0)
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
#endif /* RLDUINO78_H */
