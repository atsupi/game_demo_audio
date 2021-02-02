/******************************************************
 *    Filename:     sprite.h 
 *     Purpose:     sprite draw
 *  Created on: 	2021/01/24
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

#ifndef _SPRITE_H
#define _SPRITE_H

#include "azplf_bsp.h"
#include "azplf_hal.h"

#define SPRITE_WIDTH			32
#define SPRITE_HEIGHT			32

typedef struct _Animation {
	u8 used;
	u8 num_anim;
	u8 curr_num;
	u8 reserved;
	u32 duration;
	u32 prev_time;
	u16 src_x[8];
	u16 src_y[8];
	u16 msk_x[8];
	u16 msk_y[8];
} Animation;

typedef struct _Sprite {
	u16 x;
	u16 y;
	u16 dx;
	u16 dy;
	u16 src_x;
	u16 src_y;
	Animation anim;
} Sprite;

extern void configSpriteResouce(GfxaccelInstance *pGfxaccel, u32 resAddr, pos *basePos);
extern void drawSpriteWithAnimation(Sprite *inst, u32 wrBufAddr, u32 system_time);
extern void drawSprite(Sprite *inst, u32 wrBufAddr, u32 system_time);

#endif //_SPRITE_H
