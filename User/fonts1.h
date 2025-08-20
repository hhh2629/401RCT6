#ifndef __FONTS1_H_
#define __FONTS1_H_

#include "stdint.h"
#define OLED_CHN_CHAR_WIDTH	3   // UTF-8编码格式给3，GB2312编码格式给2
typedef struct {
    const uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;


typedef struct
{
    char Index[OLED_CHN_CHAR_WIDTH + 1];	//汉字索引
    uint8_t Data[32];						//字模数据
} ChineseCell_t;


//Font lib.
extern FontDef Font_7x10;

//16-bit(RGB565) Image lib.
/*******************************************
 *             CAUTION:
 *   If the MCU onchip flash cannot
 *  store such huge image data,please
 *           do not use it.
 * These pics are for test purpose only.
 *******************************************/

/* 128x128 pixel RGB565 image */

#endif
