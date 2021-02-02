/******************************************************
 *    Filename:     iicps.h 
 *     Purpose:     IIC PS interfce 
 *  Created on: 	2021/01/31
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

#ifndef IICPS_H_
#define IICPS_H_

#define InputClockHz				108333336

// Register offsets for the IIC PS
#define IICPS_CR_OFFSET					0x00
#define IICPS_SR_OFFSET					0x04
#define IICPS_ADDR_OFFSET				0x08
#define IICPS_DATA_OFFSET				0x0C
#define IICPS_ISR_OFFSET				0x10
#define IICPS_TRANS_SIZE_OFFSET			0x14
#define IICPS_SLV_PAUSE_OFFSET			0x18
#define IICPS_TIME_OUT_OFFSET			0x1C
#define IICPS_IMR_OFFSET				0x20
#define IICPS_IER_OFFSET				0x24
#define IICPS_IDR_OFFSET				0x28

// definitions for Control Register
#define IICPS_CR_HOLD					0x00000010

#define IICPS_FIFO_DEPTH				16

// Max TX size
#define IICPS_MAX_TRANSFER_SIZE			252

#define RX								0
#define TX								1

extern u32 iicps_config(void);
extern int iicps_setsclk(u32 BaseAddress, u32 FsclHz);
extern int iicps_isbusy(u32 BaseAddress);
extern int iicps_master_sendpolled(u32 BaseAddr, u8 *MsgPtr, int ByteCount, u16 SlaveAddr);
extern int iicps_master_recvpolled(u32 BaseAddr, u8 *MsgPtr, int ByteCount, u16 SlaveAddr);

#endif /* IICPS_H_ */
