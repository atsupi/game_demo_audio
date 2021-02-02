/******************************************************
 *    Filename:     lq070out.h 
 *     Purpose:     LQ070 LCD display driver
 *  Created on: 	2021/01/20
 * Modified on:		2021/01/26
 *      Author: 	atsupi.com 
 *     Version:		1.20
 ******************************************************/

#ifndef LQ070OUT_H_
#define LQ070OUT_H_

// definitions for LQ070 control IP
#define LQ070OUT_BASE_ADDR		XPAR_AXI_LQ070_OUT_0_BASEADDR
#define LQ070OUT_MODE_OFFSET	0x00
#define   LQ070_PG_NONE		  	0x00		// Pass-through
#define   LQ070_PG_CBAR		  	0x01		// Color Bar pattern
#define   LQ070_PG_LAMP		  	0x02		// Lamp pattern (64 step)
#define   LQ070_PG_HTCH		  	0x03		// Hatch pattern

// definition of LQ070out instance
typedef struct _LQ070outInstance {
	u32 baseAddress;						// physical address
	u32 virtAddress;						// virtual address
} LQ070outInstance;


// external functions
extern void lq070out_init(LQ070outInstance *inst, u32 baseAddr);
extern void lq070out_deinit(LQ070outInstance *inst);
extern void lq070out_setmode(LQ070outInstance *inst, int mode);

extern void lq070out_write_reg(u32 adr, u32 offset, u32 value);


#endif // LQ070OUT_H_
