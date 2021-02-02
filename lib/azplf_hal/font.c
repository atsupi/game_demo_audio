/******************************************************
 *    Filename:     font.c 
 *     Purpose:     text draw with font
 *  Created on: 	2021/01/26
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

#include <stdio.h>
#include "azplf_hal.h"
#include "azplf_util.h"
#include "font.h"

static GfxaccelInstance *l_ptGfxaccel = NULL;
static u32 l_resAddr = 0;
static pos l_bgResBasePos   = {   0,   0 };
static pos l_fontBasePos    = { 256,   0 };
static pos l_sprResBasePos  = { 512,   0 };

// pGfxaccel: logical address to graphics accelerator driver instance
// resAddr: physical address to resource frame buffer
void configFontResouce(GfxaccelInstance *pGfxaccel, u32 resAddr, pos *basePos)
{
	l_ptGfxaccel      = pGfxaccel;
	l_resAddr         = resAddr;
	l_fontBasePos.x   = basePos->x;
	l_fontBasePos.y   = basePos->y;
}

void drawText(u32 dest_fb, u16 x, u16 y, u8 *str)
{
	int src_x, src_y;
	u8 ch;

	while (*str) {
		ch = *str - 32;
		if (ch > 222) ch = 14; //('.' - 32)
		src_x = (ch % 16) * FONT_WIDTH + l_fontBasePos.x;
		src_y = (ch & 0xF0) + l_fontBasePos.y;
		gfxaccel_bitblt(l_ptGfxaccel, 
			l_resAddr, src_x, src_y, 16, 16,
			dest_fb, x, y, GFXACCEL_BB_NONE);
		x += FONT_WIDTH;
		str++;
	}
}
