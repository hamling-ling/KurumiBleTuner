/***************************************************************************
 *
 * PURPOSE
 *   RLduino78 framework interval timer module file.
 *
 * TARGET DEVICE
 *   RL78/G13
 *
 * AUTHOR
 *   Renesas Solutions Corp.
 *
 * $Date:: 2013-03-27 11:42:33 +0900#$
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
 * @file  RLduino78_timer.c
 * @brief RLduino78フレームワーク・インターバル・タイマ・モジュール・ファイル
 */
/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include "RLduino78_mcu_depend.h"
#include "RLduino78_timer.h"


/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/


/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/
volatile unsigned long g_u32timer_millis = 0;	//!< インターバルタイマ変数
volatile unsigned long g_u32delay_timer  = 0;	//!< delay() 用タイマ変数
volatile unsigned long g_timer05_overflow_count = 0;//

fITInterruptFunc_t	g_fITInterruptFunc = NULL;	//!< ユーザー定義インターバルタイマハンドラ


/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/



/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/
/**
 * システムクロックの初期化
 * ・メイン・システム・クロック　＝　高速オンチップ・オシレータ
 * ・サブ・システム・クロック　　＝　XT1発振回路
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void init_system_clock()
{
#if (RTC_CLK_SOURCE == CLK_SOURCE_XT1)
	unsigned long i;
	CMC.cmc   = 0x10;	// クロック動作モード制御レジスタの設定（X1発振回路=未使用、XT1発振回路=使用、XT1低消費発振）
	MSTOP     = 1;		// X1発信回路の発振停止
	MCM0      = 0;		// メイン・システム・クロックに高速オンチップ・オシレータ・クロックを選択
	XTSTOP    = 0;		// XT1発振回路の発振開始
	for (i = 0; i < WAIT_XT1_CLOCK; i++) {
		NOP();			// XT1発振回路の安定化待ち
	}
	OSMC.osmc = 0x00;	// 周辺機器へのサブシステム・クロックの供給許可、RTC/ITのクロックにXT1発振回路を選択
#elif (RTC_CLK_SOURCE == CLK_SOURCE_FIL)
	CMC.cmc   = 0x10;	// クロック動作モード制御レジスタの設定（X1発振回路=未使用、XT1発振回路=使用、XT1低消費発振）
	MSTOP     = 1;		// X1発信回路の発振停止
	XTSTOP    = 1;		// XT1発振回路の発振停止
	MCM0      = 0;		// メイン・システム・クロックに高速オンチップ・オシレータ・クロックを選択
	OSMC.osmc = 0x10;	// 周辺機器へのサブシステム・クロックの供給許可、RTC/ITのクロックに低速オンチップ・オシレータを選択
#endif
	HIOSTOP = 0;	// 高速オンチップ・オシレータの動作開始
	CSS     = 0;	// CPU/周辺ハードウェア・クロックににメイン・システム・クロックを選択
	while (CLS != 0);
}


/**
 * CPU/周辺ハードウェアの動作クロックにメイン・システム・クロック
 * （高速オンチップ・オシレータ）を選択
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void setup_main_system_clock()
{
	if (HIOSTOP == 1) {
		HIOSTOP = 0;	// 高速オンチップ・オシレータの動作開始
		CSS     = 0;	// CPU/周辺ハードウェア・クロックににメイン・システム・クロックを選択
		while (CLS != 0);
#if (RTC_CLK_SOURCE == CLK_SOURCE_FIL)
		XTSTOP  = 1;	// XT1発信回路の発振停止
		CMC.cmc = 0x00;	// XT1発信回路の動作停止
#endif
	}
}


/**
 * CPU/周辺ハードウェアの動作クロックにサブシステム・クロック（XT1発信回路）を選択
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
 void setup_sub_system_clock()
{
	if (CLS == 0) {
#if (RTC_CLK_SOURCE == CLK_SOURCE_FIL)
		unsigned long i;
		CMC.cmc  = 0x10;// XT1発振回路の発振開始
		XTSTOP  = 0;	// XT1発信回路の発振開始
		for (i = 0; i < WAIT_XT1_CLOCK; i++) {
			NOP();		// 1秒以上のwait
		}
#endif
		CSS     = 1;	// CPU/周辺ハードウェア・クロックににサブシステム・クロックを選択
		while (CLS != 1);
		HIOSTOP = 1;	// 高速オンチップ・オシレータの動作停止
	}
}


/**
 * インターバル･タイマの初期化
 *
 * @return 返り値なし
 *
 * @attention なし
 ***************************************************************************/
void init_interval_timer()
{
	RTCEN      = 1;			// RTCのクロック供給開始
	ITMC.itmc  = 0x0000;	// インターバル・タイマ・コントロール・レジスタの初期化
	ITMK       = 1;			// INTIT割り込みの禁止
	ITPR1      = 1;			// INTIT割り込みの優先度設定
	ITPR0      = 1;
	ITMC.itmc  = INTERVAL;	// 周期の設定

	TAU0EN    = 1;				// タイマ・アレイ・ユニットにクロック供給開始
	TPS0.tps0 = TIMER_CLOCK;	// タイマ・クロック周波数を設定
	TT0.tt0     |= 0x0020;			// マイクロ秒タイマの動作停止
	TMMK05     = 1;			// INTTM05割り込みマスク解除
	TMIF05     = 0;			// INTTM05割り込み要因のクリア
	TMPR005     = 0;			// INTTM05割り込み要因のクリア
	TMPR105     = 0;			// INTTM05割り込み要因のクリア
	TMR05.tmr05  = INTERVAL_MICRO_MODE;	// マイクロ秒タイマの動作モード設定
	TDR05.tdr05  = INTERVAL_MICRO_TDR;	// マイクロ秒タイマの周期設定
    TOM0.tom0 &= ~0x0020;
    TOL0.tol0 &= ~0x0020;
	TO0.to0     &= ~0x0020;			// マイクロ秒タイマの出力設定
	TOE0.toe0   &= ~0x0020;			// マイクロ秒タイマの出力許可設定

	ITIF       = 0;			// INTIT割り込み要因のクリア
	ITMK       = 0;			// INTIT割り込みの許可
	ITMC.itmc |= 0x8000;	// インターバル・タイマの開始
	TMIF05     = 0;			// INTTM05割り込み要因のクリア
	TMMK05     = 0;			// INTTM05割り込みマスク解除
	TS0.ts0   |= 0x0020;	// マイクロ秒タイマの開始
	g_timer05_overflow_count = 0; //
	g_u32timer_millis = 0;	// カウンタの初期化

}

/**
 * インターバル･タイマの停止
 *
 * @return 返り値なし
 *
 * @attention なし
 ***************************************************************************/
void stop_interval_timer()
{
	ITMC.itmc &= ~0x8000;	// インターバル・タイマの停止
	ITMK       = 1;			// INTIT割り込みの禁止
	ITIF       = 0;			// INTIT割り込み要因のクリア
}



/**
 * インターバル･タイマ割り込みハンドラ
 *
 * @return 返り値なし
 *
 * @attention なし
 ***************************************************************************/
INTERRUPT void interval_timer()
{
	// 1 msecカウンタのインクリメント
	g_u32timer_millis++;

	if (g_u32delay_timer  != 0) {
		// delay() タイマのデクリメント
		g_u32delay_timer--;
	}

	if (g_fITInterruptFunc != NULL) {
		(*g_fITInterruptFunc)(g_u32timer_millis);
	}
}


/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
