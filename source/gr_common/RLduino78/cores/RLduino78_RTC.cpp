/***************************************************************************
 *
 * PURPOSE
 *   RTC(Real Time Clock) function module file.
 *
 * TARGET DEVICE
 *   RL78/G13
 *
 * AUTHOR
 *   Renesas Solutions Corp.
 *
 * $Date:: 2013-02-28 14:27:07 +0900#$
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
 * @file  RLduino78_RTC.cpp
 * @brief RL78マイコン内蔵の時計機能（RTC：リアル・タイム・クロック）を使うためのライブラリです。
 * 
 * RTCクラスはこのライブラリをC++でカプセル化して使いやすくしたものです。
 */

/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include "RLduino78_mcu_depend.h"
#include "RLduino78_RTC.h"


/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/
#define RTC_WAIT_10USEC	320	//!< 10 us待ちカウント値


/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/


/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/
static void rtc_pause();
static void rtc_resume();
static inline uint8_t HEX2BCD(int s16HEX);
static inline int BCD2HEX(uint8_t u8BCD);


/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/



/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/
static fInterruptFunc_t g_fRTCInterruptFunc = NULL;	
static fInterruptFunc_t g_fRTCConstantPeriodInterruptFunc = NULL;
static RTC_CONSTANT_PERIOD_TIME g_RTCConstantPeriodTime = RTC_CONSTANT_PERIOD_TIME_NONE;


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/
/** ************************************************************************
 * @addtogroup group101
 * 
 * @{
 ***************************************************************************/
/** ************************************************************************
 * @defgroup group12 時計機能
 * 
 * @{
 ***************************************************************************/
/**
 * RTCのハードウェアを初期化します。
 *
 * @retval 0：RTCの起動に失敗しました。
 * @retval 1：RTCの起動に成功しました。
 * @retval 2：RTCは既に起動しています。
 *
 * @attention なし
 ***************************************************************************/
int rtc_init()
{
	int error = 0;

	if (RTCE == 0) {
#ifdef WORKAROUND_READ_MODIFY_WRITE
		CBI(SFR_RTCC0, SFR_BIT_RTCE);	// RCTのカウンタ動作停止
		SBI(SFR_MK1H, 1);				// INTRTC割り込み禁止
		CBI(SFR_IF1H, 1);				// INTRTC割り込みフラグのクリア
		SBI(SFR_PR11H, 1);				// INTRTC割り込み優先順位の設定
		SBI(SFR_PR01H, 1);
		RTCC0.rtcc0 = 0x08;				// RTCコントロールレジスタの初期化（1Hzの出力禁止、24時間制、定期周期割り込みを使用しない）
#else /* WORKAROUND_READ_MODIFY_WRITE */
		RTCE        = 0;	// RCTのカウンタ動作停止
		RTCMK       = 1;	// INTRTC割り込み禁止
		RTCIF       = 0;	// INTRTC割り込みフラグのクリア
		RTCPR1      = 1;	// INTRTC割り込み優先順位の設定
		RTCPR0      = 1;
		RTCC0.rtcc0 = 0x08;	// RTCコントロールレジスタの初期化（1Hzの出力禁止、24時間制、定期周期割り込みを使用しない）
#endif
		// 時間情報の初期化
		SEC.sec     = 0;
		MIN.min     = 0;
		HOUR.hour   = 0;
		WEEK.week   = 1;
		DAY.day     = 1;
		MONTH.month = 1;
		YEAR.year   = 1;
		// アラームの初期化
#ifdef WORKAROUND_READ_MODIFY_WRITE
		CBI(SFR_RTCC1, 7);	// アラーム動作の停止
		SBI(SFR_RTCC1, 6);	// アラーム割り込みを発生させる

		CBI(SFR_IF1H, 1);				// INTRTC割り込みフラグのクリア
		SBI(SFR_MK1H, 1);				// INTRTC割り込み禁止
		SBI(SFR_RTCC0, SFR_BIT_RTCE);	// RCTのカウンタ動作開始
#else /* WORKAROUND_READ_MODIFY_WRITE */
		WALE        = 0;	// アラーム動作の停止
		WALIE       = 1;	// アラーム割り込みを発生させる

		RTCIF  = 0;	// INTRTC割り込みフラグのクリア
		RTCMK  = 1;	// INTRTC割り込み禁止
		RTCE   = 1;	// RCTのカウンタ動作開始
#endif
		// コールバックハンドラの初期化
		g_fRTCInterruptFunc = NULL;
		g_fRTCConstantPeriodInterruptFunc = NULL;
		g_RTCConstantPeriodTime = RTC_CONSTANT_PERIOD_TIME_NONE;

		error = 1;
	}
	else {
		error = 2;
	}

	return error;
}


/**
 * RTCを停止します。
 *
 * @retval 0：RTCの停止に失敗しました。
 * @retval 1：RTCの停止に成功しました。
 *
 * @attention なし
 ***************************************************************************/
int rtc_deinit()
{
#ifdef WORKAROUND_READ_MODIFY_WRITE
	SBI(SFR_MK1H, 1);				// INTRTC割り込み禁止
	CBI(SFR_RTCC0, SFR_BIT_RTCE);	// RCTのカウンタ動作停止
#else /* WORKAROUND_READ_MODIFY_WRITE */
	RTCMK  = 1;	// INTRTC割り込み禁止
	RTCE   = 0;	// RCTのカンタ動作停止
#endif
	// コールバックハンドラの初期化
	g_fRTCInterruptFunc = NULL;
	g_fRTCConstantPeriodInterruptFunc = NULL;
	g_RTCConstantPeriodTime = RTC_CONSTANT_PERIOD_TIME_NONE;

	return 1;
}


/**
 * RTCの時間を設定します。
 *
 * @param[in] time 設定する時刻が格納された構造体を指定します。
 *
 * @retval 0：時刻の設定に失敗しました。
 * @retval 1：時刻の設定に成功しました。
 *
 * @attention 時刻の値はBCDではありません。また、rtc_init()関数を自動的に呼び出し、RTCを初期化します。
 ***************************************************************************/
int rtc_set_time(const RTC_TIMETYPE* time)
{
	int error = 1;

	rtc_init();
	rtc_pause();
	if (RWST == 1)	{
		YEAR.year   = HEX2BCD(time->year % 100);
		MONTH.month = HEX2BCD(time->mon);
		DAY.day     = HEX2BCD(time->day);
		HOUR.hour   = HEX2BCD(time->hour);
		MIN.min     = HEX2BCD(time->min);
		SEC.sec     = HEX2BCD(time->second);
		WEEK.week   = HEX2BCD(time->weekday);
		rtc_resume();
		if (RWST == 1U) {
			error = 0;
		}
	} else {
		error = 0;
	}

	return error;
}


/**
 * RTCの時間を取得します。
 *
 * @param[out] time 時刻を格納する構造体を指定します。
 *
 * @retval 0：時刻の取得に失敗しました。
 * @retval 1：時刻の取得に成功しました。
 *
 * @attention 時刻の値はBCDではありません。
 ***************************************************************************/
int rtc_get_time(RTC_TIMETYPE* time)
{
	int error = 1;

	rtc_pause();
	if (RWST == 1) {
		time->year    = BCD2HEX(YEAR.year);
		time->mon     = BCD2HEX(MONTH.month);
		time->day     = BCD2HEX(DAY.day);
		time->hour    = BCD2HEX(HOUR.hour);
		time->min     = BCD2HEX(MIN.min);
		time->second  = BCD2HEX(SEC.sec);
		time->weekday = BCD2HEX(WEEK.week);
		rtc_resume();
		if (RWST == 1U) {
			error = 0;
		}
	} else {
		error = 0;
	}

	return error;
}


/**
 * アラーム時に実行するハンドラを登録します。
 *
 * @param[in] fFunction アラーム時に実行するハンドラを指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_attach_alarm_handler(void (*fFunction)(void))
{
	g_fRTCInterruptFunc = fFunction;
}


/**
 * アラーム時間を設定します。
 *
 * @param[in] hour      時を指定します。
 * @param[in] min       分を指定します。
 * @param[in] week_flag 曜日を指定します。複数の曜日を指定する場合は論理和で接続します。
 *
 * @retval 0：アラームの設定に失敗しました。
 * @retval 1：アラームの設定に成功しました。
 *
 * @attention 時刻の値はBCDではありません。
 ***************************************************************************/
int rtc_set_alarm_time(int hour, int min, int week_flag)
{
#ifdef WORKAROUND_READ_MODIFY_WRITE
	SBI(SFR_MK1H, 1);		// INTRTC割り込み禁止
	RTCC1.rtcc1 &= ~0x80;	// アラーム動作を無効に設定
	RTCC1.rtcc1 |=  0x40;	// アラーム割り込み許可
	ALARMWM.alarmwm = HEX2BCD(min);
	ALARMWH.alarmwh = HEX2BCD(hour);
	ALARMWW.alarmww = week_flag;
	RTCC1.rtcc1 |=  0x80;	// アラーム動作を有効に設定
	RTCC1.rtcc1 &= ~0x10;	// アラームの一致を検出
	CBI(SFR_IF1H, 1);		// INTRTC割り込みフラグのクリア
	CBI(SFR_MK1H, 1);		// INTRTC割り込み許可
#else /* WORKAROUND_READ_MODIFY_WRITE */
	RTCMK        = 1;		// INTRTC割り込み禁止
	RTCC1.rtcc1 &= ~0x80;	// アラーム動作を無効に設定
	RTCC1.rtcc1 |=  0x40;	// アラーム割り込み許可
	ALARMWM.alarmwm = HEX2BCD(min);
	ALARMWH.alarmwh = HEX2BCD(hour);
	ALARMWW.alarmww = week_flag;
	RTCC1.rtcc1 |=  0x80;	// アラーム動作を有効に設定
	RTCC1.rtcc1 &= ~0x10;	// アラームの一致を検出
	RTCIF        = 0;		// INTRTC割り込みフラグをクリア
	RTCMK        = 0;		// INTRTC割り込み許可
#endif
	return 1;
}

/**
 * アラームをONにします。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_alarm_on()
{
#ifdef WORKAROUND_READ_MODIFY_WRITE
	SBI(SFR_MK1H, 1);		// INTRTC割り込み禁止
	RTCC1.rtcc1 |=  0x80;	// アラーム動作を有効に設定
	RTCC1.rtcc1 &= ~0x10;	// アラームの一致を検出
	CBI(SFR_IF1H, 1);		// INTRTC割り込みフラグのクリア
	CBI(SFR_MK1H, 1);		// INTRTC割り込み許可
#else /* WORKAROUND_READ_MODIFY_WRITE */
	RTCMK        = 1;		// INTRTC割り込み禁止
	RTCC1.rtcc1 |=  0x80;	// アラーム動作を有効に設定
	RTCC1.rtcc1 &= ~0x10;	// アラームの一致を検出
	RTCIF        = 0;		// INTRTC割り込みフラグをクリア
	RTCMK        = 0;		// INTRTC割り込み許可
#endif
}

/**
 * アラームをOFFにします。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_alarm_off()
{
#ifdef WORKAROUND_READ_MODIFY_WRITE
	SBI(SFR_MK1H, 1);		// INTRTC割り込み禁止
	RTCC1.rtcc1 &= ~0x80;	// アラーム動作を無効に設定
	RTCC1.rtcc1 &= ~0x10;	// アラームの一致を検出
	CBI(SFR_IF1H, 1);		// INTRTC割り込みフラグのクリア
#else /* WORKAROUND_READ_MODIFY_WRITE */
	RTCMK        = 1;		// INTRTC割り込み禁止
	RTCC1.rtcc1 &= ~0x80;	// アラーム動作を無効に設定
	RTCC1.rtcc1 &= ~0x10;	// アラームの一致を検出
	RTCIF        = 0;		// INTRTC割り込みフラグをクリア
#endif
}

/**
 * 定周期に実行するハンドラを登録します。
 *
 * @param[in] fFunction 定周期に実行するハンドラを指定します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_attach_constant_period_interrupt_handler(void (*fFunction)(void))
{
	g_fRTCConstantPeriodInterruptFunc = fFunction;
}


/**
 * 定周期期間を設定します。
 *
 * @param[in] ct 定周期期間を指定します。
 *               RTCConstantPeriodTimeNone:       定周期割り込み機能を使用しない
 *               RTCConstantPeriodTimeHalfSecond: 0.5秒に1度（秒カウントアップに同期）
 *               RTCConstantPeriodTime1Second:    1秒に1度（秒カウントアップと同時）
 *               RTCConstantPeriodTime1Minute:    1分に1度（毎分00秒）
 *               RTCConstantPeriodTime1Hour:      1時間に1度（毎時00分00秒）
 *               RTCConstantPeriodTime1Day:       1日に1度（毎日00時00分00秒）
 *               RTCConstantPeriodTime1Month:     1月に1度（毎月1日午前00時00分00秒）
 *
 * @retval 0：定周期期間の設定に失敗しました。
 * @retval 1：定周期期間の設定に成功しました。
 *
 * @attention なし
 ***************************************************************************/
static void _rtc_set_constant_period_interrupt_time(RTC_CONSTANT_PERIOD_TIME ct)
{
#ifdef WORKAROUND_READ_MODIFY_WRITE
	SBI(SFR_MK1H, 1);		// INTRTC割り込み禁止
#else /* WORKAROUND_READ_MODIFY_WRITE */
	RTCMK        = 1;		// INTRTC割り込み禁止
#endif
	RTCC0.rtcc0 = (RTCC0.rtcc0 & ~0x07) | ct;
#ifdef WORKAROUND_READ_MODIFY_WRITE
	CBI(SFR_IF1H, 1);		// INTRTC割り込みフラグのクリア
	CBI(SFR_MK1H, 1);		// INTRTC割り込み許可
#else /* WORKAROUND_READ_MODIFY_WRITE */
	RTCIF        = 0;		// INTRTC割り込みフラグをクリア
	RTCMK        = 0;		// INTRTC割り込み許可
#endif
}

int rtc_set_constant_period_interrupt_time(RTC_CONSTANT_PERIOD_TIME ct)
{
	g_RTCConstantPeriodTime = ct;
	_rtc_set_constant_period_interrupt_time(ct);
	return 1;
}

/**
 * 定周期割り込みをONにします。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_constant_period_interrupt_on()
{
	_rtc_set_constant_period_interrupt_time(g_RTCConstantPeriodTime);
}

/**
 * 定周期割り込みをOFFにします。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void rtc_constant_period_interrupt_off()
{
	_rtc_set_constant_period_interrupt_time(RTC_CONSTANT_PERIOD_TIME_NONE);
}

/// @cond
/**
 * アラームの割り込みハンドラです。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
 extern "C"
INTERRUPT void rtc_interrupt()
{
	if (WAFG == 1) {
		RTCC1.rtcc1 &= ~0x10;
		if (g_fRTCInterruptFunc != NULL) {
			(*g_fRTCInterruptFunc)();
		}
	}
	if (RIFG == 1) {
		RTCC1.rtcc1 &= ~0x08;
		if (g_fRTCConstantPeriodInterruptFunc != NULL) {
			(*g_fRTCConstantPeriodInterruptFunc)();
		}
	}
}
/// @endcond
/** @} */
/** @} group101 */


/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/
static void rtc_pause()
{
#ifdef WORKAROUND_READ_MODIFY_WRITE
	SBI(SFR_MK1H, 1);		// INTRTC割り込み禁止
	RTCC1.rtcc1 |=  0x01;
	CBI(SFR_MK1H, 1);		// INTRTC割り込み許可
	 for(int i = 0; i < RTC_WAIT_10USEC; i++ ) {
		NOP();
	}
#else /* WORKAROUND_READ_MODIFY_WRITE */
	RTCMK        = 1;		// INTRTC割り込み禁止
	RTCC1.rtcc1 |=  0x01;
	RTCMK        = 0;		// INTRTC割り込み許可
	 for(int i = 0; i < RTC_WAIT_10USEC; i++ ) {
		NOP();
	}
#endif
}

static void rtc_resume()
{
#ifdef WORKAROUND_READ_MODIFY_WRITE
	SBI(SFR_MK1H, 1);		// INTRTC割り込み禁止
	RTCC1.rtcc1 &= ~0x01;
	CBI(SFR_MK1H, 1);		// INTRTC割り込み許可
	for(int i = 0; i < RTC_WAIT_10USEC; i++ ) {
		NOP();
	}
#else /* WORKAROUND_READ_MODIFY_WRITE */
	RTCMK        = 1;		// INTRTC割り込み禁止
	RTCC1.rtcc1 &= ~0x01;
	RTCMK        = 0;		// INTRTC割り込み許可
	for(int i = 0; i < RTC_WAIT_10USEC; i++ ) {
		NOP();
	}
#endif
}

static inline uint8_t HEX2BCD(int s16HEX)
{
	return ((s16HEX / 10) << 4) | (s16HEX % 10);
}

static inline int BCD2HEX(uint8_t u8BCD)
{
	return ((u8BCD >> 4) * 10) + (u8BCD & 0x0F);
}


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
