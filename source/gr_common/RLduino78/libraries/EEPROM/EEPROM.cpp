/***************************************************************************
 *
 * PURPOSE
 *   EEPROM library file.
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
 * @file  EEPROM.cpp
 * @brief RL78マイコン内蔵のデータ・フラッシュをEEPROMとして使用するためのライブラリです。
 *
 * @attention EEPROMライブラリを使う際は、以下の内容に注意してください。
 *            システムクロック周波数、動作電圧、書き換え回数等についてはRL78マイコンの
 *            ユーザーズマニュアルの電気的特性を参照してください。
 *            EEPROMライブラリは1024バイトの固定RAM領域（0xFAF00~0xFB2FF）と
 *            最大1024バイトのデータバッファ、46バイトのスタックを使用します。
 *            EEPROMライブラリを使用するためには以下の操作を行う必要があります。
 *            ・高速オンチップオシレータの起動
 *            ・CPU動作周波数の設定（PFDL_DATA_FLASH_FX_MHZの設定：1MHz以上）
 *            ・電圧モードをフルスピード(0x00)/ワイドボルテージモード(0x01)に設定
 *            ・データ・フラッシュ・コントロール・レジスタ（DFLCTL）のDFLENビットを1に設定
 *            これらのRAM確保や動作設定は、RLduino78のフレームワーク上で
 *            EEPROMライブラリを使用する場合には、特に考慮する必要はありませが、
 *            変更等を行う場合には考慮が必要です。
 */

/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/
#include "Arduino.h"
#include "EEPROM.h"
#include "data_flash_util.h"


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

static uint8_t	eeprom_buffer[PFDL_DATA_FLASH_BLOCK_SIZE];


/***************************************************************************/
/*    Global Routines                                                      */
/***************************************************************************/


/**
 * EEPROMのデータを1byte読み出します。
 *
 * param[in] address EEPROMを読み出すアドレス(0-0x1fff)
 *
 * @return EEPROMの読み出し結果を返します。
 *
 * @attention なし
 ***************************************************************************/
uint8_t EEPROMClass::read(int address)
{
	pfdl_status_t	result = PFDL_OK;
	uint8_t	u8ReadData = 0;
	
	result = pfdl_open();
	if ( result == PFDL_OK ){
		result = pfdl_read( (uint16_t)address, &u8ReadData, 1 );
		if( result != PFDL_OK ){
			Serial.print("DataFlash Read error :");
			Serial.println(result);
		}
	}
	else{
		Serial.print("DataFlash Open error :");
		Serial.println(result);
	}
	pfdl_close();
	
	return	u8ReadData;
}


/**
 * EEPROMへデータを1byte書き込みます。
 *
 * param[in] address EEPROMに書き込むアドレス(0-0x1fff)を指定します。
 * param[in] value   EEPROMに書き込むデータ(0-0xff)を指定します。
 *
 * @return 返り値なし。
 *
 * @attention なし
 ***************************************************************************/
void EEPROMClass::write(int address, uint8_t value)
{
	pfdl_status_t	result = PFDL_OK;
	
	result = pfdl_open();
	if ( result == PFDL_OK ){
		result = pfdl_blank_check( (uint16_t)address, 1 );
		if( result != PFDL_OK ){
			result = pfdl_read( (uint16_t)((address/PFDL_DATA_FLASH_BLOCK_SIZE)*PFDL_DATA_FLASH_BLOCK_SIZE), &eeprom_buffer[0], PFDL_DATA_FLASH_BLOCK_SIZE );
			if( result != PFDL_OK ){
				Serial.print("DataFlash Read error :");
				Serial.println(result);
			}
			else{
				eeprom_buffer[address%PFDL_DATA_FLASH_BLOCK_SIZE] = value;
				result = pfdl_erase_block( (uint16_t)(address/PFDL_DATA_FLASH_BLOCK_SIZE) );
				if( result != PFDL_OK ){
					Serial.print("DataFlash Erase error :");
					Serial.println(result);
				}
				else{
					result = pfdl_write((uint16_t)((address/PFDL_DATA_FLASH_BLOCK_SIZE)*PFDL_DATA_FLASH_BLOCK_SIZE), &eeprom_buffer[0], PFDL_DATA_FLASH_BLOCK_SIZE );
					if( result != PFDL_OK ){
						Serial.print("DataFlash Write error :");
						Serial.println(result);
					}
				}
			}
		}
		else{
			result = pfdl_write( (uint16_t)address, &value, 1 );
			if( result != PFDL_OK ){
				Serial.print("DataFlash Write error :");
				Serial.println(result);
			}
		}
	}
	else{
		Serial.print("DataFlash Open error :");
		Serial.println(result);
	}
	pfdl_close();
}

EEPROMClass EEPROM;

/***************************************************************************/
/*    Local Routines                                                       */
/***************************************************************************/


/***************************************************************************/
/* End of module                                                           */
/***************************************************************************/


