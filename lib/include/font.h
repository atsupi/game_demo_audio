/******************************************************
 *    Filename:     font.h 
 *     Purpose:     text draw with font
 *  Created on: 	2021/01/26
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

#ifndef _FONT_H
#define _FONT_H

#include "azplf_bsp.h"
#include "azplf_hal.h"

#define FONT_WIDTH			16
#define FONT_HEIGHT			16

extern void configFontResouce(GfxaccelInstance *pGfxaccel, u32 resAddr, pos *basePos);
extern void drawText(u32 dest_fb, u16 x, u16 y, u8 *str);

#endif //_FONT_H
