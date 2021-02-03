/******************************************************
 *    Filename:     game.c 
 *     Purpose:     game core
 *  Created on: 	2021/01/31
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

//#define DEBUG

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "azplf_hal.h"
#include "azplf_util.h"

static int game_quit = 0;

static int game_initialized = 0;

static u32 system_time = 0; // 1/60 sec unit
struct timeval start_time;
static pthread_mutex_t mutex;

static int scene = 0;
static int scene_change = 0;	// flag to step to next_scene
static int next_scene = 0;

static fb_render_handler fb_handler = NULL;

static u32 ProcessTime(void)
{
	static u32 old_time = 0;
	struct timeval curr_time;
	float duration; // us unit

	game_lock_mutex();
	gettimeofday(&curr_time, NULL);
#ifdef DEBUG
	printf("current time=%d:%d\n", (int)curr_time.tv_sec, (int)curr_time.tv_usec);
#endif
	duration = (curr_time.tv_sec - start_time.tv_sec) * 1000000.0 + (curr_time.tv_usec - start_time.tv_usec);
	duration -= system_time * 16666.7;

	system_time += duration / 16666.7;
	if (system_time == 0xffffffff) {
		printf("system time is negative\n");
		// system time is modified after booting
		// reset start_time with current time
		gettimeofday(&start_time, NULL);
		system_time = 0;
		old_time = 0;
	}
	game_unlock_mutex();

	if (system_time - old_time >= 59) {
#ifdef DEBUG
		printf("%d\n", system_time / 60);
#endif
		old_time = system_time;
	}
	if (duration < 60 * 1000 && duration >= 16) {
		return (system_time);
	}

	return 0;
}

static int SceneChangeCheck(void)
{
	int update = 0;

	game_lock_mutex();
	if (scene_change) {
		scene = next_scene;
		scene_change = 0;
		update = 1;
	}
	game_unlock_mutex();

	if (update) printf("Scene changed to %d\n", scene);
	return (update);
}

void game_init(void)
{
	// create mutex for thread handling
	pthread_mutex_init(&mutex, NULL);

	game_initialized = 1;
	game_quit = 0;
}

void game_terminate(void)
{
	game_quit = 1;
}

void game_deinit(void)
{
	pthread_mutex_destroy(&mutex);
	game_initialized = 0;
}

void game_lock_mutex(void)
{
	// lock mutex for thread handling
	pthread_mutex_lock(&mutex);
}

void game_unlock_mutex(void)
{
	// unlock mutex for thread handling
	pthread_mutex_unlock(&mutex);
}

u32 game_get_systemtime(void)
{
	u32 result;
	game_lock_mutex();
	result = system_time;
	game_unlock_mutex();

	return (result);
}

int game_get_scene(void)
{
	return (scene);
}

void game_set_next_scene(int nextScene)
{
	game_lock_mutex();
	scene_change = 1;
	next_scene   = nextScene;
	game_unlock_mutex();
}

void game_set_fb_renderer(fb_render_handler handler)
{
	fb_handler = handler;
}

void *game_work_thread(void *arg)
{
	gettimeofday(&start_time, NULL);

	printf("Loop starts.\r\n");
	while (!game_quit) {
		if (ProcessTime()) {
			SceneChangeCheck();
			if (fb_handler)
				fb_handler(game_get_scene());
		}
		usleep(5000); // 5ms wait
	}

	return 0;
}
