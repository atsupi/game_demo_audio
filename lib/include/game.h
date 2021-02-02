/******************************************************
 *    Filename:     game.h 
 *     Purpose:     game core
 *  Created on: 	2021/01/31
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

#ifndef _GAME_H
#define _GAME_H

#include "azplf_bsp.h"
#include "azplf_hal.h"

typedef void (*fb_render_handler)(int scene);

extern void game_init(void);
extern void game_terminate(void);
extern void game_deinit(void);
extern void game_lock_mutex(void);
extern void game_unlock_mutex(void);
extern u32 game_get_systemtime(void);
extern int game_get_scene(void);
extern void game_set_next_scene(int nextScene);
extern void game_set_fb_renderer(fb_render_handler handler);
extern void *game_work_thread(void *arg);

#endif //_GAME_H
