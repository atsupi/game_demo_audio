/******************************************************
 *    Filename:     sprite.c 
 *     Purpose:     sprite draw
 *  Created on: 	2021/01/24
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

#include <stdio.h>
#include "azplf_hal.h"
#include "azplf_util.h"
#include "sprite.h"

static GfxaccelInstance *l_ptGfxaccel = NULL;
static u32 l_resAddr = 0;
static pos l_bgResBasePos   = {   0,   0 };
static pos l_fontResBasePos = { 256,   0 };
static pos l_sprResBasePos  = { 512,   0 };

// pGfxaccel: logical address to graphics accelerator driver instance
// resAddr: physical address to resource frame buffer
void configSpriteResouce(GfxaccelInstance *pGfxaccel, u32 resAddr, pos *basePos)
{
	l_ptGfxaccel      = pGfxaccel;
	l_resAddr         = resAddr;
	l_sprResBasePos.x = basePos->x;
	l_sprResBasePos.y = basePos->y;
}

// inst: pointer to Sprite structure instance
// system_time: 1/60sec unit
void drawSpriteWithAnimation(Sprite *inst, u32 wrBufAddr, u32 system_time)
{
	u16 pos_x, pos_y, size_x, size_y, res_x, res_y;
	u8 i;

	if (!inst) return;

	if (!l_ptGfxaccel || !l_resAddr) {
		printf("Error: call configSpriteResource() before invoke drawSprite()\n");
		return;
	}

	if (system_time < inst->anim.prev_time) {
		inst->anim.prev_time = system_time;
		return;
	}

	if (inst->anim.num_anim > 1 && 
		inst->anim.prev_time + inst->anim.duration <= system_time) // expired
	{
		// increment curr_num;
		inst->anim.curr_num = (inst->anim.curr_num + 1) % inst->anim.num_anim;
		inst->anim.prev_time = system_time;
	}
	i = inst->anim.curr_num;

	if (l_ptGfxaccel)
	{
		if (inst->anim.msk_x[i] < DISP_WIDTH && inst->anim.msk_y[i] < DISP_HEIGHT) {
			res_x  = inst->anim.msk_x[i] * SPRITE_WIDTH  + l_sprResBasePos.x;
			res_y  = inst->anim.msk_y[i] * SPRITE_HEIGHT + l_sprResBasePos.y;
			size_x = inst->dx;
			size_y = inst->dy;
			pos_x  = inst->x;
			pos_y  = inst->y;
			gfxaccel_bitblt(l_ptGfxaccel, 
				l_resAddr, res_x, res_y, size_x, size_y, 
				wrBufAddr, pos_x, pos_y, 
				GFXACCEL_BB_AND);
		}
		res_x  = inst->anim.src_x[i] * SPRITE_WIDTH  + l_sprResBasePos.x;
		res_y  = inst->anim.src_y[i] * SPRITE_HEIGHT + l_sprResBasePos.y;
		size_x = inst->dx;
		size_y = inst->dy;
		pos_x  = inst->x;
		pos_y  = inst->y;
		gfxaccel_bitblt(l_ptGfxaccel, 
			l_resAddr, res_x, res_y, size_x, size_y, 
			wrBufAddr, pos_x, pos_y, 
			GFXACCEL_BB_OR);
	}
}

void drawSprite(Sprite *inst, u32 wrBufAddr, u32 system_time)
{
	u16 pos_x, pos_y, size_x, size_y, res_x, res_y;

	if (!inst) return;

	if (!l_ptGfxaccel || !l_resAddr) {
		printf("Error: call configSpriteResource() before invoke drawSprite()\n");
		return;
	}

	if (inst->anim.used) {
		drawSpriteWithAnimation(inst, wrBufAddr, system_time);
		return;
	}

	if (l_ptGfxaccel) {
		res_x  = inst->src_x * SPRITE_WIDTH  + l_sprResBasePos.x;
		res_y  = inst->src_y * SPRITE_HEIGHT + l_sprResBasePos.y;
		size_x = inst->dx;
		size_y = inst->dy;
		pos_x  = inst->x;
		pos_y  = inst->y;
		gfxaccel_bitblt(l_ptGfxaccel, 
			l_resAddr, res_x, res_y, size_x, size_y, 
			wrBufAddr, pos_x, pos_y, 
			GFXACCEL_BB_NONE);
	}
}
