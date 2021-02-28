/******************************************************
 *    Filename:     azplf_bsp.h 
 *     Purpose:     azplf BSP header 
 *  Target Plf:     azplf (ZYBO learning platform) 
 *  Created on: 	2021/01/18 
 * Modified on:     2021/02/27
 *      Author: 	atsupi.com 
 *     Version:		1.00 
 ******************************************************/

#ifndef AZPLF_BSP_H_
#define AZPLF_BSP_H_

// hardware definitions 
#define XPAR_PS7_DDR_0_S_AXI_BASEADDR	0x00100000

#define Use_GfxAccel
#define XPAR_XGFXACCEL_0_BASEADDR		0x40000000

#define Use_LQ070out
#define XPAR_AXI_LQ070_OUT_0_BASEADDR	0x44A10000

#define VDMA_INSTANCE_NUM				1
#define XPAR_AXI_VDMA_0_BASEADDR		0x44A00000
//#define XPAR_AXI_VDMA_1_BASEADDR		0x44A20000

#define IIC_INSTANCE_NUM				1
#define IIC_BASEADDR					0xE0004000 // IICPS_I2C_0_BASEADDR
//#define IIC_BASEADDR					0xE0005000 // IICPS_I2C_1_BASEADDR

#define Use_I2Sout
#define I2SOUT_BASEADDR					0x43C00000
#define SSM2603_IIC_ADDRESS				0x1A

// platform status result
#define PST_SUCCESS						(0)
#define PST_FAILURE						(1)
#define PST_VDMA_MISMATCH_ERROR			(2)

// display parameters 
#ifdef (Use_LQ070out)
#define DISP_WIDTH						800
#define DISP_HEIGHT						480
#else
#define DISP_WIDTH						1024
#define DISP_HEIGHT						768
#endif

// Definition for Frame buffer and Color
// 10bit per color
#define FB(addr, offset)		(*(volatile unsigned int *)((unsigned int *)(addr)+(unsigned int)(offset)))
#define RGB1(r, g, b)  (((((r) & 1) * 0x3ff << 20) | ((g) & 1) * 0x3ff << 10) | (((b) & 1) * 0x3ff))
#define RGB8(r, g, b)  ((((r) & 0xff) << 22) | (((g) & 0xff) << 12) | (((b) & 0xff) << 2))
#define RGB10(r, g, b) ((((r) & 0x3ff) << 20) | (((g) & 0x3ff) << 10) | ((b) & 0x3ff))

#define swap(a, b)		{ u32 c = a; a = b; b = c; }

// type definitions 
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct _pos {
	int x;
	int y;
} pos;


#endif //AZPLF_BSP_H_
