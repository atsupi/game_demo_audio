/******************************************************
 *    Filename:     azplf_hal_main.c 
 *     Purpose:     HAL main routine
 *  Created on: 	2021/01/31
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

//#define DEBUG

#include <stdio.h>
#include <pthread.h>
#include "azplf_hal.h"
#include "azplf_util.h"
#include "game.h"

static pthread_t	pt;

int azplf_start_game_thread(int scene, fb_render_handler fb_renderer)
{
	game_init();
	game_set_fb_renderer(fb_renderer);
	game_set_next_scene(scene);

	// create game core worker thread
	pthread_create(&pt, NULL, &game_work_thread, NULL);

	return PST_SUCCESS;
}

void azplf_terminate_game_thread(void)
{
	game_terminate();
}

void azplf_game_deinit(void)
{
	game_deinit();
}

int azplf_start_graphics_thread(void)
{
	//ToDo: create graphics 
	printf("Error: azplf_start_graphics_thread is not implemented yet.\n");
	return PST_FAILURE;
}
