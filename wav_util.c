/******************************************************
 *    Filename:     wav_util.c 
 *     Purpose:     WAV audio file utility
 *  Target Plf:     ZYBO (azplf)
 *  Created on: 	2021/02/02
 *      Author: 	atsupi.com
 *     Version:		0.80
 ******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "azplf_bsp.h"
#include "wav_util.h"

//#define _DEBUG

static int read1byte(FILE *fp, char *result, int len)
{
	int data = fread(result, 1, len, fp);
	return (data);
}

static int read2byte(FILE *fp, short *result)
{
	u8 buf[2];
	fread(buf, 1, 2, fp);
	*result = (buf[1] << 8) | buf[0];
	return 2;
}

static int read4byte(FILE *fp, u32 *result)
{
	u8 buf[4];
	fread(buf, 1, 4, fp);
	*result = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
	return 4;
}

int wav_readfile(WavHeader *header, char *fn)
{
	FILE *fp;
	int result;

	fp = fopen(fn, "r");
	printf("File opened with id(0x%x)\r\n", (unsigned int)fp);
	read1byte(fp, header->RIFF, 4);
	read4byte(fp, &header->riff_size);
	read1byte(fp, header->riff_kind, 4);
	read1byte(fp, header->fmt, 4);					// fmt chunk
	read4byte(fp, &header->fmt_chnk);				// Nbytes of fmt chunk
	read2byte(fp, &header->fmt_id);
	read2byte(fp, &header->Nch);
	read4byte(fp, &header->fs);
	read4byte(fp, &header->dts);
	read2byte(fp, &header->bl_size);
	read2byte(fp, &header->bit);

	if (header->fmt_chnk != 16)					// extension chunk
	{
		read2byte(fp, &header->fmt_ext_size);		// Nbytes for extension
		header->cbuf = (char *)calloc((size_t)header->fmt_ext_size, sizeof(u8));
		read1byte(fp, header->cbuf, header->fmt_ext_size);
	}
	read1byte(fp, header->data_chnk, 4);			// data chunk
	read4byte(fp, &header->Nbyte);
	header->data = calloc(header->Nbyte, sizeof(short));
	if ((result = read1byte(fp, (char *)header->data, header->Nbyte)) < header->Nbyte)
		result = 1;
	fclose(fp);
	return (result);
}

void free_wavheader(WavHeader *header)
{
	if (header->cbuf) {
		free(header->cbuf);
		header->cbuf = 0;
	}
	if (header->data) {
		free(header->data);
		header->data = 0;
	}
}
