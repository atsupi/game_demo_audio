/******************************************************
 *    Filename:     gfxaccel.h 
 *     Purpose:     graphics accelerator driver
 *  Created on: 	2021/01/18
 * Modified on:		2021/01/26
 *      Author: 	atsupi.com 
 *     Version:		0.90
 ******************************************************/

#ifndef GFXACCEL_H_
#define GFXACCEL_H_

#define GFXACCEL_BASE_ADDR			XPAR_XGFXACCEL_0_BASEADDR

// Definition for Gfxaccel IP
#define GFXACCEL_MODE_FILLRECT		1
#define GFXACCEL_MODE_LINE			2
#define GFXACCEL_MODE_BITBLT		3

#define GFXACCEL_BB_NONE			0
#define GFXACCEL_BB_OR				1
#define GFXACCEL_BB_AND				2
#define GFXACCEL_BB_XOR				3

/* Graphics Accelerator HW IP
-- ------------------------Address Info-------------------
-- 0x00 : Control signals
--        bit 0  - ap_start (Read/Write/COH)
--        bit 1  - ap_done (Read/COR)
--        bit 2  - ap_idle (Read)
--        bit 3  - ap_ready (Read)
--        bit 7  - auto_restart (Read/Write)
--        others - reserved
-- 0x04 : Global Interrupt Enable Register
-- 0x08 : IP Interrupt Enable Register (Read/Write)
-- 0x0c : IP Interrupt Status Register (Read/TOW)
-- 0x10 : Data signal of src_fb
--        bit 31~0 - src_fb[31:0] (Read/Write)
-- 0x18 : Data signal of x1
--        bit 15~0 - x1[15:0] (Read/Write)
-- 0x20 : Data signal of y1
--        bit 15~0 - y1[15:0] (Read/Write)
-- 0x28 : Data signal of dx
--        bit 15~0 - dx[15:0] (Read/Write)
-- 0x30 : Data signal of dy
--        bit 15~0 - dy[15:0] (Read/Write)
-- 0x38 : Data signal of dst_fb
--        bit 31~0 - dst_fb[31:0] (Read/Write)
-- 0x40 : Data signal of x2
--        bit 15~0 - x2[15:0] (Read/Write)
-- 0x48 : Data signal of y2
--        bit 15~0 - y2[15:0] (Read/Write)
-- 0x50 : Data signal of col
--        bit 31~0 - col[31:0] (Read/Write)
-- 0x58 : Data signal of mode
--        bit 7~0 - mode[7:0] (Read/Write)
*/
#define GFXACCEL_CONTROL_ADDR_AP_CTRL     0x00
#define GFXACCEL_CONTROL_ADDR_GIE         0x04
#define GFXACCEL_CONTROL_ADDR_IER         0x08
#define GFXACCEL_CONTROL_ADDR_ISR         0x0c
#define GFXACCEL_CONTROL_ADDR_SRC_FB_DATA 0x10
#define GFXACCEL_CONTROL_ADDR_X1_DATA     0x18
#define GFXACCEL_CONTROL_ADDR_Y1_DATA     0x20
#define GFXACCEL_CONTROL_ADDR_DX_DATA     0x28
#define GFXACCEL_CONTROL_ADDR_DY_DATA     0x30
#define GFXACCEL_CONTROL_ADDR_DST_FB_DATA 0x38
#define GFXACCEL_CONTROL_ADDR_X2_DATA     0x40
#define GFXACCEL_CONTROL_ADDR_Y2_DATA     0x48
#define GFXACCEL_CONTROL_ADDR_COL_DATA    0x50
#define GFXACCEL_CONTROL_ADDR_MODE_DATA   0x58
#define GFXACCEL_CONTROL_ADDR_OP_DATA     0x60

// instance definition
typedef struct _GfxaccelInstance {
	u32 baseAddress;						// physical address
	u32 virtAddress;						// virtual address
} GfxaccelInstance;

// external functions
extern u32 gfxaccel_init(GfxaccelInstance *inst, u32 baseAddr);
extern void gfxaccel_deinit(GfxaccelInstance *inst);
extern void gfxaccel_start(GfxaccelInstance *inst);
extern void gfxaccel_stop(GfxaccelInstance *inst);
extern u32 gfxaccel_isidle(GfxaccelInstance *inst);
extern u32 gfxaccel_isready(GfxaccelInstance *inst);
extern void gfxaccel_fill_rect(GfxaccelInstance *inst, u32 fb, u16 x1, u16 y1, u16 x2, u16 y2, u32 col);
extern void gfxaccel_draw_line(GfxaccelInstance *inst, u32 fb, u16 x1, u16 y1, u16 x2, u16 y2, u32 col);
extern void gfxaccel_bitblt(GfxaccelInstance *inst, u32 src_fb, u16 x1, u16 y1, u16 dx, u16 dy, u32 dst_fb, u16 x2, u16 y2, u8 op);


#endif // GFXACCEL_H_
