/******************************************************
 *    Filename:     azplf_hal.h 
 *     Purpose:     azplf HAL header 
 *  Target Plf:     azplf (ZYBO learning platform) 
 *  Created on: 	2021/01/20 
 *      Author: 	atsupi.com 
 *     Version:		1.00 
 ******************************************************/

#ifndef AZPLF_HAL_H_
#define AZPLF_HAL_H_

#include "azplf_bsp.h"
#include "vdma.h"
#include "lq070out.h"
#include "gfxaccel.h"
#include "font.h"
#include "sprite.h"
#include "game.h"

// hardware definitions 

// display parameters 

// Definition for Frame buffer and Color

// type definitions 

typedef struct _hal_pos {
	int x;
	int y;
} hal_pos;

extern int azplf_start_game_thread(int scene, fb_render_handler fb_renderer);
extern void azplf_terminate_game_thread(void);
extern void azplf_game_deinit(void);
extern int azplf_start_graphics_thread(void);


#endif //AZPLF_HAL_H_
