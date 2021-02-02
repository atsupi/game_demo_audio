/******************************************************
 *    Filename:     azplf_audio.h 
 *     Purpose:     Audio generation for ZYBO (azplf) 
 *  Created on: 	2021/01/31
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

#ifndef _AZPLF_AUDIO_H
#define _AZPLF_AUDIO_H

#include "azplf_bsp.h"
#include "azplf_hal.h"
#include "wav_util.h"

#define REG_I2S_OUT(offset)		(*(volatile unsigned int *)(pReg_i2s_drv + (offset)))

extern void azplf_audio_init(void);
extern void azplf_audio_deinit(void);
extern void azplf_audio_set_volume(int value);
extern int azplf_audio_get_volume(void);
extern void azplf_audio_initSSM2603(void);
extern void PlayWavFile(char *fn);

extern u8 i2c_read1byte(u8 address);
extern void i2c_write1byte(u8 address, u8 data);
extern int i2c_init(void);
extern void i2c_deinit(void);

extern void i2sout_init(void);
extern void i2sout_deinit(void);
extern void i2sout_senddata(u32 address, u32 data);
extern u32 i2sout_getstatus(u32 address);


#endif //_AZPLF_AUDIO_H
