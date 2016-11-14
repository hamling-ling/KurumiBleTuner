/***************************************************************************
 *
 * PURPOSE
 *   RLduino78 framework basic function module file.
 *
 * TARGET DEVICE
 *   RL78/G13
 *
 * AUTHOR
 *   Renesas Solutions Corp.
 *
 * $Date:: 2013-03-26 16:46:45 +0900#$
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
 * @file  RLduino78_basic.cpp
 * @brief Arduino互換の機能を提供するRLduino78ライブラリです。
 */
/* Modified by Yuuki Okamiya : Dec 19, 2014*/
/* Modified by Yuuki Okamiya : Jan 16, 2015*/
/* Modified by Nozomu Fujita : Jul 27, 2015*/

/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include <stdlib.h>
#include "RLduino78_mcu_depend.h"
#include "RLduino78_timer.h"
#include "pintable.h"
#include "fastio.h"
#ifdef USE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif


/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define NOT_A_PIN  			0xFF	//!< 未定義ピン
#define NOT_A_PORT 			0xFF	//!< 未定義ポート

#define getPortModeRegisterAddr(port)		((volatile uint8_t*)(ADDR_PORT_MODE_REG + port))	//!< ポート・モード・レジスタ・ベースアドレス
#define getPortPullUpRegisterAddr(port)		((volatile uint8_t*)(ADDR_PORT_PULL_UP_REG + port))	//!< ポート・プルアップ・レジスタ・ベースアドレス
#define getPortInputModeRegisterAddr(port)	((volatile uint8_t*)(ADDR_PORT_IN_MODE_REG + port))	//!< ポート入力モード・レジスタ・ベースアドレス
#define getPortOutputModeRegisterAddr(port)	((volatile uint8_t*)(ADDR_PORT_OUT_MODE_REG + port))//!< ポート出力モード・レジスタ・ベースアドレス
#define getPortRegisterAddr(port)		((volatile uint8_t*)(ADDR_PORT_REG + port))			//!< ポート・レジスタ・ベースアドレス

#define ANALOG_PIN_START_NUMBER	(14)		//!< アナログピンの開始番号
#define EXTERNAL_NUM_INTERRUPTS	(2)			//!< 外部割込み数
#define TONE_DURATION_INFINITY	(0xFFFFFFFF)//!< トーンの無限出力時間
#define MAX_CYCLIC_HANDLER 		(8)			//!< 最大周期起動ハンドラ数
#define	ADS_TEMP_SENSOR			(0x80) 		//!< 内蔵温度センサのADS番号
#define	ADS_REF_VOLTAGE			(0x81) 		//!< 内部基準電圧のADS番号

#ifdef USE_RTOS
#define FUNC_MUTEX_LOCK		xSemaphoreTake(xFuncMutex, portMAX_DELAY)	//!< 関数用MUTEX LOCKマクロ
#define FUNC_MUTEX_UNLOCK	xSemaphoreGive(xFuncMutex)					//!< 関数用MUTEX UNLOCKマクロ
#else
#define FUNC_MUTEX_LOCK													//!< 関数用MUTEX LOCKマクロ
#define FUNC_MUTEX_UNLOCK												//!< 関数用MUTEX UNLOCKマクロ
#endif

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/
static void enterPowerManagementMode(unsigned long u32ms);
#ifdef USE_RTOS
static void _pinMode(uint8_t u8Pin, uint8_t u8Mode);
static void _digitalWrite(uint8_t u8Pin, uint8_t u8Value);
static int _digitalRead(uint8_t u8Pin);
#else
static inline void _pinMode(uint8_t u8Pin, uint8_t u8Mode) {pinMode(u8Pin, u8Mode);}
static inline void _digitalWrite(uint8_t u8Pin, uint8_t u8Value) {digitalWrite(u8Pin, u8Value);}
static inline int _digitalRead(uint8_t u8Pin) {return digitalRead(u8Pin);}
#endif
static int _analogRead(uint8_t u8ADS);
static void _startTAU0(uint16_t u16TimerClock);
static void _stopTAU0();
static void _startTimerChannel(uint8_t u8TimerChannel, uint16_t u16TimerMode, uint16_t u16Interval, bool bPWM, bool bInterrupt);
static void _modifyTimerPeriodic(uint8_t u8TimerChannel, uint16_t u16Interval);
static void _stopTimerChannel(uint8_t u8TimerChannel);
static void _turnOffPwmPin(uint8_t u8Pin);
#if defined(REL_GR_KURUMI)
static void _softwarePWM(void);
#endif

/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/
#ifndef USE_RTOS
extern volatile unsigned long g_u32timer_millis;		//!< インターバルタイマ変数
extern volatile unsigned long g_u32delay_timer;			//!< delay() 用タイマ変数
extern volatile unsigned long g_timer05_overflow_count;//

#endif
extern fITInterruptFunc_t	g_fITInterruptFunc;			//!< ユーザー定義インターバルタイマハンドラ
extern void (*INT_TM_HOOK)();


/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/
static bool g_u8AnalogWriteAvailableTable[NUM_DIGITAL_PINS] = {
	false, false, false, false,
	false, false, false, false,
	false, false, false, false,
	false, false, false, false,
	false, false, false, false,
#if defined(REL_GR_KURUMI)
	false, false, false, false,
	false,
#endif
};


const static uint8_t	g_au8AnalogPinTable[NUM_ANALOG_INPUTS] = {
	ANALOG_PIN_0, ANALOG_PIN_1, ANALOG_PIN_2, ANALOG_PIN_3,
	ANALOG_PIN_4, ANALOG_PIN_5,
#if defined(REL_GR_KURUMI)
	ANALOG_PIN_6, ANALOG_PIN_7,	
#endif
};

// 動作モード
static uint8_t g_u8PowerManagementMode = PM_NORMAL_MODE;
static uint8_t g_u8OperationClockMode = CLK_HIGH_SPEED_MODE;

// アナログ参照モード
static uint8_t g_u8AnalogReference = DEFAULT;
static uint8_t g_u8ADUL  = 0xFF;
static uint8_t g_u8ADLL  = 0x00;
static volatile boolean g_bAdcInterruptFlag = false;

// PWMの周期(TDR00の設定値)
static uint16_t g_u16TDR00 = PWM_TDR00;

// トーン関係
static uint8_t g_u8TonePin = NOT_A_PIN;
static uint8_t g_u8ToneToggle = 0;
static unsigned long g_u32ToneDuration = 0;
static unsigned long g_u32ToneInterruptCount = 0;

// 割り込み関数テーブル
static fInterruptFunc_t g_afInterruptFuncTable[EXTERNAL_NUM_INTERRUPTS] = {NULL, NULL};

// 周期起動ハンドラ関数テーブル
static struct {
	fITInterruptFunc_t afCyclicHandler;
	uint32_t au32CyclicTime;
	uint32_t au32RemainTime;
	uint32_t au32LastTime;
} g_CyclicHandlerTable[MAX_CYCLIC_HANDLER] = {
	{NULL, 0, 0, 0},
};

#if defined(REL_GR_KURUMI)
// SoftwarePWM関連
static uint8_t g_u8SwPwmTicksCount = 0;
typedef struct {
    uint8_t valid:1;
    uint8_t pin:7;
    uint8_t value;
    volatile uint8_t* port;
    uint8_t mask;
    uint8_t newValue;
} SwPwm;

static volatile SwPwm g_SwPwm[NUM_SWPWM_PINS] = {
    {0, 0, 0, 0, 0, 0},
};
#define SWPWM_MIN 1024
#endif

extern uint8_t g_u8ResetFlag;


#ifdef USE_RTOS
xSemaphoreHandle	xFuncMutex = NULL;
#endif


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/
/// @cond
/**
 * Arduinoフレームワークを初期化します。
 *
 * @return 初期化に成功した場合0を返却します。
 *
 * @attention なし
 ***************************************************************************/
int initArduinoFramework()
{
	int 	s16Result = 0;

#ifdef USE_RTOS
	// MUTEXの生成
	xFuncMutex = xSemaphoreCreateMutex();
	if (xFuncMutex == NULL) {
		s16Result = -1;
	}
#endif

	return s16Result;
}
/// @endcond


/** ************************************************************************
 * @defgroup group101 拡張関数
 * 
 * @{
 ***************************************************************************/
/** ************************************************************************
 * @defgroup group14 その他
 * 
 * @{
 ***************************************************************************/
/**
 * RLduino78のバージョン情報を取得します。
 *
 * @return バージョン情報を返却します。
 *
 * @attention なし
 ***************************************************************************/
uint16_t getVersion()
{
	return RLDUINO78_VERSION;
}
/** @} group14 その他 */


/** ************************************************************************
 * @defgroup group15 パワーマネージメント/クロック制御関数
 * 
 * @{
 ***************************************************************************/
/**
 * パワーマネージメントモードを取得します。
 *
 * 現在のパワーマネージメントモードを取得します。
 * - 通常モード：PM_NORMAL_MODE
 * - 省電力(HALT)モード：PM_HALT_MODE
 * - 省電力(STOP)モード：PM_STOP_MODE
 * - 省電力(SNOOZE)モード：PM_SNOOZE_MODE
 *
 * @return パワーマネージメントモードを返却します。
 *
 * @attention なし
 ***************************************************************************/
uint8_t getPowerManagementMode()
{
	return g_u8PowerManagementMode;
}


/**
 * パワーマネージメントモードを設定します。
 *
 * パワーマネージメントモードを設定します。
 * - 通常モード：PM_NORMAL_MODE
 * - 省電力(HALT)モード  ：PM_HALT_MODE
 * - 省電力(STOP)モード  ：PM_STOP_MODE
 * - 省電力(SNOOZE)モード：PM_SNOOZE_MODE
 *
 * パワーマネージメントモードに省電力(HALT/STOP/SNOOZE)モードを指定して delay()
 * 関数を呼び出すと、一時停止期間中はHALT/STOP命令によりスタンバイ状態になります。
 * また、パワーマネージメントモードに省電力(SNOOZE)モード指定して、 analogRead()
 * 関数を呼び出すと、SNOOZE状態になります。
 *
 * @param[in] u8PowerManagementMode パワーマネージメントモードを指定します。
 * @param[in] u16ADLL               A/D変換結果範囲の下限値を指定します。省略した場合デフォルト値（0）が設定されます。
 * @param[in] u16ADUL               A/D変換結果範囲の上限値を指定します。省略した場合デフォルト値（1023）が設定されます。
 *
 * @return なし
 *
 * @attention 
 * - 動作クロックの状態によっては、パワーマネージメントモードが変更されない
 *   場合があります。
 * - A/D変換結果下限値/上限値には0から1023の範囲を指定してください。
 *   なお、下限値/上限値の下位2bitは無視されます。
 ***************************************************************************/
void setPowerManagementMode(uint8_t u8PowerManagementMode, uint16_t u16ADLL, uint16_t u16ADUL)
{

	FUNC_MUTEX_LOCK;

	switch (u8PowerManagementMode) {
	case PM_NORMAL_MODE:
	case PM_HALT_MODE:
		g_u8ADLL = 0x00;
		g_u8ADUL = 0xFF;
		g_u8PowerManagementMode = u8PowerManagementMode;
		break;

	case PM_STOP_MODE:
		if (CLS == 0) {
			g_u8ADLL = 0x00;
			g_u8ADUL = 0xFF;
			g_u8PowerManagementMode = u8PowerManagementMode;
		}
		break;

	case PM_SNOOZE_MODE:
		if ((CLS == 0) && (MCS == 0)) { 
			if (u16ADLL > 1023) {
				u16ADLL = 1023;
			}
			if (u16ADUL > 1023) {
				u16ADUL = 1023;
			}
			if (u16ADLL > u16ADUL) {
				u16ADLL = 0x00;
				u16ADUL = 0xFF;
			} else {
				g_u8ADLL = (uint8_t)(u16ADLL >> 2);
				g_u8ADUL = (uint8_t)(u16ADUL >> 2);
			}
			g_u8PowerManagementMode = u8PowerManagementMode;
		}
		break;

	default:
		break;
	}

	FUNC_MUTEX_UNLOCK;
}


/**
 * 動作クロックモードを取得します。
 *
 * CPU/周辺ハードウェアの動作クロックモードを取得します。
 * - 高速動作モード(高速オンチップ・オシレータ)：CLK_HIGH_SPEED_MODE
 * - 低速動作モード(XT1発信回路)               ：CLK_LOW_SPEED_MODE
 *
 * @return 動作クロックモードを返却します。
 *
 * @attention なし
 ***************************************************************************/
uint8_t getOperationClockMode()
{
	return g_u8OperationClockMode;
}


/**
 * 動作クロックを設定します。
 *
 * CPU/周辺ハードウェアの動作クロックを設定します。
 * - 高速動作モード(高速オンチップ・オシレータ)：CLK_HIGH_SPEED_MODE
 * - 低速動作モード(XT1発信回路)               ：CLK_LOW_SPEED_MODE
 *
 * 高速動作モードを指定すると、CPU/周辺ハードウェアに供給するクロックに
 * メイン・システム・クロック（高速オンチップ・オシレータ）を設定します。
 * 低速動作モードを指定すると、CPU/周辺ハードウェアに供給するクロックに
 * サブシステム・クロック（XT1発信回路）を設定します。
 *
 * @param[in] u8ClockMode 動作クロックを指定します。
 *
 * @return なし
 *
 * @attention
 * - パワーマネージメントモードが省電力(STOP/SNOOZE)の場合は低速動作モード
 *   を設定することができません。
 * - GR-KURUMIの場合、高速動作モード（CLK_HIGH_SPEED_MODE）を指定すると
 *   32 MHzで動作し、低速動作モード（CLK_LOW_SPEED_MODE）を指定すると 32.768 
 *   kHzで動作します。
 ***************************************************************************/
void setOperationClockMode(uint8_t u8ClockMode)
{
	FUNC_MUTEX_LOCK;

	if (u8ClockMode == CLK_HIGH_SPEED_MODE) {
		// 動作クロックの変更
		setup_main_system_clock();
		g_u8OperationClockMode = CLK_HIGH_SPEED_MODE;
	} else if (u8ClockMode == CLK_LOW_SPEED_MODE) {
		if ((g_u8PowerManagementMode != PM_STOP_MODE) &&
			(g_u8PowerManagementMode != PM_SNOOZE_MODE)) {
			// 動作クロックの変更
			setup_sub_system_clock();
			g_u8OperationClockMode = CLK_LOW_SPEED_MODE;
		}
	}

	FUNC_MUTEX_UNLOCK;
}
/** @} group15 パワーマネージメント/クロック制御関数 */


/** ************************************************************************
 * @defgroup group16 割込みハンドラ/周期起動関数
 * 
 * @{
 ***************************************************************************/
/**
 * インターバル・タイマ割り込みハンドラ内から実行するコールバック関数を登録します。
 *
 * コールバック関数を登録すると1[ms]のインターバル・タイマ割り込み毎に登録した
 * コールバック関数が呼び出されます。また、コールバック関数呼び出し時にはシステム
 * 開始からの時間（ms）が引数として渡されます。
 *
 * @param[in] fFunction インターバル・タイマ割り込み時に実行するハンドラを指定します。
 *
 * @return なし
 *
 * @attention
 * - コールバック関数内では時間のかかる処理は行わないでください。
 * - コールバック関数内では以下の関数が呼び出し可能です。
 *
 * pinMode()、 digitalWrite()、 digitalRead()、 millis()、 micros()、 delayMicroseconds()、
 * min()、 max()、 constrain()、 map()、 lowByte()、 highByte()、 bitRead()、 bitWrite()、
 * bitSet()、 bitClear()、 bit()、 randomSeed()、 random()
 * - pinMode()関数と digitalWrite()関数は、 loop()関数内とコールバック関数内で同じピン
 * 番号を指定すると誤動作する可能性があります。
 ***************************************************************************/
void attachIntervalTimerHandler(void (*fFunction)(unsigned long u32Milles))
{
	FUNC_MUTEX_LOCK;

	g_fITInterruptFunc = fFunction;

	FUNC_MUTEX_UNLOCK;
}


/**
 * インターバル・タイマ割り込みハンドラ内から実行するコールバック関数の登録を解除します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void detachIntervalTimerHandler()
{
	FUNC_MUTEX_LOCK;

	g_fITInterruptFunc = NULL;

	FUNC_MUTEX_UNLOCK;
}

/**
 * インターバル・タイマ割り込みハンドラ内から実行するコールバック関数を登録します。
 *
 * コールバック関数を登録すると引数[us]で指定したインターバル・タイマ割り込み毎に登録した
 * コールバック関数が呼び出されます。
 *
 * @param[in] fFunction インターバル・タイマ割り込み時に実行するハンドラを指定します。
 * @param[in] interval  インターバルの時間を指定します。[us]
 *
 * @return なし
 *
 * @attention
 * - コールバック関数内では時間のかかる処理は行わないでください。
 * - コールバック関数内では以下の関数が呼び出し可能です。
 * - コールバック関数を登録するとtone()が使用できなくなります。（デフォルト）
 * - tone()から変更する場合は、HOOK_TIMER_CHANNELを変更してください。
 *
 * pinMode()、 digitalWrite()、 digitalRead()、 millis()、 micros()、 delayMicroseconds()、
 * min()、 max()、 constrain()、 map()、 lowByte()、 highByte()、 bitRead()、 bitWrite()、
 * bitSet()、 bitClear()、 bit()、 randomSeed()、 random()
 * - pinMode()関数と digitalWrite()関数は、 loop()関数内とコールバック関数内で同じピン
 * 番号を指定すると誤動作する可能性があります。
 ***************************************************************************/
void attachMicroIntervalTimerHandler(void (*fFunction)(void), uint16_t interval)
{
	FUNC_MUTEX_LOCK;

	_startTimerChannel( HOOK_TIMER_CHANNEL, INTERVAL_MICRO_MODE, interval, false, true );
	INT_TM_HOOK  = fFunction;

	FUNC_MUTEX_UNLOCK;
}


/**
 * インターバル・タイマ割り込みハンドラ内から実行するコールバック関数の登録を解除します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void detachMicroIntervalTimerHandler()
{
	FUNC_MUTEX_LOCK;

	_stopTimerChannel(SW_PWM_TIMER);
	INT_TM_HOOK  = NULL;

	FUNC_MUTEX_UNLOCK;
}

/**
 * 周期起動コールバック関数を登録します。
 *
 * 登録したコールバック関数は、u32CyclicTimeで指定した周期間隔[ms]で呼び出されます。
 * また、コールバック関数呼び出し時にはシステム開始からの時間(ms)が引数として渡されます。
 *
 * @param[in] u8HandlerNumber 周期起動コールバック関数の識別番号(0~7)を指定します。
 * @param[in] fFunction       インターバル・タイマ割り込み時に実行するハンドラを指定します。
 * @param[in] u32CyclicTime   周期起動する間隔[ms]を指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void attachCyclicHandler(uint8_t u8HandlerNumber, void (*fFunction)(unsigned long u32Milles), uint32_t u32CyclicTime)
{
	FUNC_MUTEX_LOCK;

	if (u8HandlerNumber < MAX_CYCLIC_HANDLER) {
		detachCyclicHandler(u8HandlerNumber);
		g_CyclicHandlerTable[u8HandlerNumber].au32CyclicTime  = u32CyclicTime;
		g_CyclicHandlerTable[u8HandlerNumber].au32RemainTime = u32CyclicTime;
		g_CyclicHandlerTable[u8HandlerNumber].au32LastTime = millis();
		g_CyclicHandlerTable[u8HandlerNumber].afCyclicHandler = fFunction;
	}

	FUNC_MUTEX_UNLOCK;
}


/**
 * 周期起動コールバック関数の登録を解除します。
 *
 * @param[in] u8HandlerNumber 周期起動コールバック関数の識別番号( 0 ～ 7 )を指定します。
 *
 * @return なし
 *
 * @attention
 ***************************************************************************/
void detachCyclicHandler(uint8_t u8HandlerNumber)
{
	FUNC_MUTEX_LOCK;

	if (u8HandlerNumber < MAX_CYCLIC_HANDLER) {
		g_CyclicHandlerTable[u8HandlerNumber].afCyclicHandler = NULL;
		g_CyclicHandlerTable[u8HandlerNumber].au32CyclicTime  = 0;
		g_CyclicHandlerTable[u8HandlerNumber].au32RemainTime = 0;
		g_CyclicHandlerTable[u8HandlerNumber].au32LastTime = 0;
	}

	FUNC_MUTEX_UNLOCK;
}


/// @cond
/**
 * 周期起動コールバック関数を起動します。
 *
 * @return なし
 *
 * @attention
 ***************************************************************************/
void execCyclicHandler()
{
	int i;

	for (i = 0; i < MAX_CYCLIC_HANDLER; i++) {
		if (g_CyclicHandlerTable[i].afCyclicHandler != NULL) {
			unsigned long currentTime = millis();
			unsigned long elapsedTime = currentTime - g_CyclicHandlerTable[i].au32LastTime;
			g_CyclicHandlerTable[i].au32LastTime = currentTime;
			bool exec = g_CyclicHandlerTable[i].au32RemainTime <= elapsedTime;
			g_CyclicHandlerTable[i].au32RemainTime -= elapsedTime;
			if (exec) {
				g_CyclicHandlerTable[i].au32RemainTime += g_CyclicHandlerTable[i].au32CyclicTime;
				g_CyclicHandlerTable[i].afCyclicHandler(currentTime);
			}
		}
	}
}
/// @endcond
/** @} group16 割込みハンドラ/周期起動関数 */


/** ************************************************************************
 * @addtogroup group14
 * 
 * @{
 ***************************************************************************/
/**
 * 指定したピンからクロックを出力します。
 *
 * 他のデバイスに任意のクロックを供給することができます。例えば、Smart Analogの
 * フィルタ（LPF/HPF）の外部クロック入力に指定したピンを接続して任意のカットオフ
 * 周波数を得ることができます。 
 *
 * @param[in] u8Pin        ピン番号を指定します。
 * @param[in] u32Frequency 出力するクロック（ 244 Hz ～ 32 MHz）を指定します。
 *                         0を指定した場合はクロックの出力を停止します。
 *
 * @return なし
 *
 * @attention ピン番号にはデジタルピンのD3、D5、D6、D9、D10を指定してください。
 ***************************************************************************/
void outputClock(uint8_t u8Pin, uint32_t u32Frequency)
{
	uint8_t u8Timer;
	uint16_t u16Interval;

	FUNC_MUTEX_LOCK;

	if ((u8Pin < NUM_DIGITAL_PINS) &&
		((u32Frequency == 0) ||
		((OUTPUT_CLOCK_MIN <= u32Frequency) && (u32Frequency <= OUTPUT_CLOCK_MAX)))) {
		u8Timer = getPinTable(u8Pin)->timer;
		if (u8Timer != SWPWM_PIN) {
			// 出力モードに設定
			_pinMode(u8Pin, OUTPUT);

			if (u32Frequency != 0) {
				// タイマーアレイユニットの開始
				_startTAU0(TIMER_CLOCK);

				// タイマーの開始
				u16Interval = (uint16_t)(OUTPUT_CLOCK_CKx / (u32Frequency * 2) - 1);
				_startTimerChannel(u8Timer, OUTPUT_CLOCK_MODE, u16Interval, false, false);
			}
			else {
				// タイマーの停止
				_stopTimerChannel(u8Timer);

				// タイマーアレイユニットの停止
				_stopTAU0();

			}
		}
	}

	FUNC_MUTEX_UNLOCK;
}


/**
 * MCUに内蔵されている温度センサから温度（摂氏/華氏）を取得します。
 *
 * @param[in] u8Mode 摂氏/華氏を指定します。
 *			@arg TEMP_MODE_CELSIUS    ： 摂氏
 *			@arg TEMP_MODE_FAHRENHEIT ： 華氏
  *
 * @return 温度を返却します。
 *
 ***************************************************************************/
int getTemperature(uint8_t u8Mode)
{
#if 0
	int s16Result1, s16Result2;
	float fResult;

	FUNC_MUTEX_LOCK;
	
	s16Result1 = _analogRead(ADS_TEMP_SENSOR);
	s16Result2 = _analogRead(ADS_REF_VOLTAGE);

	fResult = (1450 * (float)s16Result1 / (float)s16Result2 - 1050) / -3.6 + 25;
	if (u8Mode == TEMP_MODE_FAHRENHEIT) {
		// 摂氏を華氏へ変換
		fResult = 1.8 * fResult + 32;
	}

	FUNC_MUTEX_UNLOCK;

	return (int)fResult;
#else
	int s16Result1, s16Result2;
	long s32Temp;
	int s16Result;

	FUNC_MUTEX_LOCK;

	s16Result1 = _analogRead(ADS_TEMP_SENSOR);
	s16Result2 = _analogRead(ADS_REF_VOLTAGE);
	if (s16Result2 == 0) {
		s16Result2 = 1;
	}

	volatile long n14500L = 14500L;
	s32Temp = n14500L * s16Result1 / s16Result2 - 10500L;
	if (u8Mode == TEMP_MODE_FAHRENHEIT) {
		s16Result = s32Temp / -20;
		s16Result += 77;
	} else {
		s16Result = s32Temp / -36;
		s16Result += 25;
	}

	FUNC_MUTEX_UNLOCK;

	return s16Result;
#endif
}
/** @} group14 */
/** @} group101 拡張関数 */


/** ************************************************************************
 * @defgroup group100 Arduino互換関数
 * 
 * @{
 ***************************************************************************/
/** ************************************************************************
 * @defgroup group2 デジタル入出力関数
 * 
 * @{
 ***************************************************************************/
/**
 * ピンの動作を入力か出力に設定します。
 *
 * INPUT_PULLUPを指定することで、内部プルアップ抵抗を有効にできます。
 * INPUTを指定すると、内部プルアップは無効となります。
 *
 * @param[in] u8Pin  設定したいピン番号を指定します。
 * @param[in] u8Mode ピンの動作モード（INPUT、OUTPUT、INPUT_PULLUP）を指定します。
 *
 * @return なし
 *
 * @attention
 * - アナログ入力ピン（A0～A7）はデジタルピンとしても使えます。アナログ入力ピン0が
 * デジタルピン14、アナログ入力ピン7がデジタルピン21に対応します。
 * - アナログ入力ピン（A0～A5）には内部プルアップ抵抗が無いためINPUT_PULLUPを指定
 * しないでください。
 ***************************************************************************/
#ifdef USE_RTOS
void pinMode(uint8_t u8Pin, uint8_t u8Mode)
{
	FUNC_MUTEX_LOCK;

	_pinMode(u8Pin, u8Mode);

	FUNC_MUTEX_UNLOCK;
}
#endif


/**
 * HIGHまたはLOWを、指定したピンに出力します。
 *
 * 指定したピンが pinMode()関数でOUTPUTに設定されている場合は、次の電圧にセッ
 * トされます。
 * - HIGH = 5V（3.3Vのボードでは3.3V）
 * - LOW  = 0V (GND) 
 *
 * 指定したピンがINPUTに設定されている場合は、HIGHを指定すると内部プルアップ
 * 抵抗が有効になります。LOWを指定すると内部プルアップは無効になります。
 *
 * @param[in] u8Pin   ピン番号を指定します。
 * @param[in] u8Value HIGHかLOWを指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
#ifdef USE_RTOS
void digitalWrite(uint8_t u8Pin, uint8_t u8Value)
{
	FUNC_MUTEX_LOCK;
	
	_digitalWrite(u8Pin, u8Value);

	FUNC_MUTEX_UNLOCK;
}
#endif


/**
 * 指定したピンの値を読み取ります。
 *
 * その結果はHIGHまたはLOWとなります。
 *
 * @param[in] u8Pin ピン番号を指定します。
 *
 * @return HIGHまたはLOWを返却します。
 *
 * @attention
 * なにも接続していないピンを読み取ると、HIGHとLOWがランダムに現れることがあ
 * ります。
 ***************************************************************************/
#ifdef USE_RTOS
int digitalRead(uint8_t u8Pin)
{
	int s16Value;

	FUNC_MUTEX_LOCK;

	s16Value = _digitalRead(u8Pin);

	FUNC_MUTEX_UNLOCK;

	return s16Value;
}
#endif
/** @} */


/** ************************************************************************
 * @defgroup group3 アナログ入出力関数
 * 
 * @{
 ***************************************************************************/
/**
 * アナログ入力で使われる基準電圧を設定します。
 *
 * analogRead()関数は入力が基準電圧と同じとき1023を返します。
 *
 * @param[in] u8Type 次のうちの１つを指定します。
 *                   @arg DEFAULT:  電源電圧(5V)が基準電圧となります。
 *                   @arg INTERNAL: 内蔵基準電圧を用います。RL78/G13では1.45Vです。
 *                   @arg EXTERNAL: AREFPピンに供給される電圧(0V～5V)を基準電圧とします。
 *
 * @return なし
 *
 * @attention
 * - 外部基準電圧を0V未満あるいは5V(電源電圧)より高い電圧に設定してはいけませ
 * ん。AREFPピンに外部基準電圧源を接続した場合、 analogRead()関数を実行する前に、
 * 必ずanalogReference(EXTERNAL)を実行しましょう。デフォルトの設定のまま外部
 * 基準電圧源を使用すると、チップ内部で短絡(ショート)が生じ、ボードが損傷する
 * かもしれません。
 * 抵抗器を介して外部基準電圧源をAREFPピンに接続する方法があります。これによ
 * り内部と外部の基準電圧源を切り替えながら使えるようになります。ただし、AREFP
 * ピンの内部抵抗の影響で基準電圧が変化する点に注意してください。
 * - GR-KURUMIでEXTERNALを指定した場合、基準電圧には電源電圧（Vdd）が使用されます。
 ***************************************************************************/
void analogReference(uint8_t u8Type)
{
	FUNC_MUTEX_LOCK;

	g_u8AnalogReference = u8Type;

	FUNC_MUTEX_UNLOCK;
}


/**
 * 指定したアナログピンから値を読み取ります。
 *
 * 8チャネルの10ビットA/Dコンバータを搭載しています。(A/Dはanalog to digital
 * の略)これにより、0から analogReference()関数で設定された基準電圧（デフォルト5ボルト）
 * の入力電圧を0から1023の数値に変換することができます。基準電圧が5ボルトの場合、
 * 分解能は1単位あたり4.9mVとなります。
 *
 * setPowerManagementMode()関数を用いてパワーマネージメントモードに省電力(SNOOZE)
 * モードを指定した場合、 analogRead()関数を呼び出すと、 setPowerManagementMode()
 * 関数で指定したA/D変換検結果検出範囲（u16ADLL ≦ A/D変換結果 ≦ u16ADUL）のA/D
 * 変換結果が得られるまでSTOP命令によりスタンバイ状態（SNOOZE状態）になり、A/D変
 * 換結果が範囲内になったときにスタンバイ状態が解除され analogRead()関数からリタ
 * ーンします。
 *
 * @param[in] u8Pin 読みたいピン番号を指定します。
 *
 * @return 0から1023までの整数値を返却します。
 *
 * @attention
 * 何も接続されていないピンに対して analogRead()関数を実行すると、不安定に変動
 * する値が得られます。これには様々な要因が関係していて、手を近づけるだけで
 * も値が変化します。
 * ピン番号には0～7を指定してください。なお、アナログピンのマクロ定義（A0～A7）
 * を使用することも可能です。
 ***************************************************************************/
int analogRead(uint8_t u8Pin)
{
	int	s16Result = 0;

	FUNC_MUTEX_LOCK;

	// ピン番号がANALOG_PIN_START_NUMBER（GR-KURUMIの場合は14）以上の場合0～に割り当てなおす。
	if (u8Pin >= ANALOG_PIN_START_NUMBER) {
		u8Pin -= ANALOG_PIN_START_NUMBER;
	}

	if (u8Pin < NUM_ANALOG_INPUTS) {
		// ピンモードをAnalogモードに変更
		switch (g_au8AnalogPinTable[u8Pin]) {
		case 18:
			PMC14.pmc14 |= 0x80;// P147をアナログポートに設定
			break;
			
		case 19:
			PMC12.pmc12 |= 0x01;// P120をアナログポートに設定
			break;

		default:
			uint8_t oldadpc = ADPC.adpc;
			uint8_t newadpc = (u8Pin - 14) + ANALOG_ADPC_OFFSET - 1;
			if ( newadpc > oldadpc ) {
				ADPC.adpc = newadpc;
			}
			break;
		}
		// pinModeの指定はデジタルピンの番号で行う
		_pinMode(u8Pin + ANALOG_PIN_START_NUMBER, INPUT);	// 入力モードに設定

		// アナログ値の読み込み
		s16Result = _analogRead(g_au8AnalogPinTable[u8Pin]);
	}

	FUNC_MUTEX_UNLOCK;

	return s16Result;
}


/**
 * 指定したピンからアナログ値(PWM波)を出力します。
 *
 * LEDの明るさを変えたいときや、モータの回転スピードを調整したいときに使用します。
 * analogWrite()関数が実行されると、次に analogWrite()関数、 digitalRead()関数、
 * digitalWrite()関数がそのピンに対して使用されるまで、安定した矩形波が出力さ
 * れます。PWM信号の周波数は約490Hzです。
 *
 * デジタルピンのD3、D5、D6、D9、D10でこの機能が使えます。 analogWrite()
 * 関数の前に pinMode()関数を呼び出して出力に設定する必要はありません。
 *
 * @param[in] u8Pin   出力に使うピン番号を指定します。
 * @param[in] s16Value デューティ比（0から255）を指定します。
 *
 * @return なし
 *
 * @attention
 * - s16Valueに0を指定すると、0Vの電圧が出力され、255を指定すると5Vが出力され
 *   ます。ただし、これは電源電圧が5ボルトの場合で、3.3Vの電源を使用するボード
 *   では3.3Vが出力されます。つまり、出力電圧の最大値は電源電圧と同じです。
 * - GR-KURUMIではD3、D5、D6、D9、D10の他のデジタルピン
 *   でもこの機能が使えます。また、D3、D5、D6、D9、D10のピンについてはハード
 *   ウェアでPWMを実現しています。
 * - 他のピンについてはソフトウェアでPWMを実現しています。
 *   ソフトウェアPWMで実現しているPWMの周波数はハードウェアPWMの周波数と異なり
 *   125Hzとなります。
 ***************************************************************************/
void analogWrite(uint8_t u8Pin, int s16Value)
{
	uint8_t u8Timer;
	unsigned short u16Duty;

	FUNC_MUTEX_LOCK;

	if (u8Pin < NUM_DIGITAL_PINS) {
		{
			s16Value = min(max(s16Value, PWM_MIN), PWM_MAX);
			u8Timer = getPinTable(u8Pin)->timer;
			if (u8Timer == SWPWM_PIN) {
				///////////////////////
				// Software PWM対応ピンの場合
				///////////////////////
#if defined(REL_GR_KURUMI)
				_startTAU0(TIMER_CLOCK);
				if (!g_u8AnalogWriteAvailableTable[u8Pin]) {
					_pinMode(u8Pin, OUTPUT);		// 初期時のみ出力モードを設定
					g_u8AnalogWriteAvailableTable[u8Pin] = true;
				}
				int i;
				int j = NUM_SWPWM_PINS;
				for (i = 0; i < NUM_SWPWM_PINS; i++) {
					if (!g_SwPwm[i].valid) {
						if (j >= NUM_SWPWM_PINS) {
							j = i;
						}
					} else if (g_SwPwm[i].pin == u8Pin) {
						break;
					}
				}
				if (i >= NUM_SWPWM_PINS) {
					i = j;
				}
				if (i < NUM_SWPWM_PINS) {
					if (g_SwPwm[i].valid && g_SwPwm[i].pin == u8Pin) {
						g_SwPwm[i].newValue = s16Value;
					} else {
						g_SwPwm[i].valid = false;
						g_SwPwm[i].pin = u8Pin;
						g_SwPwm[i].value = g_SwPwm[i].newValue = s16Value;
						g_SwPwm[i].port = getPinTable(u8Pin)->portRegisterAddr;
						g_SwPwm[i].mask = getPinTable(u8Pin)->mask;
						g_SwPwm[i].valid = true;
					}
					if (!(TE0.te0 & 0x0040)) { // No pin uses Software PWM
						_startTimerChannel( SW_PWM_TIMER, 0x0001, SWPWM_MIN, false, true );
					}
				} else {
					_digitalWrite(u8Pin, s16Value >= ((PWM_MIN + PWM_MAX + 1) / 2) ? HIGH : LOW);
				}

#endif
			} else {
				///////////////////////
				// PWM対応ピンの場合
				///////////////////////
				_startTAU0(TIMER_CLOCK);

				if (!(TE0.te0 & 0x0001)) {
					// Masterチャネルの設定
					TT0.tt0     |= 0x0001;			// タイマ停止
					TMR00.tmr00  = PWM_MASTER_MODE;	// 動作モードの設定
					TDR00.tdr00  = g_u16TDR00;		// PWM出力の周期の設定
					TO0.to0     &= ~0x0001;			// タイマ出力の設定
					TOE0.toe0   &= ~0x0001;			// タイマ出力許可の設定
					// マスタチャネルのタイマ動作許可
					TS0.ts0   |= 0x00001;
				}

				u16Duty = (unsigned short)(((unsigned long)s16Value * (g_u16TDR00 + 1)) / PWM_MAX);
				if (!g_u8AnalogWriteAvailableTable[u8Pin]) {
					_pinMode(u8Pin, OUTPUT);			// 出力モードに設定
					_digitalWrite(u8Pin, LOW);

					// Slaveチャネルの設定
					_startTimerChannel(u8Timer, PWM_SLAVE_MODE, u16Duty, true, false);
					g_u8AnalogWriteAvailableTable[u8Pin] = true;

				} else {
					_modifyTimerPeriodic(u8Timer, u16Duty);
				}
			}
		}
	}

	FUNC_MUTEX_UNLOCK;
}

/**
 * アナログ出力(PWM)の周波数を設定します。
 *
 * @param[in] u32Hz アナログ出力(PWM)の周波数(単位は[Hz])を指定します。
 *
 * @return なし
 *
 * @attention
 * - 全てのアナログ出力(PWM)が停止している状態で、本関数を実行してください。
 * - GR-KURUMIでは、PWMの周波数を変更できるのはD3、D5、D6、D9、D10のピンのみです。
 * - 他のピンについてはPWMの周波数は125Hz固定です。
 ***************************************************************************/
void analogWriteFrequency(uint32_t u32Hz)
{
	// PWM出力パルス周期設定
	//   パルス周期 = (TDR00の設定値+1) x カウント・クロック周期
	//   例）パルス周期が2[ms]の場合
	//       2[ms] = 1/32[MHz] x (TDR00の設定値 + 1)
	//       TDR00の設定値 = 63999
	if (u32Hz > PWM_MASTER_CLOCK) {
		g_u16TDR00 = 0x0000;
	}
	else {
		g_u16TDR00 = (PWM_MASTER_CLOCK / u32Hz) - 1;
	}
}
/** @} */


/** ************************************************************************
 * @defgroup group4 その他の入出力関数
 * 
 * @{
 ***************************************************************************/
/**
 * 1バイト分のデータを1ビットずつ「シフトアウト」します。
 *
 * 最上位ビット(MSB)と最下位ビット(LSB)のどちらからもスタートできます。各ビット
 * はまずu8DataPinに出力され、その後u8ClockPinが反転して、そのビットが有効になっ
 * たことが示されます。
 * この機能はソフトウエアで実現されています。より高速な動作が必要な場合はハード
 * ウエアで実現されているSPIライブラリを検討してください。
 *
 * @param[in] u8DataPin  各ビットを出力するピン番号を指定します。
 * @param[in] u8ClockPin クロックを出力するピン番号を指定します。u8DataPinに正し
                         い値がセットされたら、このピンが1回反転します
 * @param[in] u8BitOrder MSBFIRSTまたはLSBFIRSTを指定します。
 * @param[in] u8Value    送信したいデータ (byte) を指定します。
 *
 * @return なし
 *
 * @attention
 * u8DataPinとu8ClockPinは、あらかじめ pinMode()関数によって出力(OUTPUT)に設定され
 * ている必要があります。
 ***************************************************************************/
void shiftOut(uint8_t u8DataPin, uint8_t u8ClockPin, uint8_t u8BitOrder, uint8_t u8Value)
{
	int i;

	FUNC_MUTEX_LOCK;

	for (i = 0; i < 8; i++) {
		if (u8BitOrder == LSBFIRST) {
			_digitalWrite(u8DataPin, !!(u8Value & (1 << i)));
		}
		else {
			_digitalWrite(u8DataPin, !!(u8Value & (1 << (7 - i))));
		}
		_digitalWrite(u8ClockPin, HIGH);
		_digitalWrite(u8ClockPin, LOW);		
	}

	FUNC_MUTEX_UNLOCK;
}


/**
 * 複数バイト分のデータを1ビットずつ「シフトアウト」します。
 *
 * shiftOut()関数の拡張で32ビットまで1ビット単位で指定できます。最上位ビット(MSB)
 * と最下位ビット(LSB)のどちらからもスタートできます。各ビットはまずu8DataPinに出
 * 力され、その後u8ClockPinが反転して、そのビットが有効になったことが示されます。
 * この機能はソフトウエアで実現されています。より高速な動作が必要な場合はハード
 * ウエアで実現されているSPIライブラリを検討してください。
 *
 * @param[in] u8DataPin  各ビットを出力するピン番号を指定します。
 * @param[in] u8ClockPin クロックを出力するピン番号を指定します。u8DataPinに正し
                         い値がセットされたら、このピンが1回反転します
 * @param[in] u8BitOrder MSBFIRSTまたはLSBFIRSTを指定します。
 * @param[in] u8Len      出力するデータのビット数
 * @param[in] u32Value   送信したいデータ (byte) を指定します。
 *
 * @return なし
 *
 * @attention
 * u8DataPinとu8ClockPinは、あらかじめ pinMode()関数によって出力(OUTPUT)に設定され
 * ている必要があります。
 ***************************************************************************/
void shiftOutEx(uint8_t u8DataPin, uint8_t u8ClockPin, uint8_t u8BitOrder, uint8_t u8Len, uint32_t u32Value)
{
	int i;

	FUNC_MUTEX_LOCK;

	for (i = 0; i < u8Len; i++) {
		if (u8BitOrder == LSBFIRST) {
			_digitalWrite(u8DataPin, !!(u32Value & (1UL << i)));
			}
		else {
			_digitalWrite(u8DataPin, !!(u32Value & (1UL << (31 - i))));
		}
		_digitalWrite(u8ClockPin, HIGH);
		_digitalWrite(u8ClockPin, LOW);
	}

	FUNC_MUTEX_UNLOCK;
}


/**
 * 1バイトのデータを1ビットずつ「シフトイン」します。
 *
 * 最上位ビット(MSB)と最下位ビット(LSB)のどちらからもスタートできます。各ビット
 * について次のように動作します。まずu8ClockPinがHIGHになり、u8DdataPinから次の
 * ビットが読み込まれ、u8ClockPinがLOWに戻ります。
 *
 * @param[in] u8DataPin  ビットを入力するピン番号を指定します。
 * @param[in] u8ClockPin クロックを出力するピン番号を指定します。
 * @param[in] u8BitOrder MSBFIRSTまたはLSBFIRSTを指定します。
 *
 * @return 読み取った値(byte)を返却します。
 *
 * @attention なし
 ***************************************************************************/
uint8_t shiftIn(uint8_t u8DataPin, uint8_t u8ClockPin, uint8_t u8BitOrder)
{
	uint8_t i;
	uint8_t u8Value = 0;

	FUNC_MUTEX_LOCK;

	for (i = 0; i < 8; ++i) {
		_digitalWrite(u8ClockPin, HIGH);
		if (u8BitOrder == LSBFIRST) {
			u8Value |= (uint8_t)_digitalRead(u8DataPin) << i;
		}
		else {
			u8Value |= (uint8_t)_digitalRead(u8DataPin) << (7 - i);
		}
		_digitalWrite(u8ClockPin, LOW);
	}

	FUNC_MUTEX_UNLOCK;

	return u8Value;
}


#ifdef __cplusplus
/**
 * ピンに入力されるパルスを検出します。
 *
 * たとえば、パルスの種類(u8Value)をHIGHに指定した場合、 pulseIn()関数は入力がHIGHに
 * 変わると同時に時間の計測を始め、またLOWに戻ったら、そこまでの時間(つまりパルス
 * の長さ)をマイクロ秒単位で返します。
 *
 * @param[in] u8Pin      パルスを入力するピン番号を指定します。
 * @param[in] u8Value    測定するパルスの種類（HIGHまたはLOW）を指定します。
 *
 * @return パルスの長さ(マイクロ秒)を返却します。
 *         パルスがスタートする前にタイムアウトとなった場合は0を返却します。
 *
 * @attention なし
 ***************************************************************************/
unsigned long pulseIn(uint8_t u8Pin, uint8_t u8Value)
{
	return pulseIn(u8Pin, u8Value, 1000000);
}
#endif


/**
 * ピンに入力されるパルスを検出します。
 *
 * たとえば、パルスの種類(u8Value)をHIGHに指定した場合、 pulseIn()関数は入力がHIGHに
 * 変わると同時に時間の計測を始め、またLOWに戻ったら、そこまでの時間(つまりパルス
 * の長さ)をマイクロ秒単位で返します。タイムアウトを指定した場合は、その時間を超
 * えた時点で0を返します。
 *
 * @param[in] u8Pin      パルスを入力するピン番号を指定します。
 * @param[in] u8Value    測定するパルスの種類（HIGHまたはLOW）を指定します。
 * @param[in] u32Timeout タイムアウトまでの時間(単位・マイクロ秒)を指定します。
 *
 * @return パルスの長さ(マイクロ秒)を返却します。
 *         パルスがスタートする前にタイムアウトとなった場合は0を返却します。
 *
 * @attention なし
 ***************************************************************************/
static bool pulseInWait(
	volatile uint8_t* Px,
	uint8_t u8Bit,
	uint8_t u8StateMask,
	unsigned long* u32TimeoutCycles
) {
	bool ret;
#if 0
	const unsigned char u16PulseInWaitCycles = 30;
	while ((*Px & u8Bit) == u8StateMask) {
		if (*u32TimeoutCycles >= u16PulseInWaitCycles) {
			*u32TimeoutCycles -= u16PulseInWaitCycles;
		} else {
			ret = true;
			break;
		}
	}
	ret = false;
#else
	__asm __volatile(
		"	clrb	%0					\n"
		"	movw	de, %1					\n"
		"	mov	a, %2					\n"
		"	mov	b, a					\n"
		"	mov	a, %3					\n"
		"	mov	c, a					\n"
		"	movw	hl, %4					\n"
		"1:							\n"
		"	mov	a, [de]				;1	\n"
		"	and	a, b				;1	\n"
		"	cmp	a, c				;1	\n"
		"	bnz	$2f				;2/4	\n"
		"	movw	ax, [hl]			;1	\n"
		"	subw	ax, #PulseInWaitCycles		;1	\n"
		"	movw	[hl], ax			;1	\n"
		"	movw	ax, [hl+2]			;1	\n"
		"	sknc					;1	\n"
		"	subw	ax, #1				;1	\n"
		"	movw	[hl+2], ax			;1	\n"
		"	bnc	$1b				;2/4	\n"
		"	.equiv	PulseInWaitCycles, 1+1+1+2+1+1+1+1+1+1+1+4	\n"
		"	oneb	%0					\n"
		"2:		 					\n"
		:"=&r"(ret)
		:"r"(Px), "r"(u8Bit), "r"(u8StateMask), "r"(u32TimeoutCycles)
		:"ax", "bc", "de", "hl"
	);
#endif
	return ret;
}

static bool pulseInCount(
	volatile uint8_t* Px,
	uint8_t u8Bit,
	uint8_t u8StateMask,
	unsigned long u32TimeoutCycles,
	unsigned long* u32PulseCycles
) {
	bool ret;
#if 0
	const unsigned long u16PulseInCountCycles = 35;
	*u32PulseCycles = 0;
	while ((*Px & u8Bit) == u8StateMask) {
		if (u32TimeoutCycles >= u16PulseInCountCycles) {
			u32TimeoutCycles -= u16PulseInCountCycles;
		} else {
			ret = true;
			break;
		}
		*u32PulseCycles += u16PulseInCountCycles;
	}
	ret = false;
#else
	__asm __volatile(
		"	clrb	%0					\n"
		"	movw	de, %1					\n"
		"	mov	a, %2					\n"
		"	mov	b, a					\n"
		"	mov	a, %3					\n"
		"	mov	c, a					\n"
		"	movw	hl, %5					\n"
		"	movw	ax, PulseInWaitCycles/2+PulseInCountCycles/2	\n"
		"	movw	[hl], ax				\n"
		"	clrw	ax					\n"
		"	movw	[hl+2], ax				\n"
		"	br	$2f					\n"
		"1:							\n"
		"	movw	ax, [hl]			;1	\n"
		"	addw	ax, #PulseInCountCycles		;1	\n"
		"	movw	[hl], ax			;1	\n"
		"	movw	ax, [hl+2]			;1	\n"
		"	sknc					;1	\n"
		"	incw	ax				;1	\n"
		"	movw	[hl+2], ax			;1	\n"
		"2:							\n"
		"	mov	a, [de]				;1	\n"
		"	and	a, b				;1	\n"
		"	cmp	a, c				;1	\n"
		"	bnz	$3f				;2/4	\n"
		"	movw	ax, %4+0			;1	\n"
		"	subw	ax, #PulseInCountCycles	;1		\n"
		"	movw	%4+0, ax			;1	\n"
		"	movw	ax, %4+2			;1	\n"
		"	sknc					;1	\n"
		"	subw	ax, #1				;1	\n"
		"	movw	%4+2, ax			;1	\n"
		"	bnc	$1b				;2/4	\n"
		"	.equiv	PulseInCountCycles, 1+1+1+1+1+1+1+1+1+1+2+1+1+1+1+1+1+1+4	\n"
		"	oneb	%0					\n"
		"3:		 					\n"
		:"=&r"(ret)
		:"r"(Px), "r"(u8Bit), "r"(u8StateMask), "r"(u32TimeoutCycles), "r"(u32PulseCycles)
		:"ax", "bc", "de", "hl"
	);
#endif
	return ret;
}

unsigned long pulseIn(uint8_t u8Pin, uint8_t u8Value, unsigned long u32Timeout)
{
	uint8_t u8StateMask;
	unsigned long u32TimeoutCycles;
	unsigned long u32PulseCycles;
	unsigned long u32PulseLength = 0;
	bool timeOut;

	FUNC_MUTEX_LOCK;

	if (u8Pin < NUM_DIGITAL_PINS) {
		PinTableType* p = getPinTable(u8Pin);
		u8StateMask = (u8Value ? p->mask : 0);
		{
			u32TimeoutCycles = microsecondsToClockCycles(u32Timeout);
			timeOut =  pulseInWait(p->portRegisterAddr, p->mask, u8StateMask, &u32TimeoutCycles);

			if (!timeOut) {
				timeOut =  pulseInWait(p->portRegisterAddr, p->mask, u8StateMask ^ p->mask, &u32TimeoutCycles);
				if (!timeOut) {
					timeOut = pulseInCount(p->portRegisterAddr, p->mask, u8StateMask, u32TimeoutCycles, &u32PulseCycles);
				}
			}
			if (!timeOut) {
				u32PulseLength = clockCyclesToMicroseconds(u32PulseCycles);
			}
		}
	}

	FUNC_MUTEX_UNLOCK;

	return u32PulseLength;
}

#ifdef __cplusplus
/**
 * 指定した周波数の矩形波(50%デューティ)を生成します。
 *
 * noTone()関数を実行するまで動作を続けます。
 * 出力ピンに圧電ブザーやスピーカに接続することで、一定ピッチの音を再生できます。
 * 同時に生成できるのは1音だけです。すでに他のピンで tone()関数が実行されている
 * 場合、次に実行した tone()関数は効果がありません。同じピンに対して tone()関数
 * を実行した場合は周波数が変化します。
 *
 * @param[in] u8Pin        トーンを出力するピン番号を指定します。
 * @param[in] u16Frequency 周波数（Hz）を指定します。
 *
 * @return なし
 *
 * @attention この関数はデジタルピンのD3のPWM出力を妨げます。
 ***************************************************************************/
void tone(uint8_t u8Pin, unsigned int u16Frequency)
{
	tone(u8Pin, u16Frequency, 0);
}
#endif

/**
 * 指定した周波数の矩形波(50%デューティ)を生成します。
 *
 * 時間(duration)を指定しなかった場合、 noTone()関数を実行するまで動作を続けます。
 * 出力ピンに圧電ブザーやスピーカに接続することで、一定ピッチの音を再生できます。
 * 同時に生成できるのは1音だけです。すでに他のピンで tone()関数が実行されている
 * 場合、次に実行した tone()関数は効果がありません。同じピンに対して tone()関数
 * を実行した場合は周波数が変化します。
 *
 * @param[in] u8Pin        トーンを出力するピン番号を指定します。
 * @param[in] u16Frequency 周波数（Hz）を指定します。
 * @param[in] u32Duration  出力する時間をミリ秒で指定します。
 *
 * @return なし
 *
 * @attention この関数はデジタルピンのD3のPWM出力を妨げます。
 ***************************************************************************/
void tone(uint8_t u8Pin, unsigned int u16Frequency, unsigned long u32Duration)
{
	unsigned int u16TDR0x;

	FUNC_MUTEX_LOCK;

	if (u8Pin < NUM_DIGITAL_PINS) {
//  removed from v1.05
//		if (g_u8TonePin == u8Pin) {
//			noTone(u8Pin);	// 既にトーンを出力中の場合、トーンを停止する。
//		}

		if (g_u8TonePin == NOT_A_PIN) {
			// 出力モードに設定
			_pinMode(u8Pin, OUTPUT);

			// トーン出力ピンの保存
			g_u8TonePin = u8Pin;

			_startTAU0(TIMER_CLOCK);

			// 周期の計算
			//
			// パルス周期[s] = (TDR0x設定値 + 1) *
			// TDR0x設定値 = パルス周期(1/u16Frequency/2) * タイマ・クロック周期(125kHz) - 1
			//
			u16TDR0x = (unsigned int)(125000 / u16Frequency / 2 - 1);

			// 出力時間の設定
			if (u32Duration == 0) {
				// 出力時間を無限に設定
				g_u32ToneDuration = TONE_DURATION_INFINITY;
			}
			else {
				// 出力時間[ms]から割り込み回数に変換
				//
				// 出力時間[ms] = パルス周期(1/u16Frequence/2) * 割り込み回数 * 1000
				// 割り込み回数 = 出力時間[ms] / パルス周期(1/u16Frequency/2) / 1000
				g_u32ToneDuration = u32Duration * u16Frequency * 2 / 1000;
			}

			// 変数の初期化
			g_u8ToneToggle = 0;
			g_u32ToneInterruptCount = 0;

			// タイマーの設定
			_startTimerChannel(TONE_TIMER, TONE_MODE, u16TDR0x, false, true);
		}
	}

	FUNC_MUTEX_UNLOCK;
}


/**
 * tone()関数で開始された矩形波の生成を停止します。
 *
 * tone()関数が実行されていない場合はなにも起こりません。
 * 
 * @param[in] u8Pin トーンの生成を停止したいピン番号を指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void noTone(uint8_t u8Pin)
{
	FUNC_MUTEX_LOCK;

	if (g_u8TonePin != NOT_A_PIN) {
		// タイマーの停止
		_stopTimerChannel(TONE_TIMER);

		// タイマーアレイユニットの停止
		_stopTAU0();

		// 入力モードに設定
		_pinMode(u8Pin, INPUT);

		g_u8TonePin = NOT_A_PIN;
	}

	FUNC_MUTEX_UNLOCK;
}
/** @} */


/** ************************************************************************
 * @defgroup group5 時間に関する関数
 * 
 * @{
 ***************************************************************************/
/**
 * プログラムの実行を開始した時から現在までの時間（ms）返します。
 *
 * プログラムの実行を開始した時から現在までの時間をミリ秒単位で返します。
 * 約50日間でオーバーフローし、ゼロに戻ります。
 *
 * @return 実行中のプログラムがスタートしてからの時間を返却します。
 *
 * @attention なし
 ***************************************************************************/
unsigned long millis(void)
{
	unsigned long u32ms;

	FUNC_MUTEX_LOCK;

#ifdef USE_RTOS
	u32ms = xTaskGetTickCount() / portTICK_RATE_MS;
#else
	byte _psw = PSW.psw;
	noInterrupts();
	u32ms = g_u32timer_millis;
	PSW.psw = _psw;
#endif

	FUNC_MUTEX_UNLOCK;

	return u32ms;
}


/**
 * プログラムの実行を開始した時から現在までの時間（us）を返します。
 *
 * プログラムの実行を開始した時から現在までの時間をマイクロ秒単位で返します。
 * 約70分間でオーバーフローし、ゼロに戻ります。
 *
 * @return 実行中のプログラムが動作し始めてからの時間をマイクロ秒単位で返却します。
 *
 * @attention
 * この関数の分解能は1usになります。
 * ただし、動作クロックが低速動作モード（CLK_LOW_SPEED_MODE）の場合は分解能が1msになります。
 ***************************************************************************/
unsigned long micros(void)
{
    unsigned long a;
    unsigned long m;
    uint16_t t;

    FUNC_MUTEX_LOCK;

#ifdef USE_RTOS
    u32us = (unsigned long)xTaskGetTickCount() / portTICK_RATE_MS * 1000;
#else

    if (g_u8OperationClockMode == CLK_HIGH_SPEED_MODE) {
        // 割り込み禁止状態で
        if (isNoInterrupts()) {
            // TCR05.tcr05 を参照する前の g_timer05_overflow_count の値
            m = g_timer05_overflow_count;
            // TCR05.tcr05 を参照する直前でオーバーフローしてるか?
            bool ov0 = TMIF05;
            // TCR05.tcr05 の値
            t = TCR05.tcr05;
            // TCR05.tcr05 を参照した直前でオーバーフローしてるか?
            bool ov1 = TMIF05;

            if (!ov0 && ov1) {
                // TCR05.tcr05 を参照した付近でオーバーフローしたのであれば、
                // t の値は捨てて TDR の初期値を代入し、オーバーフローの補正を行う
                t = INTERVAL_MICRO_TDR;
                m++;
            } else if (ov0) {
                // タイマーが最初っからオーバーフローしてるのであれば、g_timer05_overflow_count の値の補正を行う
                m++;
            }
        // 割り込み許可状態で
        } else {
            // TCR05.tcr05 を参照する直前の g_timer05_overflow_count の値
            a = g_timer05_overflow_count;
            // TCR05.tcr05 の値
            t = TCR05.tcr05;
            // TCR05.tcr05 を参照した直後の g_timer05_overflow_count の値
            m = g_timer05_overflow_count;

            if (a != m) {
                // TCR05.tcr05 を参照する直前と直後の g_timer05_overflow_count の値が
                // 異なっているのであれば、これはどう考えても TCR05.tcr05 の値を参照した付近で
                // インターバルのタイミングが発生してたということなので、t に格納されてる値は捨てて、
                // TCR05.tcr05 の値は TDR の値に設定して問題ない
                t = INTERVAL_MICRO_TDR;
            } else if (t == INTERVAL_MICRO_TDR) {
                // TCR05 がオーバーフローを起こし、割り込みが発生していれば割り込みの処理で
                // 32クロック以上掛かるので、TCR05.tcr05 の値は TDR05 の値より -1 以上
                // 小さくなっている筈。それがなく、TCR05.tcr05 の値が TDR05 と等しいということは
                // 割り込み処理がまだ行われてないということなので補正を行う
                m++;
            }
        }
        m = m * MICROSECONDS_PER_TIMER05_OVERFLOW + (INTERVAL_MICRO_TDR - t);
    }
    else {
        // ミリ秒 x 1000;
        m = millis() * 1000;
    }
#endif

    FUNC_MUTEX_UNLOCK;

    return m;
}

/**
 * プログラムを指定した時間(ms)だけ止めます。
 *
 * 単位はミリ秒です(1,000ミリ秒=1秒)。
 * このパラメータはunsigned long型です。32767より大きい整数を指定するときは、
 * 値の後ろにULを付け加えます。例 delay(60000UL);
 *
 * setPowerManagementMode()関数で省電力(HALTまたはSTOP/SNOOZE)モードを指定た場合、
 * delay()関数を呼び出すとHALT命令またはSTOP命令によりスタンバイ状態に遷移し、
 * delay()関数の引数で指定した一時停止時間（u32ms）経過後にスタンバイ状態が解除
 * され delay()関数からリターンします。<br>
 * また、 setPowerManagementMode()関数で省電力(STOP/SNOOZE)モードを指定し、かつ、
 * 一時停止時間（u32ms）に0xFFFFFFFFを設定した場合、何らかの割込み（外部割込み、RTC等）
 * が発生するまでSTOP命令によりスタンバイ状態に遷移し delay()関数から戻りません。
 * このときインターバルタイマも停止するため millis()関数、 micros()関数で取得で
 * きる時間情報も更新されないので注意してください。<br>
 * なお、 setPowerManagementMode()関数で省電力(STOP/SNOOZE)モードを指定しても、
 * PWM、tone、Serial、SPI、I2C等が動作している場合は、STOP命令の代わりにHALT命令を発行します。
 *
 * @param[in] u32ms 一時停止する時間(ms)を指定します。
 *
 * @return なし
 *
 * @attention
 * delay()関数を使えば簡単にLEDをチカチカさせることができます。また、スイッチの
 * バウンス対策のために delay()関数を使っているスケッチもよく見られます。ただし、
 * こうした delay()関数の使い方には不利な点があります。 delay()関数の実行中は、
 * 計算やピン操作といった他の処理が実質的に止まってしまうのです。 delay()関数の
 * 代わりに millis()関数を使って時間を測り、タイミングをコントロールするほうが
 * いいでしょう。熟練したプログラマーは、よほどスケッチが簡単になる場合を除き、
 * 10ms以上のイベントのコントロールに delay()関数を使うことは避けるでしょう。
 * delay()関数の実行中も割り込みは有効なので、いくつかの処理は同時に実行可能です。
 * シリアルポート(RX)に届いたデータは記録され、PWM(analogWrite)の状態は維持
 * されます。つまり割り込み処理は影響を受けません。
 **********************	*****************************************************/
void delay(unsigned long ms)
{
#ifdef USE_RTOS
	vTaskDelay(ms / portTICK_RATE_MS);
#else

	if (g_u8PowerManagementMode == PM_NORMAL_MODE) {
		uint16_t start = (uint16_t)micros();

		while (ms > 0) {
			if (((uint16_t)micros() - start) >= 1000) {
				ms--;
				start += 1000;
			}
		}
	} else {
		enterPowerManagementMode(ms);
	}

#endif
}


/**
 * プログラムを指定した時間(us)だけ止めます。
 *
 * 単位はマイクロ秒です。数千マイクロ秒を超える場合は delay()関数を使用すること
 * を推奨します。
 *
 * @param[in] u16us: 一時停止する時間(us)を指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void delayMicroseconds(unsigned int us)
{
	if (g_u8PowerManagementMode == PM_NORMAL_MODE) {
		unsigned long s, w, d;
		s = micros();
		w = us;
		d = 0;
		while (w > d) {
			d = micros() - s;
		}
	} else {
		enterPowerManagementMode(us / 1000);
	}
}
/** @} */


/** ************************************************************************
 * @defgroup group6 数学的な関数
 * 
 * @{
 ***************************************************************************/
/**
 * 数値をある範囲から別の範囲に変換します。
 *
 * 現在の範囲の下限（s32fromLow）と同じ値を与えると、変換後の範囲の下限（s32toLow）が返り、
 * 現在の範囲の上限（s32fromHigh）と同じ値なら変換後の範囲の上限（s32toHigh）となります。
 * その中間の値は、2つの範囲の大きさの比に基づいて計算されます。
 * そのほうが便利な場合があるので、この関数は範囲外の値も切り捨てません。
 * ある範囲のなかに収めたい場合は、 constrain()関数と併用してください。
 * 範囲の下限を上限より大きな値に設定できます。そうすると値の反転に使えます。
 *   例 y = map(x, 1, 50, 50, 1);
 * 範囲を指定するパラメータに負の数を使うこともできます。
 *   例 y = map(x, 1, 50, 50, -100);
 * map()関数は整数だけを扱います。計算の結果、小数が生じる場合、小数部分は単純
 * に切り捨てられます。
 *
 * @param[in] s32Value    変換したい数値を指定します。
 * @param[in] s32fromLow  現在の範囲の下限を指定します。
 * @param[in] s32fromHigh 現在の範囲の上限を指定します。
 * @param[in] s32toLow    変換後の範囲の下限を指定します。
 * @param[in] s32toHigh   変換後の範囲の上限を指定します。
 *
 * @return 変換後の数値を返却します。
 *
 * @attention なし
 ***************************************************************************/
long map(long s32Value, long s32fromLow, long s32fromHigh, long s32toLow, long s32toHigh)
{
	return (s32Value - s32fromLow) * (s32toHigh - s32toLow) / (s32fromHigh - s32fromLow) + s32toLow;
}
/** @} */


/** ************************************************************************
 * @defgroup group7 三角関数
 * 
 * @{
 ***************************************************************************/
/** @} */


/** ************************************************************************
 * @defgroup gropu8 乱数に関する関数
 * 
 * @{
 ***************************************************************************/
/**
 * 疑似乱数ジェネレータを初期化します。
 *
 * randomSeed()関数は疑似乱数ジェネレータを初期化して、乱数列の任意の点からス
 * タートします。この乱数列はとても長いものですが、常に同一です。
 * random()関数がプログラムを実行するたびに異なった乱数列を発生することが重要
 * な場合、未接続のピンを analogRead()関数で呼んだ値のような、真にランダムな数
 * 値と組み合わせて randomSeed()関数を実行してください。
 * 逆に、疑似乱数が毎回同じ数列を作り出す性質を利用する場合は、randomSeed関
 * 数を毎回、同じ値で実行してください。
 *
 * @param[in] u16Seed 乱数の種となる数値を指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void randomSeed(unsigned int u16Seed)
{
	if (u16Seed != 0) {
		srand(u16Seed);
	}
}

#ifdef __cplusplus
/**
 * 疑似乱数を生成します。
 *
 * @param[in] s32Max 生成する乱数の上限を指定します。
 *
 * @return 0からmax-1の間の整数を返却します。
 *
 * @attention なし
 ***************************************************************************/
long random(long s32Max)
{
	return random(0, s32Max);
}
#endif

/**
 * 疑似乱数を生成します。
 *
 * @param[in] s32Min 生成する乱数の下限を指定します。
 * @param[in] s32Max 生成する乱数の上限を指定します。
 *
 * @return minからmax-1の間の整数を返却します。
 *
 * @attention なし
 ***************************************************************************/
long random(long s32Min, long s32Max)
{
	int s16Value;
	long s32Result = s32Min;

	FUNC_MUTEX_LOCK;
	
	if (s32Min < s32Max) {
		s16Value = rand();	// 0 <= s16Value <= RAND_MAX
		s32Result = map(s16Value, 0, RAND_MAX, s32Min, s32Max);
	}

	FUNC_MUTEX_LOCK;

	return s32Result;
}
/** @} */


/** ************************************************************************
 * @defgroup group9 ビットとバイトの操作
 * 
 * @{
 ***************************************************************************/
/** @} */


/** ************************************************************************
 * @defgroup group10 外部割込み
 * 
 * @{
 ***************************************************************************/
/**
 * 外部割込みハンドラを登録します。
 *
 * 外部割り込みが発生したときに実行する関数を指定します。すでに指定されてい
 * た関数は置き換えられます。割り込み0(ピン2)と割り込み1(ピン3)と呼ばれる2つ
 * の外部割り込みに対応しています。呼び出せる関数は引数と戻り値が不要なもの
 * だけです。
 *
 * @param[in] u8Interrupt 割り込み番号 0または1を指定します。
 * @param[in] fFunction   割り込み発生時に呼び出す関数を指定します。
 * @param[in] s16Mode     割り込みを発生させるトリガを指定します。
 *                        @arg LOW     : ピンがLOWのとき発生(RL78は未対応のためCHANGEが設定されます)
 *                        @arg CHANGE  : ピンの状態が変化したときに発生
 *                        @arg RISING  : ピンの状態がLOWからHIGHに変わったときに発生
 *                        @arg FALLING : ピンの状態がHIGHからLOWに変わったときに発生
 *
 * @return なし
 *
 * @attention
 * attachInterrupt()関数で指定した関数の中では次の点に気をつけてください。
 *  - delay()関数は機能しません
 *  - millis()関数の戻り値は増加しません
 *  - シリアル通信により受信したデータは、失われる可能性があります
 *  - 割り当てた関数の中で値が変化する変数にはvolatileをつけて宣言すべきです
 ***************************************************************************/
void attachInterrupt(uint8_t u8Interrupt, void (*fFunction)(void), int s16Mode)
{
	FUNC_MUTEX_LOCK;

	if (u8Interrupt < EXTERNAL_NUM_INTERRUPTS) {
		/* ユーザー定義割り込みハンドラの登録 */
		g_afInterruptFuncTable[u8Interrupt] = fFunction;
		switch (u8Interrupt) {
		case 0:
			/* 割り込みモードの設定 */
			if (s16Mode == FALLING) {
				EGP0.egp0 &= ~(1 << EXTERNAL_INTERRUPT_0);
				EGN0.egn0 |=  (1 << EXTERNAL_INTERRUPT_0);
			}
			else if (s16Mode == RISING) {
				EGP0.egp0 |=  (1 << EXTERNAL_INTERRUPT_0);
				EGN0.egn0 &= ~(1 << EXTERNAL_INTERRUPT_0);
			}
			else {
				EGP0.egp0 |=  (1 << EXTERNAL_INTERRUPT_0);
				EGN0.egn0 |=  (1 << EXTERNAL_INTERRUPT_0);
			}
			/* 割り込み要因のクリア＆割り込みマスクを許可 */
#ifdef WORKAROUND_READ_MODIFY_WRITE
#if EXTERNAL_INTERRUPT_0 == 0
			SBI(SFR_PR00L, 2);
			SBI(SFR_PR10L, 2);
			CBI(SFR_IF0L,  2);
			CBI(SFR_MK0L,  2);
#elif EXTERNAL_INTERRUPT_0 == 1
			SBI(SFR_PR00L, 3);
			SBI(SFR_PR10L, 3);
			CBI(SFR_IF0L,  3);
			CBI(SFR_MK0L,  3);
#elif EXTERNAL_INTERRUPT_0 == 2
			SBI(SFR_PR00L, 4);
			SBI(SFR_PR10L, 4);
			CBI(SFR_IF0L,  4);
			CBI(SFR_MK0L,  4);
#elif EXTERNAL_INTERRUPT_0 == 3
			SBI(SFR_PR00L, 5);
			SBI(SFR_PR10L, 5);
			CBI(SFR_IF0L,  5);
			CBI(SFR_MK0L,  5);
#elif EXTERNAL_INTERRUPT_0 == 4
			SBI(SFR_PR00L, 6);
			SBI(SFR_PR10L, 6);
			CBI(SFR_IF0L,  6);
			CBI(SFR_MK0L,  6);
			PIF4 = 0;	PMK4 = 0;
#elif EXTERNAL_INTERRUPT_0 == 5
			SBI(SFR_PR00L, 7);
			SBI(SFR_PR10L, 7);
			CBI(SFR_IF0L,  7);
			CBI(SFR_MK0L,  7);
#endif
#else	/* WORKAROUND_READ_MODIFY_WRITE */
#if EXTERNAL_INTERRUPT_0 == 0
			PPR00 = 1; PPR10 = 1;
			PIF0 = 0;	PMK0 = 0;
#elif EXTERNAL_INTERRUPT_0 == 1
			PPR01 = 1; PPR11 = 1;
			PIF1 = 0;	PMK1 = 0;
#elif EXTERNAL_INTERRUPT_0 == 2
			PPR02 = 1; PPR12 = 1;
			PIF2 = 0;	PMK2 = 0;
#elif EXTERNAL_INTERRUPT_0 == 3
			PPR03 = 1; PPR13 = 1;
			PIF3 = 0;	PMK3 = 0;
#elif EXTERNAL_INTERRUPT_0 == 4
			PPR04 = 1; PPR14 = 1;
			PIF4 = 0;	PMK4 = 0;
#elif EXTERNAL_INTERRUPT_0 == 5
			PPR05 = 1; PPR15 = 1;
			PIF5 = 0;	PMK5 = 0;
#endif
#endif
			break;
		
		case 1:
			/* 割り込みモードの設定 */
			if (s16Mode == FALLING) {
				EGP0.egp0 &= ~(1 << EXTERNAL_INTERRUPT_1);
				EGN0.egn0 |=  (1 << EXTERNAL_INTERRUPT_1);
			}
			else if (s16Mode == RISING) {
				EGP0.egp0 |=  (1 << EXTERNAL_INTERRUPT_1);
				EGN0.egn0 &= ~(1 << EXTERNAL_INTERRUPT_1);
			}
			else {
				EGP0.egp0 |=  (1 << EXTERNAL_INTERRUPT_1);
				EGN0.egn0 |=  (1 << EXTERNAL_INTERRUPT_1);
			}
			/* 割り込み要因のクリア＆割り込みマスクを許可 */
#ifdef WORKAROUND_READ_MODIFY_WRITE
#if EXTERNAL_INTERRUPT_1 == 0
			SBI(SFR_PR00L, 2);
			SBI(SFR_PR10L, 2);
			CBI(SFR_IF0L,  2);
			CBI(SFR_MK0L,  2);
#elif EXTERNAL_INTERRUPT_1 == 1
			SBI(SFR_PR00L, 3);
			SBI(SFR_PR10L, 3);
			CBI(SFR_IF0L,  3);
			CBI(SFR_MK0L,  3);
#elif EXTERNAL_INTERRUPT_1 == 2
			SBI(SFR_PR00L, 4);
			SBI(SFR_PR10L, 4);
			CBI(SFR_IF0L,  4);
			CBI(SFR_MK0L,  4);
#elif EXTERNAL_INTERRUPT_1 == 3
			SBI(SFR_PR00L, 5);
			SBI(SFR_PR10L, 5);
			CBI(SFR_IF0L,  5);
			CBI(SFR_MK0L,  5);
#elif EXTERNAL_INTERRUPT_1 == 4
			SBI(SFR_PR00L, 6);
			SBI(SFR_PR10L, 6);
			CBI(SFR_IF0L,  6);
			CBI(SFR_MK0L,  6);
#elif EXTERNAL_INTERRUPT_1 == 5
			SBI(SFR_PR00L, 7);
			SBI(SFR_PR10L, 7);
			CBI(SFR_IF0L,  7);
			CBI(SFR_MK0L,  7);
#endif
#else	/* WORKAROUND_READ_MODIFY_WRITE */
#if EXTERNAL_INTERRUPT_1 == 0
			PPR00 = 1; PPR10 = 1;
			PIF0 = 0;	PMK0 = 0;
#elif EXTERNAL_INTERRUPT_1 == 1
			PPR01 = 1; PPR11 = 1;
			PIF1 = 0;	PMK1 = 0;
#elif EXTERNAL_INTERRUPT_1 == 2
			PPR02 = 1; PPR12 = 1;
			PIF2 = 0;	PMK2 = 0;
#elif EXTERNAL_INTERRUPT_1 == 3
			PPR03 = 1; PPR13 = 1;
			PIF3 = 0;	PMK3 = 0;
#elif EXTERNAL_INTERRUPT_1 == 4
			PPR04 = 1; PPR14 = 1;
			PIF4 = 0;	PMK4 = 0;
#elif EXTERNAL_INTERRUPT_1 == 5
			PPR05 = 1; PPR15 = 1;
			PIF5 = 0;	PMK5 = 0;
#endif
#endif
			break;
		}
	}

	FUNC_MUTEX_UNLOCK;
}


/**
 * 外部割込みハンドラの登録を解除します。
 *
 * 指定した割り込みの外部割込みハンドラの登録を解除します。
 *
 * @param[in] u8Interrupt 停止したい割り込みの番号を指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void detachInterrupt(uint8_t u8Interrupt)
{
	FUNC_MUTEX_LOCK;

	if (u8Interrupt < EXTERNAL_NUM_INTERRUPTS) {
		switch (u8Interrupt) {

		case 0:
			/* エッジ検出禁止に設定 */
			EGP0.egp0 &= ~(1 << EXTERNAL_INTERRUPT_0);
			EGN0.egn0 &= ~(1 << EXTERNAL_INTERRUPT_0);
			/* 外部割込み0の割り込みマスクの禁止 */
#ifdef WORKAROUND_READ_MODIFY_WRITE
#if EXTERNAL_INTERRUPT_0 == 0
			SBI(SFR_MK0L, 2);
#elif EXTERNAL_INTERRUPT_0 == 1
			SBI(SFR_MK0L, 3);
#elif EXTERNAL_INTERRUPT_0 == 2
			SBI(SFR_MK0L, 4);
#elif EXTERNAL_INTERRUPT_0 == 3
			SBI(SFR_MK0L, 5);
#elif EXTERNAL_INTERRUPT_0 == 4
			SBI(SFR_MK0L, 6);
#elif EXTERNAL_INTERRUPT_0 == 5
			SBI(SFR_MK0L, 7);
#endif
#else	/* WORKAROUND_READ_MODIFY_WRITE */
#if EXTERNAL_INTERRUPT_0 == 0
			PMK0 = 1;
#elif EXTERNAL_INTERRUPT_0 == 1
			PMK1 = 1;
#elif EXTERNAL_INTERRUPT_0 == 2
			PMK2 = 1;
#elif EXTERNAL_INTERRUPT_0 == 3
			PMK3 = 1;
#elif EXTERNAL_INTERRUPT_0 == 4
			PMK4 = 1;
#elif EXTERNAL_INTERRUPT_0 == 5
			PMK5 = 1;
#endif
#endif
			break;

		case 1:
			/* エッジ検出禁止に設定 */
			EGP0.egp0 &= ~(1 << EXTERNAL_INTERRUPT_1);
			EGN0.egn0 &= ~(1 << EXTERNAL_INTERRUPT_1);
			/* 外部割込み1の割り込みマスクの禁止 */
#ifdef WORKAROUND_READ_MODIFY_WRITE
#if EXTERNAL_INTERRUPT_1 == 0
			SBI(SFR_MK0L, 2);
#elif EXTERNAL_INTERRUPT_1 == 1
			SBI(SFR_MK0L, 3);
#elif EXTERNAL_INTERRUPT_1 == 2
			SBI(SFR_MK0L, 4);
#elif EXTERNAL_INTERRUPT_1 == 3
			SBI(SFR_MK0L, 5);
#elif EXTERNAL_INTERRUPT_1 == 4
			SBI(SFR_MK0L, 6);
#elif EXTERNAL_INTERRUPT_1 == 5
			SBI(SFR_MK0L, 7);
#endif
#else	/* WORKAROUND_READ_MODIFY_WRITE */
#if EXTERNAL_INTERRUPT_1 == 0
			PMK0 = 1;
#elif EXTERNAL_INTERRUPT_1 == 1
			PMK1 = 1;
#elif EXTERNAL_INTERRUPT_1 == 2
			PMK2 = 1;
#elif EXTERNAL_INTERRUPT_1 == 3
			PMK3 = 1;
#elif EXTERNAL_INTERRUPT_1 == 4
			PMK4 = 1;
#elif EXTERNAL_INTERRUPT_1 == 5
			PMK5 = 1;
#endif
#endif
			break;
		}
		/* ユーザー定義割り込みハンドラの登録解除 */
		g_afInterruptFuncTable[u8Interrupt] = NULL;
	}

	FUNC_MUTEX_UNLOCK;
}
/** @} */


/** ************************************************************************
 * @defgroup group11 割込み
 * 
 * @{
 ***************************************************************************/
/** @} */
/** @} group100 Arduino互換関数　*/

/// @cond
/**
 * トーン生成用割り込みハンドラ1
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
INTERRUPT void tone_interrupt(void)
{
	if (g_u32ToneDuration != TONE_DURATION_INFINITY) {
		if (g_u32ToneInterruptCount >= g_u32ToneDuration) {
			noTone(g_u8TonePin);
		}
	}
	// tone()関数が呼び出されている場合、ピンの出力を反転させる。
	if (g_u8TonePin != NOT_A_PIN) {
		g_u32ToneInterruptCount++;
		// Toggle pin.
		g_u8ToneToggle ^= 1;
		_digitalWrite(g_u8TonePin, g_u8ToneToggle);
	}
}


/**
 * 外部割込みハンドラ0
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
INTERRUPT void external_interrupt_0(void)
{
	if (g_afInterruptFuncTable[0] != NULL) {
		(*g_afInterruptFuncTable[0])();
	}
}

/**
 * 外部割込みハンドラ1
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
INTERRUPT void external_interrupt_1(void)
{
	if (g_afInterruptFuncTable[1] != NULL) {
		(*g_afInterruptFuncTable[1])();
	}
}


/**
 * A/D変換割り込みハンドラ
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
INTERRUPT void adc_interrupt(void)
{
	g_bAdcInterruptFlag = true;
}

/**
 * micros用TM05割り込みハンドラ
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
#if defined(REL_GR_KURUMI)
#ifdef __cplusplus
extern "C"
#endif
INTERRUPT void tm05_interrupt(void)
{
	g_timer05_overflow_count++;
}
#endif

/**
 * SoftwarePWM用TM06割り込みハンドラ
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
#if defined(REL_GR_KURUMI)
#ifdef __cplusplus
extern "C"
#endif
INTERRUPT void tm06_interrupt(void)
{
	_softwarePWM();
}

#endif
/// @endcond

/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/
#ifdef WORKAROUND_READ_MODIFY_WRITE
#define cbi(psfr, bit) \
{ \
	__asm __volatile( \
		"	movw	hl, %0					\n" \
		"	mov	a, %1					\n" \
		"	xor	a, #0xff				\n" \
		"	push	psw					\n" \
		"	di						\n" \
		"	and	a, [hl]					\n" \
		"	mov	[hl], a					\n" \
		"	pop	psw					\n" \
		: \
		:"r"(psfr), "r"(bit) \
		:"a", "h", "l" \
	); \
}

#define sbi(psfr, bit) \
{ \
	__asm __volatile( \
		"	movw	hl, %0					\n" \
		"	mov	a, %1					\n" \
		"	push	psw					\n" \
		"	di						\n" \
		"	or	a, [hl]					\n" \
		"	mov	[hl], a					\n" \
		"	pop	psw					\n" \
		: \
		:"r"(psfr), "r"(bit) \
		:"a", "h", "l" \
	); \
}
#endif


/**
 * Note: updated from V1.02
 * 指定した時間、setPowerManagementMode()関数で指定されたパワーマネージメントモードへ遷移します。
 *
 * setPowerManagementMode()関数で省電力(STOP/SNOOZE)モードを指定た場合、
 * delay()関数を呼び出すとSTOP命令によりスタンバイ状態に遷移し、 delay()
 * 関数の引数で指定した一時停止時間（u32ms）経過後にスタンバイ状態が解除され
 * delay()関数からリターンします。
 * また、一時停止時間（u32ms）に0xFFFFFFFFを設定した場合、何らかの割込み（除く
 * インターバルタイマ）かリセットが発生するまでSTOP命令によりスタンバイ状態に遷移し
 * delay()関数から戻りません。このときインターバルタイマも停止するため millis()
 * 関数、 micros()関数で取得できる時間情報も更新されません。

 *
 * @param[in] u32ms 一時停止する時間(ms)を指定します。
 *
 * @return なし
 *
 * @attention  Serial通信で送信時はSerial.flush()で、転送が完了されて
 * からdelay()を実行しないと、通信が完了する前にクロックが停止して正常に完了しません。
 * analogWriteやServo、Toneを使用している場合はHALTモードになります。
 ***************************************************************************/
static void enterPowerManagementMode(unsigned long u32ms)
{
	uint8_t  u8PMmode;


    // 設定された省電力モードとRL78の状態をチェックし、実際に発行できる命令を決定する。
    if (TE0.te0 & 0x00DE) {
        u8PMmode = PM_HALT_MODE;
    } else {
        u8PMmode = PM_STOP_MODE;
    }


	if (u32ms == 0xFFFFFFFF) {
		ITMK       = 1;			// Mask Interval Timer
		STOP();
		ITMK       = 0;			// Unmask Interval Timer
	}
	else {
		g_u32delay_timer = u32ms;
		TMMK05     = 1;
		//Note: TM05 stops during STOP, overflow count should be adjusted.
		//    : have a margin of error of approx. 50ms.
		g_timer05_overflow_count = g_timer05_overflow_count + (u32ms / MILLISECONDS_PER_TIMER05_OVERFLOW);
		do {
            if (u8PMmode == PM_STOP_MODE) {
                STOP();
            }
            else {
                HALT();
            }
		} while (g_u32delay_timer  != 0);
		TMMK05     = 0;

	}
}


/**
 * ピンの動作を入力か出力に設定します。
 *
 * Arduino 1.0.1から、INPUT_PULLUPを指定することで、内部プルアップ抵抗を有効
 * にできます。INPUTを指定すると、内部プルアップは無効となります。
 *
 * @param[in] u8Pin  設定したいピン番号を指定します。
 * @param[in] u8Mode ピンの動作モード（INPUT、OUTPUT、INPUT_PULLUP）を指定します。
 *
 * @return なし
 *
 * @attention
 * アナログ入力ピンはデジタルピンとしても使えます。アナログ入力ピン0がデジタ
 * ルピン14、アナログ入力ピン5がデジタルピン19に対応します。
 ***************************************************************************/
#ifdef USE_RTOS
static void _pinMode(uint8_t u8Pin, uint8_t u8Mode)
#else
void pinMode(uint8_t u8Pin, uint8_t u8Mode)
#endif
{
	if (u8Pin < NUM_DIGITAL_PINS) {
		// アナログピンかどうか？
		if (14 <= u8Pin && u8Pin <= 21) {
			// ピンモードをデジタルモードに変更
			if (u8Pin == 20) {
				PMC14.pmc14 &= ~0x80;// P147をデジタルポートに設定
			}
			else if (u8Pin == 21) {
				PMC12.pmc12 &= ~0x01;// P120をデジタルポートに設定
			}
			else {
				uint8_t oldadpc = ADPC.adpc;
				uint8_t newadpc = (u8Pin - 14) + ANALOG_ADPC_OFFSET - 1;
				if ((oldadpc  == 0x00 ) || (oldadpc > newadpc)) {
					ADPC.adpc = newadpc;
				}
			}
		}

		PinTableType* p = getPinTable(u8Pin);
		{
			if (g_u8AnalogWriteAvailableTable[u8Pin]) {
				_turnOffPwmPin(u8Pin);	// PWMの設定解除
			}
#ifdef WORKAROUND_READ_MODIFY_WRITE
#if 0
			if (u8Mode == INPUT) { 
				sbi(p->portModeRegisterAddr,  p->mask);	// 入力モードに設定
				sbi(p->portInputModeRegisterAddr, p->mask);	// TTL入力バッファに設定
				cbi(p->portPullUpRegisterAddr,  p->mask);	// プルアップ抵抗を無効に設定
			} else if (u8Mode == INPUT_PULLUP) {
				sbi(p->portModeRegisterAddr,  p->mask);	// 入力モードに設定
				cbi(p->portInputModeRegisterAddr, p->mask);	// CMOS入力バッファに設定
				sbi(p->portPullUpRegisterAddr,  p->mask);	// プルアップ抵抗を有効に設定
			} else {
				cbi(p->portModeRegisterAddr,  p->mask);	// 出力モードに設定
				cbi(p->portOutputModeRegisterAddr, p->mask);	// 通常出力モードに設定
			}
#else
			switch (u8Mode) {
			case INPUT:
			case INPUT_PULLUP:
			case INPUT_TTL:
			case INPUT_TTL_PULLUP:
				sbi(p->portModeRegisterAddr,  p->mask);	// 入力モードに設定
				if (u8Mode == INPUT_PULLUP || u8Mode == INPUT_TTL_PULLUP) {
					sbi(p->portPullUpRegisterAddr,  p->mask);	// プルアップ抵抗を有効に設定
				} else {
					cbi(p->portPullUpRegisterAddr,  p->mask);	// プルアップ抵抗を無効に設定
				}
				if (u8Mode == INPUT_TTL || u8Mode == INPUT_TTL_PULLUP) {
					sbi(p->portInputModeRegisterAddr, p->mask);	// TTL入力バッファに設定
				} else {
					cbi(p->portInputModeRegisterAddr, p->mask);	// CMOS入力バッファに設定
				}
				break;
			case OUTPUT:
			case OUTPUT_OPENDRAIN:
				cbi(p->portModeRegisterAddr,  p->mask);	// 出力モードに設定
				if (u8Mode == OUTPUT_OPENDRAIN) {
					sbi(p->portOutputModeRegisterAddr, p->mask);	// N-chオープン・ドレイン出力モードに設定
				} else {
					cbi(p->portOutputModeRegisterAddr, p->mask);	// 通常出力モードに設定
				}
				break;
			}
#endif
//			cbi(p->portRegisterAddr, p->mask);			// 出力をLOWに設定
#else
			if (u8Mode == INPUT) { 
				*p->portModeRegisterAddr  |= p->mask;		// 入力モードに設定
				*p->portInputModeRegisterAddr |= p->mask;		// TTL入力バッファに設定
				*p->portPullUpRegisterAddr  &= ~p->mask;	// プルアップ抵抗を無効に設定
			} else if (u8Mode == INPUT_PULLUP) {
				*p->portModeRegisterAddr  |= p->mask;		// 入力モードに設定
				*p->portInputModeRegisterAddr &= ~p->mask;	// CMOS入力バッファに設定
				*p->portPullUpRegisterAddr  |= p->mask;		// プルアップ抵抗を有効に設定
			} else {
				*p->portModeRegisterAddr  &= ~p->mask;	// 出力モードに設定
				*p->portOutputModeRegisterAddr &=  ~p->mask;	// 通常出力モードに設定
			}
//			*p->portRegisterAddr &= ~p->mask;			// 出力をLOWに設定
#endif
		}
	}
}


/**
 * HIGHまたはLOWを、指定したピンに出力します。
 *
 * 指定したピンが pinMode()関数でOUTPUTに設定されている場合は、次の電圧にセッ
 * トされます。
 * HIGH = 5V
 * LOW  = 0V (GND) 
 * 指定したピンがINPUTに設定されている場合は、HIGHを出力すると内部プルアップ
 * 抵抗が有効になります。LOWで内部プルアップは無効になります。
 *
 * @param[in] u8Pin   ピン番号を指定します。
 * @param[in] u8Value HIGHかLOWを指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
#ifdef USE_RTOS
static void _digitalWrite(uint8_t u8Pin, uint8_t u8Value)
#else
void digitalWrite(uint8_t u8Pin, uint8_t u8Value)
#endif
{
#if 0
	if (u8Pin < NUM_DIGITAL_PINS) {
		PinTableType* p = getPinTable(u8Pin);
		{
			if (*p->portModeRegisterAddr & p->mask) {
				// 入力モードの場合
#ifdef WORKAROUND_READ_MODIFY_WRITE
				if (u8Value == LOW) {
					cbi(p->portPullUpRegisterAddr, p->mask);	// プルアップ抵抗を無効に設定
					cbi(p->portRegisterAddr, p->mask);		// 出力をLOWに設定
				} else {
					sbi(p->portPullUpRegisterAddr, p->mask);	// プルアップ抵抗を有効に設定
					sbi(p->portRegisterAddr, p->mask);		// 出力をHIGHに設定
				}
#else
				if (u8Value == LOW) {
					*p->portPullUpRegisterAddr &= ~p->mask;		// プルアップ抵抗を無効に設定
				} else {
					*p->portPullUpRegisterAddr |= p->mask;		// プルアップ抵抗を有効に設定
				}
#endif
			}
			else {
				// 出力モードの場合
#ifdef WORKAROUND_READ_MODIFY_WRITE
				if (u8Value == LOW) {
					cbi(p->portRegisterAddr, p->mask);		// 出力をLOWに設定
				} else {
					sbi(p->portRegisterAddr, p->mask);		// 出力をHIGHに設定
				}
#else
				if (u8Value == LOW) {
					*p->portRegisterAddr &= ~p->mask;		// 出力をLOWに設定
				} else {
					*p->portRegisterAddr |= p->mask;		// 出力をHIGHに設定
				}
#endif
			}
		}
	}
#else
	__asm __volatile(
	    "\n"
	    "        /* %%0 = %0 */                  \n"
	    "        /* %%1 = %1 */                  \n"
	    "        /* %%2 = %2 */                  \n"
	    "        /* %%3 = %3 */                  \n"
	    "        /* %%4 = %4 */                  \n"
	    "        /* %%5 = %5 */                  \n"
	    "        /* %%6 = %6 */                  \n"
	    "        /* %%7 = %7 */                  \n"
	    "        /* %%8 = %8 */                  \n"
	    "        mov     a, %0                   \n"
	    "        cmp     a, %1                   \n"
	    "        bnc     $8f                     \n"
	    "        mov     x, %2                   \n"
	    "        mulu    x                       \n"
	    "        movw    bc, ax                  \n"
	    "        movw    ax, %u3[bc]             \n"
	    "        movw    hl, ax                  \n"
	    "        mov     a, %u7[bc]              \n"
	    "        and     a, [hl]                 \n"
	    "        movw    ax, %u5[bc]             \n"
	    "        skz                             \n"
	    "        movw    ax, %u4[bc]             \n"
	    "        movw    hl, ax                  \n"
	    "        mov     a, %8                   \n"
	    "        add     a, #0xff                \n"
	    "        mov     a, %u6[bc]              \n"
	    "        bt      a.2, $4f                \n"
	    "        bt      a.1, $2f                \n"
	    "        bt      a.0, $1f                \n"
	    "        mov1    [hl].0, cy              \n"
	    "        br      $8f                     \n"
	    "1:                                      \n"
	    "        mov1    [hl].1, cy              \n"
	    "        br      $8f                     \n"
	    "2:                                      \n"
	    "        bt      a.0, $3f                \n"
	    "        mov1    [hl].2, cy              \n"
	    "        br      $8f                     \n"
	    "3:                                      \n"
	    "        mov1    [hl].3, cy              \n"
	    "        br      $8f                     \n"
	    "4:                                      \n"
	    "        bt      a.1, $6f                \n"
	    "        bt      a.0, $5f                \n"
	    "        mov1    [hl].4, cy              \n"
	    "        br      $8f                     \n"
	    "5:                                      \n"
	    "        mov1    [hl].5, cy              \n"
	    "        br      $8f                     \n"
	    "6:                                      \n"
	    "        bt      a.0, $7f                \n"
	    "        mov1    [hl].6, cy              \n"
	    "        br      $8f                     \n"
	    "7:                                      \n"
	    "        mov1    [hl].7, cy              \n"
	    "8:                                      \n"
	    :
	    : "m"(u8Pin),
	      "i"(NUM_DIGITAL_PINS),
	      "i"((unsigned)&PinTable[1] - (unsigned)&PinTable[0]),
	      "i"(&PinTable->portModeRegisterAddr),
	      "i"(&PinTable->portPullUpRegisterAddr),
	      "i"(&PinTable->portRegisterAddr),
	      "i"(&PinTable->bit),
	      "i"(&PinTable->mask),
	      "m"(u8Value)
	    : "a", "x", "b", "c", "h", "l"
	);
#endif
}


/**
 * 指定したピンの値を読み取ります。
 *
 * その結果はHIGHまたはLOWとなります。
 *
 * @param[in] u8Pin ピン番号を指定します。
 *
 * @return HIGHまたはLOWを返却します。
 *
 * @attention
 * なにも接続していないピンを読み取ると、HIGHとLOWがランダムに現れることがあ
 * ります。
 ***************************************************************************/
#ifdef USE_RTOS
static int _digitalRead(uint8_t u8Pin)
#else
int digitalRead(uint8_t u8Pin)
#endif
{
#if 0
	int	s16Value;
	if (u8Pin < NUM_DIGITAL_PINS) {
		PinTableType* p = getPinTable(u8Pin);
		if (*p->portRegisterAddr & p->mask) {
#if 1
			__asm __volatile(
				"movw %0, %1 \n"
				: "=r"(s16Value)
				: "i"(HIGH)
			);
			return s16Value;
#else
			return HIGH;
#endif
		}
	}

#if 1
	__asm __volatile(
		"movw %0, %1 \n"
		: "=r"(s16Value)
		: "i"(LOW)
	);
	return s16Value;
#else
	return LOW;
#endif
#else
	int value = 0;
	__asm __volatile(
	    "\n"
	    "        /* %%0 = %0 */                 \n"
	    "        /* %%1 = %1 */                 \n"
	    "        /* %%2 = %2 */                 \n"
	    "        /* %%3 = %3 */                 \n"
	    "        /* %%4 = %4 */                 \n"
	    "        /* %%5 = %5 */                 \n"
	    "        /* %%6 = %6 */                 \n"
	    "        mov     a, %2                  \n"
	    "        cmp     a, %3                  \n"
	    "        bnc     $9f                    \n"
	    "        mov     x, %4                  \n"
	    "        mulu    x                      \n"
	    "        movw    bc, ax                 \n"
	    "        movw    ax, %u5[bc]            \n"
	    "        movw    hl, ax                 \n"
	    "        mov     a, %u6[bc]             \n"
	    "        and     a, [hl]                \n"
	    "        add     a, #0xff               \n"
	    "        mov1    %1.0, cy               \n"
	    "9:                                     \n"
	    : "=r"(value)
	    : "r"(value),
	      "m"(u8Pin),
	      "i"(NUM_DIGITAL_PINS),
	      "i"((unsigned)&PinTable[1] - (unsigned)&PinTable[0]),
	      "i"(&PinTable->portRegisterAddr),
	      "i"(&PinTable->mask)
	    : "a", "x", "b", "c", "h", "l"
	);
	return value;
#endif
}


/**
 * 指定したアナログピンから値を読み取ります。
 *
 * @param[in] u8Pin 読みたいピン番号を指定します。
 *
 * @return 0から1023までの整数値を返却します。
 *
 * @attention
 ***************************************************************************/
static int _analogRead(uint8_t u8ADS)
{
	int	s16Result = 0;

	if ((( 0 <= u8ADS) && (u8ADS <= 14)) ||
		((16 <= u8ADS) && (u8ADS <= 26)) ||
		(u8ADS == ADS_TEMP_SENSOR) || (u8ADS == ADS_REF_VOLTAGE)) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
		SBI2(SFR2_PER0, SFR2_BIT_ADCEN);// A/Dコンバータにクロック供給開始
		ADM0.adm0 = 0x00;		// A/Dコンバータの動作停止、fclk/64、ノーマル1モードに設定
		SBI(SFR_MK1H, 0);		// INTADの割り込み禁止
		CBI(SFR_IF1H, 0);		// INTADの割り込みフラグのクリア
		SBI(SFR_PR11H, 1);		// INTADの割り込み優先順位の設定
		SBI(SFR_PR01H, 1);
#else /* WORKAROUND_READ_MODIFY_WRITE */
		ADCEN     = 1;			// A/Dコンバータにクロック供給開始
		NOP();
		NOP();
		NOP();
		NOP();
		ADM0.adm0 = 0x00;		// A/Dコンバータの動作停止、fclk/64、ノーマル1モードに設定
		ADMK      = 1;			// INTADの割り込み禁止
		ADIF      = 0;			// INTADの割り込みフラグのクリア
		ADPR1     = 1;			// INTADの割り込み優先順位の設定
		ADPR0     = 1;			// INTADの割り込み優先順位の設定
#endif
		if ((u8ADS == ADS_TEMP_SENSOR) || (u8ADS == ADS_REF_VOLTAGE)) {
			ADM2.adm2 = 0x00;	// Vddリファレンスに設定
		}
		else if (g_u8AnalogReference == EXTERNAL) {
			ADM2.adm2 = 0x40;	// 外部リファレンスに設定
		}
		else if (g_u8AnalogReference == INTERNAL) {
			ADM2.adm2 = 0x80;	// 内部リファレンス(1.45V)に設定
		}
		else {
			ADM2.adm2 = 0x00;	// Vddリファレンスに設定
		}
		if (g_u8PowerManagementMode == PM_SNOOZE_MODE) {
			ADM1.adm1  = 0xE3;	// ハードウェア・トリガ(INTIT)・ウェイト･モード、ワンショットに設定
		} else {
			ADM1.adm1  = 0x20;	// ソフトウェア・トリガ・モード、ワンショットに設定
		}
		ADUL.adul = g_u8ADUL;
		ADLL.adll = g_u8ADLL;
		ADS.ads   = u8ADS;		// アナログチャンネルの設定
		delayMicroseconds(5);	// 5 us 待ち
#ifdef WORKAROUND_READ_MODIFY_WRITE
		SBI(SFR_ADM0, SFR_BIT_ADCE);// A/Dコンパレータを有効に設定
#else /* WORKAROUND_READ_MODIFY_WRITE */
		ADCE      = 1;			// A/Dコンパレータを有効に設定
#endif
		if (g_u8PowerManagementMode == PM_SNOOZE_MODE) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
			CBI(SFR_MK1H, 0);	// INTADの割り込み許可
			ADM2.adm2 |= 0x04;	// SNOOZEモードの設定
#else /* WORKAROUND_READ_MODIFY_WRITE */
			ADMK  = 0;			// INTADの割り込み許可
			ADM2.adm2 |= 0x04;	// SNOOZEモードの設定
#endif
			while (g_bAdcInterruptFlag == false) {
				enterPowerManagementMode(0xFFFFFFFF);// A/Dコンバート待ち
			}
			ADM2.adm2 &= ~0x04;	// SNOOZEモードの設定解除
			g_bAdcInterruptFlag = false;
		} else {
			delayMicroseconds(1);// 1 us 待ち
#ifdef WORKAROUND_READ_MODIFY_WRITE
			SBI(SFR_ADM0, SFR_BIT_ADCS);// A/Dコンバータの開始
#else /* WORKAROUND_READ_MODIFY_WRITE */
			ADCS      = 1;		// A/Dコンバータの開始
#endif
			while (ADIF == 0);	// A/Dコンバート待ち
		}
		s16Result = (ADCR.adcr >> 6);// A/Dコンバート結果の取得

		if ((u8ADS == ADS_TEMP_SENSOR) || (u8ADS == ADS_REF_VOLTAGE)) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
			CBI(SFR_IF1H, 0);	// INTADの割り込みフラグのクリア
			SBI(SFR_ADM0, SFR_BIT_ADCS);// A/Dコンバータの開始
#else /* WORKAROUND_READ_MODIFY_WRITE */
			ADIF      = 0;		// INTADの割り込みフラグのクリア
			ADCS      = 1;		// A/Dコンバータの開始
#endif
			while (ADIF == 0);	// A/Dコンバート待ち
			s16Result = (ADCR.adcr >> 6);// A/Dコンバート結果の取得
		}
#ifdef WORKAROUND_READ_MODIFY_WRITE
		SBI(SFR_MK1H, 0);		// INTADの割り込み禁止
		CBI(SFR_IF1H, 0);		// INTADの割り込みフラグのクリア
		CBI(SFR_ADM0, SFR_BIT_ADCE);// A/Dコンパレータを無効に設定
		CBI2(SFR2_PER0, SFR2_BIT_ADCEN);// A/Dコンバータにクロック供給停止
#else /* WORKAROUND_READ_MODIFY_WRITE */
		ADMK      = 1;			// INTADの割り込み禁止
		ADIF      = 0;			// INTADの割り込みフラグのクリア
		ADCE      = 0;			// A/Dコンパレータを無効に設定
		ADCEN     = 0;			// A/Dコンバータのクロック供給停止
#endif
	}

	return s16Result;
}


/**
 * タイマーアレイユニットの開始
 *
 * @param[in] u8TimerCloc タイマクロックを指定してください。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
static void _startTAU0(uint16_t u16TimerClock)
{
	// タイマ・アレイ・ユニットが動作しているか？
	if (TAU0EN == 0) {
		// タイマ・アレイ・ユニット0の設定
#ifdef WORKAROUND_READ_MODIFY_WRITE
		SBI2(SFR2_PER0, SFR2_BIT_TAU0EN);// タイマ・アレイ・ユニットにクロック供給開始
#else	/* WORKAROUND_READ_MODIFY_WRITE*/
		TAU0EN    = 1;   			// タイマ・アレイ・ユニットにクロック供給開始
#endif
		NOP();
		NOP();
		NOP();
		NOP();
		TPS0.tps0 = u16TimerClock;	// タイマ・クロック周波数を設定
	}
}


/**
 * タイマーアレイユニットの停止
 *
* @return なし
 *
 * @attention なし
 ***************************************************************************/
static void _stopTAU0()
{
	// タイマ・アレイ・ユニットが動作しているか？
	if (TAU0EN != 0) {
		if (TE0.te0 == 0x00000) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
			CBI2(SFR2_PER0, SFR2_BIT_TAU0EN);// タイマ・アレイ・ユニットにクロック供
#else	/* WORKAROUND_READ_MODIFY_WRITE*/
			TAU0EN    = 0;   			// タイマ・アレイ・ユニットにクロック供給停止
#endif
		}
	}
}

/**
 * タイマーチャンネルの開始
 *
 * @param[in] u8Timer 開始するタイマ番号
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
static void _startTimerChannel(uint8_t u8TimerChannel, uint16_t u16TimerMode, uint16_t u16Interval, bool bPWM, bool bInterrupt)
{
#ifdef WORKAROUND_READ_MODIFY_WRITE
	TT0.tt0   |= (1 << u8TimerChannel);	// タイマの停止
	switch (u8TimerChannel) {
	case 1:
		SBI(SFR_MK1L,    5);		// 割り込みマスクを禁止に設定
		CBI(SFR_IF1L,    5);		// 割り込み要求フラグのクリア
		SBI(SFR_PR11L,   5);		// 割り込み優先順位の設定
		SBI(SFR_PR01L,   5);
		TMR01.tmr01 = u16TimerMode;	// タイマ・チャネルの動作モードの設定
		TDR01.tdr01 = u16Interval;	// インターバル（周期）の設定
		if (bInterrupt == true) {
			CBI(SFR_MK1L,  5);		// 割り込みマスクを許可に設定
		}

		break;

	case 2:
		SBI(SFR_MK1L,    6);		// 割り込みマスクを禁止に設定
		CBI(SFR_IF1L,    6);		// 割り込み要求フラグのクリア
		SBI(SFR_PR11L,   6);		// 割り込み優先順位の設定
		SBI(SFR_PR01L,   6);
		TMR02.tmr02 = u16TimerMode;	// タイマ・チャネルの動作モードの設定
		TDR02.tdr02 = u16Interval;	// インターバル（周期）の設定
		if (bInterrupt == true) {
			CBI(SFR_MK1L,  6);		// 割り込みマスクを許可に設定
		}

		break;

	case 3:
		SBI(SFR_MK1L,    7);		// 割り込みマスクを禁止に設定
		CBI(SFR_IF1L,    7);		// 割り込み要求フラグのクリア
		SBI(SFR_PR11L,   7);		// 割り込み優先順位の設定
		SBI(SFR_PR01L,   7);
		TMR03.tmr03 = u16TimerMode;	// タイマ・チャネルの動作モードの設定
		TDR03.tdr03 = u16Interval;	// インターバル（周期）の設定
		if (bInterrupt == true) {
			CBI(SFR_MK1L,  7);		// 割り込みマスクを許可に設定
		}

		break;

	case 4:
		SBI(SFR_MK1H,    7);		// 割り込みマスクを禁止に設定
		CBI(SFR_IF1H,    7);		// 割り込み要求フラグのクリア
		SBI(SFR_PR11H,   7);		// 割り込み優先順位の設定
		SBI(SFR_PR01H,   7);
		TMR04.tmr04 = u16TimerMode;	// タイマ・チャネルの動作モードの設定
		TDR04.tdr04 = u16Interval;	// インターバル（周期）の設定
		if (bInterrupt == true) {
			CBI(SFR_MK1H,  7);		// 割り込みマスクを許可に設定
		}

		break;

	case 5:
		SBI(SFR_MK2L,    0);		// 割り込みマスクを禁止に設定
		CBI(SFR_IF2L,    0);		// 割り込み要求フラグのクリア
		SBI(SFR_PR12L,   0);		// 割り込み優先順位の設定
		SBI(SFR_PR02L,   0);
		TMR05.tmr05 = u16TimerMode;	// タイマ・チャネルの動作モードの設定
		TDR05.tdr05 = u16Interval;	// インターバル（周期）の設定
		if (bInterrupt == true) {
			CBI(SFR_MK2L,  0);		// 割り込みマスクを許可に設定
		}

		break;

	case 6:
		SBI(SFR_MK2L,    1);		// 割り込みマスクを禁止に設定
		CBI(SFR_IF2L,    1);		// 割り込み要求フラグのクリア
		SBI(SFR_PR12L,   1);		// 割り込み優先順位の設定
		SBI(SFR_PR02L,   1);
		TMR06.tmr06 = u16TimerMode;	// タイマ・チャネルの動作モードの設定
		TDR06.tdr06 = u16Interval;	// インターバル（周期）の設定
		if (bInterrupt == true) {
			CBI(SFR_MK2L,  1);		// 割り込みマスクを許可に設定
		}

		break;

	case 7:
		SBI(SFR_MK2L,    2);		// 割り込みマスクを禁止に設定
		CBI(SFR_IF2L,    2);		// 割り込み要求フラグのクリア
		SBI(SFR_PR12L,   2);		// 割り込み優先順位の設定
		SBI(SFR_PR02L,   2);
		TMR07.tmr07 = u16TimerMode;	// タイマ・チャネルの動作モードの設定
		TDR07.tdr07 = u16Interval;	// インターバル（周期）の設定
		if (bInterrupt == true) {
			CBI(SFR_MK2L,  2);		// 割り込みマスクを許可に設定
		}

		break;
	}
	if (bPWM == true) {
		TOM0.tom0 |=  (1 << u8TimerChannel);// タイマ出力モードの設定
	} else {
		TOM0.tom0 &= ~(1 << u8TimerChannel);// タイマ出力モードの設定
	}

	if (bInterrupt == true) {
		TOE0.toe0 &= ~(1 << u8TimerChannel);// タイマ出力禁止の設定
	} else {
		TOE0.toe0 |=  (1 << u8TimerChannel);// タイマ出力許可の設定

	}
	TS0.ts0   |=  (1 << u8TimerChannel);// タイマ動作許可

#else	/* WORKAROUND_READ_MODIFY_WRITE */
	TT0.tt0   |= (1 << u8TimerChannel);	// タイマの停止
	switch (u8TimerChannel) {
	case 1:
		TMMK01      = 1;				// 割り込みマスクを禁止に設定
		TMIF01      = 0;				// 割り込み要求フラグのクリア
		TMPR101     = 1;				// 割り込み優先順位の設定
		TMPR001     = 1;
		TMR01.tmr01 = u16TimerMode;		// タイマ・チャネルの動作モードの設定
		TDR01.tdr01 = u16Interval;		// インターバル（周期）の設定
		if (bInterrupt == true) {
			TMMK01  = 0;				// 割り込みマスクを許可に設定
		}
		break;

	case 2:
		TMMK02      = 1;				// 割り込みマスクを禁止に設定
		TMIF02      = 0;				// 割り込み要求フラグのクリア
		TMPR102     = 1;				// 割り込み優先順位の設定
		TMPR002     = 1;
		TMR02.tmr02 = u16TimerMode;		// タイマ・チャネルの動作モードの設定
		TDR02.tdr02 = u16Interval;		// インターバル（周期）の設定
		if (bInterrupt == true) {
			TMMK02  = 0;				// 割り込みマスクを許可に設定
		}
		break;

	case 3:
		TMMK03      = 1;				// 割り込みマスクを禁止に設定
		TMIF03      = 0;				// 割り込み要求フラグのクリア
		TMPR103     = 1;				// 割り込み優先順位の設定
		TMPR003     = 1;
		TMR03.tmr03 = u16TimerMode;		// タイマ・チャネルの動作モードの設定
		TDR03.tdr03 = u16Interval;		// インターバル（周期）の設定
		if (bInterrupt == true) {
			TMMK03  = 0;				// 割り込みマスクを許可に設定
		}
		break;

	case 4:
		TMMK04      = 1;				// 割り込みマスクを禁止に設定
		TMIF04      = 0;				// 割り込み要求フラグのクリア
		TMPR104     = 1;				// 割り込み優先順位の設定
		TMPR004     = 1;
		TMR04.tmr04 = u16TimerMode;		// タイマ・チャネルの動作モードの設定
		TDR04.tdr04 = u16Interval;		// インターバル（周期）の設定
		if (bInterrupt == true) {
			TMMK04  = 0;				// 割り込みマスクを許可に設定
		}
		break;

	case 5:
		TMMK05      = 1;				// 割り込みマスクを禁止に設定
		TMIF05      = 0;				// 割り込み要求フラグのクリア
		TMPR105     = 1;				// 割り込み優先順位の設定
		TMPR005     = 1;
		TMR05.tmr05 = u16TimerMode;		// タイマ・チャネルの動作モードの設定
		TDR05.tdr05 = u16Interval;		// インターバル（周期）の設定
		if (bInterrupt == true) {
			TMMK05  = 0;				// 割り込みマスクを許可に設定
		}
		break;

	case 6:
		TMMK06      = 1;				// 割り込みマスクを禁止に設定
		TMIF06      = 0;				// 割り込み要求フラグのクリア
		TMPR106     = 1;				// 割り込み優先順位の設定
		TMPR006     = 1;
		TMR06.tmr06 = u16TimerMode;		// タイマ・チャネルの動作モードの設定
		TDR06.tdr06 = u16Interval;		// インターバル（周期）の設定
		if (bInterrupt == true) {
			TMMK06  = 0;				// 割り込みマスクを許可に設定
		}
		break;

	case 7:
		TMMK07      = 1;				// 割り込みマスクを禁止に設定
		TMIF07      = 0;				// 割り込み要求フラグのクリア
		TMPR107     = 1;				// 割り込み優先順位の設定
		TMPR007     = 1;
		TMR07.tmr07 = u16TimerMode;		// タイマ・チャネルの動作モードの設定
		TDR07.tdr07 = u16Interval;		// インターバル（周期）の設定
		if (bInterrupt == true) {
			TMMK07  = 0;				// 割り込みマスクを許可に設定
		}
		break;
	}

	if (bPWM == true) {
		TOM0.tom0 |=  (1 << u8TimerChannel);// タイマ出力モードの設定
	} else {
		TOM0.tom0 &= ~(1 << u8TimerChannel);// タイマ出力モードの設定
	}

	if (bInterrupt == true) {
		TOE0.toe0 &= ~(1 << u8TimerChannel);// タイマ出力禁止の設定
	} else {
		TOE0.toe0 |=  (1 << u8TimerChannel);// タイマ出力許可の設定

	}
	TS0.ts0   |=  (1 << u8TimerChannel);// タイマ動作許可
#endif
}

/**
 * タイマー周期の変更
 *
 * @param[in] u8Timer 変更するタイマ番号
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
static void _modifyTimerPeriodic(uint8_t u8TimerChannel, uint16_t u16Interval)
{
	switch (u8TimerChannel) {
	case 1:
		TDR01.tdr01 = u16Interval;	// インターバル（周期）の設定
		break;

	case 2:
		TDR02.tdr02 = u16Interval;	// インターバル（周期）の設定
		break;

	case 3:
		TDR03.tdr03 = u16Interval;	// インターバル（周期）の設定
		break;

	case 4:
		TDR04.tdr04 = u16Interval;	// インターバル（周期）の設定
		break;

	case 5:
		TDR05.tdr05 = u16Interval;	// インターバル（周期）の設定
		break;

	case 6:
		TDR06.tdr06 = u16Interval;	// インターバル（周期）の設定
		break;

	case 7:
		TDR07.tdr07 = u16Interval;	// インターバル（周期）の設定
		break;
	}

}


/**
 * タイマーチャンネルの停止
 *
 * @param[in] u8Timer 停止するタイマ番号
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
static void _stopTimerChannel(uint8_t u8TimerChannel)
{
#ifdef WORKAROUND_READ_MODIFY_WRITE
	TT0.tt0   |=  (1 << u8TimerChannel);	// タイマ動作停止
	TOE0.toe0 &=  ~(1 << u8TimerChannel);	// タイマ出力禁止の設定
	TO0.to0   &= ~(1 << u8TimerChannel);	// タイマ出力の設定

	switch (u8TimerChannel) {
	case 1:	SBI(SFR_MK1L,    5);// 割り込みマスクを禁止に設定
		break;

	case 2:	SBI(SFR_MK1L,    6);// 割り込みマスクを禁止に設定
		break;

	case 3:	SBI(SFR_MK1L,    7);// 割り込みマスクを禁止に設定
		break;

	case 4:	SBI(SFR_MK1H,    7);// 割り込みマスクを禁止に設定
		break;

	case 5: SBI(SFR_MK2L,    0);// 割り込みマスクを禁止に設定
		break;

	case 6:	SBI(SFR_MK2L,    1);// 割り込みマスクを禁止に設定
		break;

	case 7:	SBI(SFR_MK2L,    2);// 割り込みマスクを禁止に設定
		break;
	}
	if (!(TE0.te0 & 0x009E)) {
		TT0.tt0 |= 0x0001;		// Master チャンネルの停止
	}
#else /* WORKAROUND_READ_MODIFY_WRITE */
	TT0.tt0   |=  (1 << u8TimerChannel);	// タイマ動作停止
	TOE0.toe0 &=  ~(1 << u8TimerChannel);	// タイマ出力禁止の設定
	TO0.to0   &= ~(1 << u8TimerChannel);	// タイマ出力の設定
	// 割り込みマスクを禁止に設定
	switch (u8TimerChannel) {
	case 1:	TMMK01  = 1; break;
	case 2:	TMMK02  = 1; break;
	case 3:	TMMK03  = 1; break;
	case 4:	TMMK04  = 1; break;
	case 5:	TMMK05  = 1; break;
	case 6:	TMMK06  = 1; break;
	case 7:	TMMK07  = 1; break;
	}
	if (!(TE0.te0 & 0x009E)) {
		TT0.tt0 |= 0x0001;		// Master チャンネルの停止
	}
#endif
}


/**
 * PWMの停止
 *
 * @param[in] u8Timer 停止するPWMのタイマ番号
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
static void _turnOffPwmPin(uint8_t u8Pin)
{
	unsigned int u16TMR0x;

	uint8_t u8Timer = 0xFF;
	if (u8Pin < NUM_DIGITAL_PINS) {
		u8Timer = getPinTable(u8Pin)->timer;
		g_u8AnalogWriteAvailableTable[u8Pin] = false;
	}
	if (u8Timer == SWPWM_PIN) {
		///////////////////////
		// Software PWM対応ピンの場合
		///////////////////////
#if defined(REL_GR_KURUMI)
		int i;

		for (i = 0; i < NUM_SWPWM_PINS; i++) {
			if (g_SwPwm[i].valid) {
				if (g_SwPwm[i].pin == u8Pin) {
					g_SwPwm[i].valid = false;
				} else {
					break;
				}
			}
		}
		if (i >= NUM_SWPWM_PINS) {			// SoftwarePWMの設定なし
			_stopTimerChannel(SW_PWM_TIMER);
		}
#endif
	} else {
		///////////////////////
		// PWM対応ピンの場合
		///////////////////////
		switch (u8Timer) {
		case 1:
			u16TMR0x = TMR01.tmr01;
			break;

		case 2:
			u16TMR0x = TMR02.tmr02;
			break;

		case 3:
			u16TMR0x = TMR03.tmr03;
			break;

		case 4:
			u16TMR0x = TMR04.tmr04;
			break;

		case 5:
			u16TMR0x = TMR05.tmr05;
			break;

		case 6:
			u16TMR0x = TMR06.tmr06;
			break;

		case 7:
			u16TMR0x = TMR07.tmr07;
			break;

		default:
			u16TMR0x = PWM_MASTER_MODE;
			break;
		}
		if (u16TMR0x == PWM_SLAVE_MODE) {
			_stopTimerChannel(u8Timer);
		}
	}
}


#if defined(REL_GR_KURUMI)
/**
 * SoftwarePWMの次回割り込みタイミングの設定処理
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
static void _softwarePWM(void)
{
	for (SwPwm* p = (SwPwm*)&g_SwPwm[0]; p < (SwPwm*)&g_SwPwm[NUM_SWPWM_PINS]; p++) {
		if (p->valid) {
			if (g_u8SwPwmTicksCount == PWM_MIN) {
				p->value = p->newValue;
			}
			if (g_u8SwPwmTicksCount == p->value) {
				*p->port &= ~p->mask;	// 出力をLOWに設定
			} else if (g_u8SwPwmTicksCount == PWM_MIN) {
				*p->port |= p->mask;	// 出力をHIGHに設定
			}
		}
	}
	if (++g_u8SwPwmTicksCount >= PWM_MAX) {
		g_u8SwPwmTicksCount = PWM_MIN;
	};
}

/**
 * Software Reset
 *
 * Generate software reset.
 *
 * @return なし
 *
 * @attention:  This function uses trap instruction.
 *              So RESF register in MCU is set to 0x80 after reset.
 *              Unable to use when using debugger.
 *
 ***************************************************************************/

void softwareReset(void) {
	__asm __volatile(" .byte 0xff");
}

/**
 * Get Reset Flag
 *
 * Get the flag why reset occur.
 *
 * @return 0x00: External reset or Power on reset
 *         0x01: Low voltage detection
 *         0x02: Illegal memory access
 *         0x04: RAM parity error ( read from no initialized area )
 *         0x10: Watch dog timer
 *         0x80: Illegal instruction
 *
 * @attention:
 *
 ***************************************************************************/

uint8_t getResetFlag() {
	return g_u8ResetFlag;
}

#endif



/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
