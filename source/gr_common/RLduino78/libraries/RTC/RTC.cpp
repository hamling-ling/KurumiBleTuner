/***************************************************************************
 *
 * PURPOSE
 *   RTC(Real Time Clock) class module file.
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
 * @file  RTC.cpp
 * @brief RL78マイコン内蔵の時計機能（RTC：リアル・タイム・クロック）を使うためのクラスライブラリです。
 */

/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include "RTC.h"


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



/***************************************************************************/
/*    Local Variables                                                      */
/***************************************************************************/


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/
/** ************************************************************************
 * @addtogroup group101
 * 
 * @{
 ***************************************************************************/
/** ************************************************************************
 * @defgroup group13 時計機能（クラスライブラリ）
 * 
 * @{
 ***************************************************************************/
/**
 * RTCクラスのコンストラクタ
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
RTC::RTC()
{

}

/**
 * RTCクラスのデストラクタ
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
RTC::~RTC()
{
	end();
}

/**
 * RTCを開始します。
 *
 * @return 
 *
 * @attention なし
 ***************************************************************************/
int RTC::begin()
{
	return rtc_init();
}


/**
 * RTCを停止します。
 *
 * @return 
 *
 * @attention なし
 ***************************************************************************/
int RTC::end()
{
	return rtc_deinit();
}


/**
 * RTCの時間を設定します。
 *
 * @param[in] year 年を指定します。
 * @param[in] mon  月を指定します。
 * @param[in] day  日を指定します。
 * @param[in] hour 時を指定します。
 * @param[in] min  分を指定します。
 * @param[in] sec  秒を指定します。
 * @param[in] week 曜日を指定します。
 *
 * @return 時間の設定に成功した場合はtrueを返却します。失敗した場合はfalseを返却します。
 *
 * @attention なし
 ***************************************************************************/
bool RTC::setDateTime(int year, int mon, int day, int hour, int min, int sec, int week)
{
	bool bError = true;
	RTC_TIMETYPE time;

	time.year   = year;
	time.mon    = mon;
	time.day    = day;
	time.hour   = hour;
	time.min    = min;
	time.second = sec;
	time.weekday= week;

	if (rtc_set_time(&time) == 0) {
		bError = false;
	}

	return (bError);
}


/**
 * RTCの時間を取得します。
 *
 * @param[out] year 年の格納先を指定します。
 * @param[out] mon  月の格納先を指定します。
 * @param[out] day  日の格納先を指定します。
 * @param[out] hour 時の格納先を指定します。
 * @param[out] min  分の格納先を指定します。
 * @param[out] sec  秒の格納先を指定します。
 * @param[out] week 曜日の格納先を指定します。
 *
 * @return 時間の取得に成功した場合はtrueを返却します。失敗した場合はfalseを返却します。
 *
 * @attention なし
 ***************************************************************************/
bool RTC::getDateTime(int &year, int &mon, int &day, int &hour, int &min, int &sec, int &week)
{
	bool bError = true;
	RTC_TIMETYPE time;

	if (rtc_get_time(&time) == 0) {
		year = 0;
		mon  = 0;
		day  = 0;
		hour = 0;
		sec  = 0;
		week = 0;
		bError = false;
	}
	else {
		year = time.year;
		mon  = time.mon;
		day  = time.day;
		hour = time.hour;
		min  = time.min;
		sec  = time.second;
		week = time.weekday;
	}

	return (bError);
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
void RTC::attachAlarmHandler(void (*fFunction)(void))
{
	rtc_attach_alarm_handler(fFunction);
}


/**
 * アラーム時間を設定します。
 *
 * @param[in] hour      時を指定します。
 * @param[in] min       分を指定します。
 * @param[in] week_flag 曜日を指定します。複数の曜日を指定する場合は論理和で接続します。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void RTC::setAlarmTime(int hour, int min, int week_flag)
{
	rtc_set_alarm_time(hour, min, week_flag);
}

/**
 * アラームをONにします。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void RTC::alarmOn()
{
	rtc_alarm_on();
}

/**
 * アラームをOFFにします。
 *
 * @return なし
 *
 * @attention なし
 ***************************************************************************/
void RTC::alarmOff()
{
	rtc_alarm_off();
}
/** @} */
/** @} group101 */


/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/
