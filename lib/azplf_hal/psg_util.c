/******************************************************
 *    Filename:     psg_util.c 
 *     Purpose:     PSG audio utility
 *  Target Plf:     ZYBO (azplf)
 *  Created on: 	2021/02/07
 * Modified on:     2021/02/12
 *      Author: 	atsupi.com
 *     Version:		0.9
 ******************************************************/

//#define _DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include "psg_util.h"

#define PSG_FRAME_SIZE			1200
#define VAL_NO_TONE			0x7FFFFFFF
#define TBL_tone_no			8
#define FIRST_TBL_TONE		3

typedef struct _Ch_Data {
	char mml[MAX_MML_LEN];
	int fr_value;
	int fr_tune;
	int mml_pos;
	int slice_pos;
	int frame_pos;
	int fr_counter;
	int len;
	int def_len;
	int def_keylen;
	int tempo;
	int octave;
	int local_vol;
	int int_vol;
	int tone_rate;
	int pwm_rate;
	int env_no;
	int env_len;
	int tone_no;
	int pitch_no;
	int tune_depth;
	int tie;
} Ch_Data;

static int  skip_frame = 0;
static u32  pcm[CH_NUM][PSG_FRAME_SIZE];
static char tone_tbl[TBL_tone_no][256];
static int  init_tonetbl = 0;

// volume curve table
static float vol_curve[] = {
	 0,  1,   4,   9,  16,  25,  36,  49,  64, 81, 100, 121, 144, 169, 204, 256.0
};

// channel data table
// please put "r" in every channel if no note is attached. 
static Ch_Data psg[CH_NUM] = {
	{
		"r16",
		0,		// fr_value(c~b)
		0,		// fr_tune
		0,		// mml_pos
		0,		// slice_pos
		0,		// frame_pos,
		0,		// fr_counter
		0,		// len
		30*800,	// def_len: T120L4 = 30*800[1/60sec]
		4,		// def_keylen(l)
		120,	// tempo(t)
		4,		// octave(o)
		15,		// local_vol(v) (0~15)
		15,		// int_vol
		8,		// tone_rate(q) (1~8)
		50,		// pwm_rate(x) (1~99)
		0,		// env_no(s) (0:flat 1~3)
		1,		// env_len(m)
		0,		// tone_no(@) (0:none 1)
		0,		// pitch_no(p)
		0,		// tune_depth(h) (0~255)
		0,		// tie
	},
	{
		"t120l8v8x25q6r16",
		0,		// fr_value(c~b)
		0,		// fr_tune
		0,		// mml_pos
		0,		// slice_pos
		0,		// frame_pos,
		0,		// fr_counter
		0,		// len
		30*800,	// def_len: T120L4 = 30*800[1/60sec]
		4,		// def_keylen(l)
		120,	// tempo(t)
		4,		// octave(o)
		15,		// local_vol(v) (0~15)
		15,		// int_vol
		8,		// tone_rate(q) (1~8)
		50,		// pwm_rate(x) (1~99)
		0,		// env_no(s) (0:flat 1~3)
		1,		// env_len(m)
		0,		// tone_no(@) (0:none 1)
		0,		// pitch_no(p)
		0,		// tune_depth(h) (0~255)
		0,		// tie
	},
	{
		"t60l4v15x12@4o1cego2cego3cego4cego5cego6cego7cegr2.", //mml
		0,		// fr_value(c~b)
		0,		// fr_tune
		0,		// mml_pos
		0,		// slice_pos
		0,		// frame_pos,
		0,		// fr_counter
		0,		// len
		30*800,	// def_len: T120L4 = 30*800[1/60sec]
		4,		// def_keylen(l)
		120,	// tempo(t)
		4,		// octave(o)
		15,		// local_vol(v) (0~15)
		15,		// int_vol
		8,		// tone_rate(q) (1~8)
		50,		// pwm_rate(x) (1~99)
		0,		// env_no(s) (0:flat 1~3)
		1,		// env_len(m)
		0,		// tone_no(@) (0:none 1)
		0,		// pitch_no(p)
		0,		// tune_depth(h) (0~255)
		0,		// tie
	},
	{
		// no play
		"t60r", //mml
		0,		// fr_value(c~b)
		0,		// fr_tune
		0,		// mml_pos
		0,		// slice_pos
		0,		// frame_pos,
		0,		// fr_counter
		0,		// len
		30*800,	// def_len: T120L4 = 30*800[1/60sec]
		4,		// def_keylen(l)
		120,	// tempo(t)
		4,		// octave(o)
		15,		// local_vol(v) (0~15)
		15,		// int_vol
		8,		// tone_rate(q) (1~8)
		50,		// pwm_rate(x) (1~99)
		0,		// env_no(s) (0:flat 1~3)
		1,		// env_len(m)
		0,		// tone_no(@) (0:none 1)
		0,		// pitch_no(p)
		0,		// tune_depth(h) (0~255)
		0,		// tie
	},
};

static int tone12_tbl[7] = {	0,  2,  4,  5,  7,  9, 11 };

static float freq_tbl[84] = { 
	  32.7,   34.7,   36.7,   38.9,   41.2,   43.7,   46.3,   49.0,   51.9,   55.0,   58.3,   61.7,
	  65.4,   69.3,   73.4,   77.8,   82.4,   87.3,   92.5,   98.0,  103.8,  110.0,  116.5,  123.5,
	 130.8,  138.6,  146.8,  155.6,  164.8,  174.6,  185.0,  196.0,  207.7,  220.0,  233.1,  246.9,
	 261.6,  277.2,  293.7,  311.1,  329.6,  349.2,  370.0,  392.0,  415.3,  440.0,  466.2,  493.9,
	 523.3,  554.4,  587.3,  622.3,  659.3,  698.5,  740.0,  784.0,  830.6,  880.0,  932.3,  987.8,
	1046.5, 1108.7, 1174.7, 1244.5, 1318.5, 1396.9, 1480.0, 1568.0, 1661.2, 1760.0, 1864.7, 1975.5,
	2093.0, 2217.5, 2349.3, 2489.0, 2637.0, 2793.8, 2960.0, 3136.0, 3322.4, 3520.0, 3729.3, 3951.1,
};

static short GenTableToneWave(Ch_Data *inst)
{
	int freq_value = inst->fr_value + inst->fr_tune;
	float int_pos = 256 * inst->fr_counter / freq_value;
	short data;

	data = tone_tbl[inst->tone_no - FIRST_TBL_TONE][(int)(int_pos)] << 8;

	if (++inst->fr_counter >= freq_value) inst->fr_counter = 0;
	return (data);
}

static short GenPsgWave(Ch_Data *inst)
{
	short data;
	float tone_dur = inst->len * inst->tone_rate / 8;
	float pwm_dur;
	int freq_value = inst->fr_value + inst->fr_tune;

	if ((int)tone_dur < inst->slice_pos) {
		data = 0x0;
	} else {
		pwm_dur = (freq_value) * (100 - inst->pwm_rate) / 100;
		if (inst->fr_counter < (int)pwm_dur)
			data = 0x0;
		else
			data = 0x7FFF;
	}

	if (++inst->fr_counter >= freq_value) inst->fr_counter = 0;

	return (data);
}

static short GenAudioWaveform(Ch_Data *inst)
{
	if (inst->tone_no < FIRST_TBL_TONE)
		return (GenPsgWave(inst));
	else if (inst->tone_no < TBL_tone_no + FIRST_TBL_TONE)
		return (GenTableToneWave(inst));
	else
		return (GenPsgWave(inst));
}

static void InitToneTable(int num)
{
	int i;
	float data;
	short sdata;

#ifdef _DEBUG
	switch (num) {
	case 3: // triange wave
		printf("Triangle wave generated.\n");
		break;
	case 4: // sine wave
		printf("Sine wave generated.\n");
		break;
	case 5: // saw wave
		printf("Saw wave generated.\n");
		break;
	default:
		printf("None of wave generated.\n");
		break;
	}
#endif

	for (i = 0; i < 256; i++) {
		switch (num) {
		case 3: // triange wave
			tone_tbl[num-FIRST_TBL_TONE][i] = (i < 128)? i * 2 - 128: 383 - i * 2;
			break;
		case 4: // sine wave
			data = (float)sin(2 * M_PI * i / 256) * 127;
			sdata = (short)data;
			tone_tbl[num-FIRST_TBL_TONE][i] = (char)sdata;
			break;
		case 5: // saw wave
			tone_tbl[num-FIRST_TBL_TONE][i] = i - 128;
			break;
		default:
			tone_tbl[num-FIRST_TBL_TONE][i] = rand() % 0xff;
			break;
		}

#ifdef _DEBUG
		printf("%02x ", (u8)tone_tbl[num-FIRST_TBL_TONE][i]);
		if (i % 16 == 15) printf("\n");
#endif
	}
}

static void SkipDigits(Ch_Data *inst)
{
	char ch = inst->mml[inst->mml_pos];
	while (isdigit(ch)) {
		inst->mml_pos++;
		ch = inst->mml[inst->mml_pos];
	}
}

static void ProcessPitch(Ch_Data *inst)
{
	if (!inst->pitch_no)
		return;

	switch (inst->pitch_no) {
	case 1: // vibrato
		if (inst->frame_pos < 8) break;
		if (inst->env_len == 1) {
			inst->fr_tune = 
				((inst->frame_pos - 8) % 2)? -inst->tune_depth: inst->tune_depth;
		} else {
			inst->fr_tune = 
				(((inst->frame_pos - 8) % inst->env_len) < inst->env_len / 2)? 0: -inst->tune_depth;
		}
		break;
	case 2: // synth drum
		if (inst->env_len == 1 || inst->frame_pos % inst->env_len == 0) {
			inst->fr_tune += inst->tune_depth;
		}
		break;
	case 3: // pitch up
		if (inst->env_len == 1 || inst->frame_pos % inst->env_len == 0) {
			inst->fr_tune -= inst->tune_depth;
		}
		break;
	default:
		break;
	}
}

static void ProcessEnvelope(Ch_Data *inst)
{
	float target;

	if (!inst->frame_pos || !inst->env_no) // flat
		return;

	switch (inst->env_no) {
	case 1: // down to 75% of local volume
		target = inst->local_vol * 0.75;
		if (inst->int_vol > target) inst->int_vol--;
		break;
	case 2: // down to 25% of local volume
		target = inst->local_vol * 0.50;
		if (inst->int_vol > target) {
			inst->int_vol -= 2;
			if (inst->int_vol < 0) inst->int_vol = 0;
		}
		break;
	case 3: // down to 0% of local volume
		if (inst->env_len == 1 || inst->frame_pos % inst->env_len == 0) {
			if (inst->int_vol) inst->int_vol--;
		}
		break;
	default:
		break;
	}
}

void PlayMusicSlice(int debug_mode)
{
	int i, j;
	float len;
	float def_len;
	char note;
	int note_p;
	int vol = azplf_audio_get_volume();
	float data;
	int num_data[CH_NUM];
	int tone_set;
	int key_len;
	u32 pcm_data;
	u32 l_pcm;
	u32 r_pcm;
	Ch_Data *ppsg;

	if (!init_tonetbl) {
		for (i = FIRST_TBL_TONE; i < FIRST_TBL_TONE + TBL_tone_no; i++)
			InitToneTable(i);
		init_tonetbl = 1;
	}

	if (skip_frame) {
		skip_frame--;
		return; // skipped at once
	}

	for (j = 0; j < CH_NUM; j++) {
		num_data[j] = 0;
		ppsg = &psg[j];

		ppsg->frame_pos++;
		ProcessEnvelope(ppsg);
		ProcessPitch(ppsg);

		for (i = 0; i < PSG_FRAME_SIZE; i++) {
			if (ppsg->fr_value) {
				data = GenAudioWaveform(ppsg);
				data *= vol_curve[ppsg->int_vol] / 256 * vol_curve[vol] / 256;
				pcm[j][num_data[j]++] = (u32)(((u16)data << 16) | (u16)data); // L=data; R=data
			}
	
			if (ppsg->len && ++ppsg->slice_pos < ppsg->len) continue;
	
			tone_set = 0;
			while (!tone_set) {
				if (!ppsg->mml[ppsg->mml_pos]) ppsg->mml_pos = 0;
	
				note = tolower(ppsg->mml[ppsg->mml_pos++]);
	
				if (note < '#' || note == '.' || isdigit(note)) // not correct
					continue;
	
				if (note == '&') {
					ppsg->tie = 1;
					continue;
				}
	
				if (note == 'q') {
					note = ppsg->mml[ppsg->mml_pos];
					if (note >= '1' && note <= '8') {
						ppsg->tone_rate = atoi(&ppsg->mml[ppsg->mml_pos]);
						SkipDigits(ppsg);
					}
					continue;
				}
	
				if (note == 'v') {
					ppsg->local_vol = atoi(&ppsg->mml[ppsg->mml_pos]);
					SkipDigits(ppsg);
					continue;
				}
	
				if (note == 'o') {
					ppsg->octave = atoi(&ppsg->mml[ppsg->mml_pos]) - 1;
					SkipDigits(ppsg);
					continue;
				}
	
				if (note == '>' || note == '<') {
					if (note == '>' && ppsg->octave < 7)
						ppsg->octave++;
					else if (ppsg->octave > 0)
						ppsg->octave--;
					continue;
				}
	
				if (note == 'x') {
					ppsg->pwm_rate = atoi(&ppsg->mml[ppsg->mml_pos]);
					SkipDigits(ppsg);
					continue;
				}
	
				if (note == 's') {
					ppsg->env_no = atoi(&ppsg->mml[ppsg->mml_pos]);
					SkipDigits(ppsg);
					continue;
				}
	
				if (note == 'm') {
					ppsg->env_len = atoi(&ppsg->mml[ppsg->mml_pos]);
					if (ppsg->env_len < 0) ppsg->env_len = 1;
					SkipDigits(ppsg);
					continue;
				}
	
				if (note == '@') {
					ppsg->tone_no = atoi(&ppsg->mml[ppsg->mml_pos]);
					SkipDigits(ppsg);
					continue;
				}
	
				if (note == 'p') {
					ppsg->pitch_no = atoi(&ppsg->mml[ppsg->mml_pos]);
					SkipDigits(ppsg);
					continue;
				}
	
				if (note == 'h') {
					ppsg->tune_depth = atoi(&ppsg->mml[ppsg->mml_pos]);
					SkipDigits(ppsg);
					continue;
				}
	
				if (note == 't' || note == 'l') {
					def_len = 800 * 60; // 1min
					if (note == 't')
						ppsg->tempo = atoi(&ppsg->mml[ppsg->mml_pos]);
					else if (note == 'l')
						ppsg->def_keylen = atoi(&ppsg->mml[ppsg->mml_pos]);
					def_len = def_len * 60 / ppsg->tempo * 4 / ppsg->def_keylen;
					ppsg->def_len = (int)def_len;
					SkipDigits(ppsg);
					continue;
				}
		
				if (note == 'r') {
					ppsg->fr_value = VAL_NO_TONE;
					tone_set = 1;
				} else {
					if (note < 'c') note += 7; // a or b
					note_p = tone12_tbl[note - 'c'] + ppsg->octave * 12; // 7 * 12 = 84 (keys)
		
					if (note_p > sizeof(freq_tbl)/sizeof(freq_tbl[0])) {
						ppsg->fr_value = VAL_NO_TONE;
					} else {
						if (ppsg->mml[ppsg->mml_pos] == '#' || ppsg->mml[ppsg->mml_pos] == '+') {
							note_p++;
							ppsg->mml_pos++;
						} else if (ppsg->mml[ppsg->mml_pos] == '-') {
							note_p--;
							ppsg->mml_pos++;
						}
						len = 48000.0 / freq_tbl[note_p];
						ppsg->fr_value = (int)len;
						if (!ppsg->tie) 
							ppsg->int_vol = ppsg->local_vol;
					}
					tone_set = 1;
				}
	
				if (tone_set) {
					key_len = atoi(&ppsg->mml[ppsg->mml_pos]);
					if (!key_len)  {
						len = ppsg->def_len;
					} else {
						SkipDigits(ppsg);
						len = 800 * 60; // 1min
						len = len * 60 / ppsg->tempo * 4 / key_len;
					}
					if (ppsg->tie) {
						// bond between old and new key
						ppsg->len += len;
						ppsg->tie  = 0;
					} else {
						// start new key
						ppsg->len        = len;
						ppsg->slice_pos  = 0;
						ppsg->frame_pos  = 0;
						ppsg->fr_counter = 0;
						ppsg->fr_tune    = 0;
					}
					if (ppsg->mml[ppsg->mml_pos] == '.') {
						ppsg->len += len / 2;
						ppsg->mml_pos++;
					}
				}
	
#ifdef _DEBUG
				if (j == 0) {
					printf("[PSG1] mml_pos = %d, fr_value = %d, pwm_rate = %d, len = %d\n", ppsg->mml_pos, ppsg->fr_value, ppsg->pwm_rate, ppsg->len);
					printf("       slice_pos = %d, tempo = %d, def_len = %d, def_keylen = %d\n", ppsg->slice_pos, ppsg->tempo, ppsg->def_len, ppsg->def_keylen);
				}
#endif
			}
		}
	}
	for (i = 0; i < num_data[0]; i++) {
		while ((i2sout_getstatus(0x0C) & 0xC));

		l_pcm = (pcm[0][i] >> 16) + 
				(pcm[1][i] >> 16) + 
				(pcm[2][i] >> 16) + 
				(pcm[3][i] >> 16);
		r_pcm = (pcm[0][i] & 0xffff) + 
				(pcm[1][i] & 0xffff) + 
				(pcm[2][i] & 0xffff) + 
				(pcm[3][i] & 0xffff);
		pcm_data = (l_pcm >> 2) << 16 | (r_pcm >> 2);
		i2sout_senddata(4, pcm_data);
	}
}

void AttachMMLData(int ch, char *data)
{
	if (ch < 0 || ch > CH_NUM) return;

	strcpy(psg[ch].mml, data);
}

int LoadMMLData(char *fn)
{
	FILE *fp;
	int ch = 0;
	char data[MAX_MML_LEN];
	char *p;

	fp = fopen(fn, "r");
	if (!fp) {
		printf("Error: Cannot open MML data file.\n");
		return (PST_FAILURE);
	}

	while (p = fgets(data, sizeof(data), fp)) {
		AttachMMLData(ch, data);
		if (++ch >= CH_NUM) break;
	}
	fclose(fp);
	return (PST_SUCCESS);
}
