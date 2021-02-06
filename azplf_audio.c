/******************************************************
 *    Filename:     azplf_audio.c 
 *     Purpose:     Audio generation for ZYBO (azplf)
 *  Created on: 	2021/01/31
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

//#define DEBUG

#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/i2c-dev.h>
#include "azplf_hal.h"
#include "azplf_util.h"
#include "azplf_audio.h"

static u32 pReg_i2s_drv = 0;
static int pReg_iic = 0;

#define INIT_COUNT		 11*2

static u8 SSM2603_Init[INIT_COUNT] = {
 30 | 0x00, 0x00 ,    /* R15: RESET */
  0 | 0x01, 0x17 ,    /* R0: L_in vol : LR simul-update, unmute, 0dB */
  2 | 0x01, 0x17 ,    /* R1: R_in vol : LR simul-update, unmute, 0dB */
  4 | 0x01, 0xF9 ,    /* R2: L_HP vol : LR simul-update, zero-cross, 0dB */
  6 | 0x01, 0xF9 ,    /* R3: R_HP vol : LR simul-update, zero-cross, 0dB */
  8 | 0x00, 0x12 ,    /* R4: Analog Audio Path : No Sidetone, No bypass, DAC for Out, Line out for ADC, Mic Mute */
 10 | 0x00, 0x00 ,    /* R5: Digital Path: DAC unmute, De-emphasis 48k, ADC HPF enable */
 12 | 0x00, 0x02 ,    /* R6: Power Down : Microphone is down*/
 14 | 0x00, 0x0E ,    /* R7: Digital Audio Format : Slave, 16bit, I2S */
 16 | 0x00, 0x01 ,    /* R8: Sampling Rate, 48kHz, USB mode*/
 18 | 0x00, 0x01      /* R9: Activation : Active. */
};

u32 page_size;

// set volume (0 - 15)
static int volume = 4;

void azplf_audio_set_volume(int value)
{
	if (value < 0) 
		volume = 0;
	else if (value > 15)
		volume = 15;
	else
		volume = value;
}

int azplf_audio_get_volume(void)
{
	return (volume);
}

void azplf_audio_free_wav(WavHeader *wav)
{
	free_wavheader(wav);
}

int azplf_audio_load_wav(char *fn, WavHeader *wav)
{
	int result;
	u32 ldata;
	u32 stat;
	int i;
	float sdata;

	result = wav_readfile(wav, fn /*"test.wav"*/);
	if (wav->bit != WAV_BITS)
	{
		printf("Error: bit width is not 16.\r\n");
		azplf_audio_free_wav(wav);
		return 0;
	}

	return 1;
}

void PlayWavFile(char *fn)
{
	int result;
	WavHeader wavheader;
	u32 ldata;
	u32 stat;
	int i;
	float sdata;

	result = wav_readfile(&wavheader, fn /*"test.wav"*/);
	if (wavheader.bit != WAV_BITS)
	{
		printf("Error: bit width is not 16.\r\n");
		return 0;
	}
	for (i = 0; i < wavheader.Nbyte/2; i++)
	{
		while ((stat = i2sout_getstatus(0x0C) & 0xC));

		sdata = wavheader.data[i];
		sdata = sdata * (volume + 1) / 16.0; // attenuator
		// bit debug[0] inverts the MSB of I2S data
		ldata = (u32)sdata;
		ldata = ((ldata & 0xFFFF) << 16) | (ldata & 0xFFFF);
#if defined(_DEBUG)
		if (i < 64)
		{
			printf("out_aud: %08x\r\n", (u32)ldata);
		}
#endif
		i2sout_senddata(4, ldata);
	}
	free_wavheader(&wavheader);
}

void azplf_audio_init(void)
{
	i2c_init();
	i2sout_init();
}

void azplf_audio_deinit(void)
{
	i2c_deinit();
	i2sout_deinit();
}

void azplf_audio_initSSM2603(void)
{
	int i;
	for (i = 0; i < 11; i++)
	{
		i2c_write1byte(SSM2603_Init[i * 2], SSM2603_Init[i * 2 + 1]);
#if defined (DEBUG)
		u8 value;
		value = i2c_read1byte(SSM2603_Init[i * 2]);
		printf("SSM: Register value 0x%x = 0x%02x\n", SSM2603_Init[i * 2], value);
#endif
	}
}

u8 i2c_read1byte(u8 address)
{
	u8 SendBuffer[1];
	u8 RecvBuffer[1] = {0x00};

	SendBuffer[0] = address;
	write(pReg_iic, SendBuffer, 1);

	read(pReg_iic, RecvBuffer, 1);
	return (RecvBuffer[0]);
}

void i2c_write1byte(u8 address, u8 data)
{
	u8 SendBuffer[2];

	SendBuffer[0] = address;
	SendBuffer[1] = data;
	write(pReg_iic, SendBuffer, 2);
}

int i2c_init(void)
{
	pReg_iic = open("/dev/i2c-0", O_RDWR);
	ioctl(pReg_iic, I2C_SLAVE, SSM2603_IIC_ADDRESS);
	return 0;
}

void i2c_deinit(void)
{
	close(pReg_iic);
}

void i2sout_init(void)
{
	int fd;
	page_size = sysconf(_SC_PAGESIZE);
	printf("File page size=0x%08x (%dKB)\n", page_size, page_size>>10);
	fd = open("/dev/mem", O_RDWR);
	pReg_i2s_drv = (u32)mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, I2SOUT_BASEADDR);
	printf("Mapping I/O: 0x%08x to vmem: 0x%08x\n", I2SOUT_BASEADDR, pReg_i2s_drv);
	close(fd);
}

void i2sout_deinit(void)
{
	//Deinitialize the I2S Output Driver
	if (pReg_i2s_drv) munmap((void *)pReg_i2s_drv, page_size);
}

void i2sout_senddata(u32 address, u32 data)
{
	REG_I2S_OUT(address) = data;
}

u32 i2sout_getstatus(u32 address)
{
	return (REG_I2S_OUT(address));
}

