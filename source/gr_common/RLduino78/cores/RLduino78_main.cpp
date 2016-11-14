/***************************************************************************
 *
 * PURPOSE
 *   RLduino78 framework main module file.
 *
 * TARGET DEVICE
 *   RL78/G13
 *
 * AUTHOR
 *   Renesas Solutions Corp.
 *
 * $Date:: 2013-01-25 14:32:21 +0900#$
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
 * @file  RLduino78_main.cpp
 * @brief RLduino78フレームワーク・メイン・モジュール・ファイル
 */
/* Modified by Yuuki Okamiya : Jan 16, 2015*/

/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include "RLduino78_mcu_depend.h"
#include "RLduino78_timer.h"
#ifdef USE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#endif


/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/
int initArduinoFramework();
void execCyclicHandler();
static void vArduinoTask();


/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/
uint8_t g_u8ResetFlag;


/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/
/**
 * H/W初期化関数
 *
 * この関数では以下のH/Wの初期設定を行います。
 * - 周辺I/Oリダイレクションレジスタの設定
 * - ポート・モード・コントロール・レジスタの設定
 * - 動作クロックの設定
 * - インターバル・タイマーの設定
 *
 * @return 返り値なし
 *
 * @attention この関数はスタートアップルーチンから呼び出されます。
 ***************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
void HardwareSetup(void)
{
	// リセット要因の保存
	g_u8ResetFlag = RESF.resf;

	// 割り込みの禁止
	noInterrupts();

#if defined(REL_GR_KURUMI)
	// 周辺I/Oリダイレクション・レジスタの設定
	PIOR.pior = 0x09;	// PCLBUZ0とTI0x/TO0xをリダイレクション
	RPECTL.rpectl = 0x80;
#elif defined(REL_GR_KURUMI_PROTOTYPE)
	// ポート・モード・コントロール・レジスタの設定
	PMC0.pmc0 = 0xF3;
#endif

	// 動作クロックの初期化
	init_system_clock();

	// タイマーの初期化
	init_interval_timer();

	// 割り込みの許可
	interrupts();

	return;
}


/**
 * メイン関数
 *
 * RLduino78フレームワーク処理を呼び出します。
 *
 * @return 返り値なし
 *
 * @attention この関数はスタートアップルーチンから呼び出されます。
 ***************************************************************************/
int main( void )
{
#ifdef USE_RTOS
	// Arduinoフレームワークタスクの生成
	xTaskCreate(vArduinoTask, (signed char*)"ArduinoTask", 512 / sizeof(portSTACK_TYPE), NULL, 0, NULL);

	// スケジューラの起動
	vTaskStartScheduler();

	for( ;; );
#else
	// Arduinoフレームワークタスクの呼び出し
	vArduinoTask();
#endif
	return 0;
}

/**
 * 終了関数
 *
 * RLduino78フレームワーク終了処理
 *
 * @return 返り値なし
 *
 * @attention 呼び出されたが最後帰ってきません。
 ***************************************************************************/
#include <stdlib.h>

void abort(void)
{
	for(;;);
}

void exit(int) __attribute__ ((weak, alias ("abort")));

#include <exception>

void std::terminate() __attribute__ ((weak, alias ("abort")));

#ifdef USE_RTOS
/**
 * メモリ確保失敗フック関数
 *
 * pvPortMalloc()関数の呼び出しに失敗した場合に呼び出されます。これはFreeRTOSの
 * ヒープ中の有効なフリーメモリが不十分であることを意味します。
 * pvPortMalloc()関数は、タスク、キュー、ソフトウェアタイマ、セマフォを生成する
 * FreeRTOS APIにより内部的に呼び出されます。FreeRTOSのヒープのサイズは
 * FreeRTOSConfig.hのconfigTOTAL_HEAP_SIZEにより設定されます。
 * 
 * @return 返り値なし
 *
 * @attention なし
 ***************************************************************************/
void vApplicationMallocFailedHook( void )
{
	taskDISABLE_INTERRUPTS();
	for( ;; );
}


/**
 * スタックオーバーフロー検出フック関数
 *
 * configCHECK_FOR_STACK_OVERFLOWが"1"または"2"に定義された場合、スタックオーバー
 * フローを検出した時にこの関数が呼び出されます。
 *
 * @return 返り値なし
 *
 * @attention なし
 ***************************************************************************/
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;
	taskDISABLE_INTERRUPTS();
	for( ;; );
}


/**
 * IDLEフック関数
 *
 * IDLEタスク毎にこの関数が呼び出されます。
 * この関数ではFreeRTOSのヒープの残りの量を問い合わせるだけです。
 *
 * @return 返り値なし
 *
 * @attention なし
 ***************************************************************************/
void vApplicationIdleHook( void )
{
	volatile size_t xFreeHeapSpace;
	xFreeHeapSpace = xPortGetFreeHeapSize();
}
#endif


/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/
/**
 * Arduino フレームワークタスク
 *
 * ユーザ定義のsetup関数とloop関数を呼び出します。
 *
 * @return 返り値なし
 *
 * @attention なし
 ***************************************************************************/
static void vArduinoTask()
{
	int s16Result;

	// フレームワークの初期化
	s16Result = initArduinoFramework();

	if (s16Result == 0) {
		// ユーザーのセットアップ関数の呼び出し
		setup();

		for (;;) {
			// ユーザーのメインループ処理の呼び出し
			loop();

			// 周期起動ハンドラの呼び出し
			execCyclicHandler();
		}
	}
}


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
