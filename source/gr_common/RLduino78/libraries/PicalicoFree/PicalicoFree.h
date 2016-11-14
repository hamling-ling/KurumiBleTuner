//
//  PicalicoFree.h
//  OITcodec
//
//  Created by Nobuo Iizuka on 2014/09/22.
//  Copyright (c) 2014年 CASIO R&D. All rights reserved.
//

#ifndef OITcodec_PicalicoFree_h
#define OITcodec_PicalicoFree_h

    bool PCFsendNum(int num, int *array);   //数字
  
    bool PCFsendChar(char chr, int *array);  //文字
  
    bool PCFsendTime(int h, int m, int *array);  //時間
  
    bool PCFsendPercent(int num, int *array);  //％表示
  
    bool PCFsendTemperature(int num, int *array); //温度表示
  
    bool PCFsendFixedPhrases(int msgNum, int *array);  //固定メッセージ

#endif // OITcodec_PicalicoFree_h