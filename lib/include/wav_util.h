/******************************************************
 *    Filename:     wav_util.h 
 *     Purpose:     WAV audio file utility
 *  Target Plf:     ZYBO (azplf)
 *  Created on: 	2021/02/02
 *      Author: 	atsupi.com
 *     Version:		0.80
 ******************************************************/

#ifndef WAV_UTIL_H_
#define WAV_UTIL_H_

static int WAV_BITS = 16;

typedef struct {
	u8		RIFF[4];
	u32		riff_size;
	u8		riff_kind[4];
	u8		fmt[4];
	u32		fmt_chnk;
	u16		fmt_id;
	u16		Nch;
	u32		fs;
	u32		dts;
	u16		bl_size;
	u16		bit;
	u16		fmt_ext_size;
	u8		*cbuf;
	u8		data_chnk[4];
	u32		Nbyte;
	short	*data;
} WavHeader;

extern int wav_readfile(WavHeader *header, char *fn);
extern void free_wavheader(WavHeader *header);

#endif //WAV_UTIL_H_
