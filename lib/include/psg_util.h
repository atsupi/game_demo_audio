/******************************************************
 *    Filename:     psg_util.h 
 *     Purpose:     PSG audio utility
 *  Created on: 	2021/02/13
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.90
 ******************************************************/

#ifndef _PSG_UTIL_H
#define _PSG_UTIL_H

#include "azplf_bsp.h"
#include "azplf_hal.h"

#define CH_NUM				4
#define MAX_MML_LEN			512

extern void PlayMusicSlice(int debug_mode);
extern void AttachMMLData(int ch, char *data);
extern int LoadMMLData(char *fn);

#endif //_PSG_UTIL_H
