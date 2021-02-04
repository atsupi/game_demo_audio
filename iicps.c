/******************************************************
 *    Filename:     iicps.c 
 *     Purpose:     IIC PS interfce 
 *  Created on: 	2021/01/31
 * Modified on:
 *      Author: 	atsupi.com 
 *     Version:		0.80
 ******************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "azplf_bsp.h"
#include "iicps.h"

//#define _DEBUG

// definition for SLCR
#define SLCR_BASEADDR					0xF8000000
// SLCR unlock register
#define SLCR_UNLOCK_ADDR				(0x8)
#define SLCR_APER_CLK_CTRL_ADDR			(0x12C)
// SLCR I2C reset control address register
#define SLCR_I2C_RST_CTRL_ADDR			(0x224)
// SLCR unlock code
#define SLCR_UNLOCK_CODE				0x0000DF0D
#define SLCR_I2C_RST_CTRL_VAL			0x3

static u32 page_size;

static u8 *SendBufferPtr;	/* Pointer to send buffer */
static u8 *RecvBufferPtr;	/* Pointer to recv buffer */
static int SendByteCount;	/* Number of bytes still expected to send */
static int RecvByteCount;	/* Number of bytes still expected to receive */

static u32 iicps_readreg(u32 adr, u32 offset)
{
	volatile u32 *p = (u32 *)(adr + offset);
	return (*p);
}

static void iicps_writereg(u32 adr, u32 offset, u32 value)
{
	volatile u32 *p = (u32 *)(adr + offset);
	*p = value;
#if defined (_DEBUG)
	u32 data = iicps_readreg(adr, offset);
	printf("iicps_writereg(offset:0x%04x) %x : readreg %x\n", offset, value, data);
#endif
}

static void iicps_sendbyte(BaseAddress)
{
	iicps_writereg((BaseAddress), IICPS_DATA_REG, *SendBufferPtr++);
	SendByteCount--;
}

static void iicps_recvbyte(BaseAddress)
{
	*RecvBufferPtr++ = (u8)iicps_readreg(BaseAddress, IICPS_DATA_REG);
	RecvByteCount--;
}

void iicps_abort(u32 BaseAddr)
{
	u32 maskreg;
	u32 status;

	// Pause all of interrupts
	maskreg = iicps_readreg(BaseAddr, IICPS_IMR_REG);
	iicps_writereg(BaseAddr, IICPS_IDR_REG, 0x2FF);

	// Reset control register and clear FIFO
	iicps_writereg(BaseAddr, IICPS_CTRL_REG, 0x40);

	// Read and write interrupt status to consume all of interrupts
	status = iicps_readreg(BaseAddr, IICPS_ISR_REG);
	iicps_writereg(BaseAddr, IICPS_ISR_REG, status);

	// Restore interrupt status
	maskreg &= ~maskreg;
	iicps_writereg(BaseAddr, IICPS_IER_REG, maskreg);
}

void iicps_reset(u32 BaseAddr)
{
	// Force to abort transactions
	iicps_abort(BaseAddr);

	// Reset registers
	iicps_writereg(BaseAddr, IICPS_CTRL_REG, 0);
	iicps_writereg(BaseAddr, IICPS_TIME_OUT_REG, 0x1F);
	iicps_writereg(BaseAddr, IICPS_IDR_REG, 0x000002FF);
}

static u32 slcr_readreg(u32 adr, u32 offset)
{
	return (*(volatile u32 *)(adr + offset));
}

static void slcr_writereg(u32 adr, u32 offset, u32 value)
{
	*(volatile u32 *)(adr + offset) = value;
#if defined (_DEBUG)
	u32 data = slcr_readreg(adr, offset);
	printf("slcr_readreg(offset:0x%04x) %x : readreg 0x%x\n", offset, value, data);
#endif
}

static void slcr_iicps_reset(void)
{
	u32 RegVal;
	int fd;
	u32 ptr;
	fd = open("/dev/mem", O_RDWR);
	ptr = (u32)mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, SLCR_BASEADDR);
	printf("Mapping I/O: 0x%08x to vmem: 0x%08x\n", SLCR_BASEADDR, ptr);
	close(fd);

	RegVal = slcr_readreg(ptr, SLCR_APER_CLK_CTRL_ADDR);
#if defined (_DEBUG)
	printf("slcr_readreg(offset:0x%04x) : readreg 0x%x\n", SLCR_APER_CLK_CTRL_ADDR, RegVal);
#endif

	// !!! IMPORTANT !!!
	// slcr.APER_CLK_CTRL
//	RegVal |= 1<<19;		// I2C1
	RegVal |= 1<<18;		// I2C0
//	RegVal |= 1<<15;		// SPI1
//	RegVal |= 1<<14;		// SPI0
	slcr_writereg(ptr, SLCR_APER_CLK_CTRL_ADDR, RegVal);

	// Unlock slcr register
	slcr_writereg(ptr, SLCR_UNLOCK_ADDR, SLCR_UNLOCK_CODE);
	// send reset command
	RegVal = slcr_readreg(ptr, SLCR_I2C_RST_CTRL_ADDR);
	RegVal |= SLCR_I2C_RST_CTRL_VAL;
	slcr_writereg(ptr, SLCR_I2C_RST_CTRL_ADDR, RegVal);
//	/* Release the reset */
//	RegVal = slcr_readreg(ptr, SLCR_I2C_RST_CTRL_ADDR);
	RegVal &= ~SLCR_I2C_RST_CTRL_VAL;
	slcr_writereg(ptr, SLCR_I2C_RST_CTRL_ADDR, RegVal);

#if defined (_DEBUG)
	printf("\nCheck Slcr registers\n");
	printf("slcr_readreg(offset:0x%04x) %x : ", SLCR_APER_CLK_CTRL_ADDR, RegVal);
	RegVal = slcr_readreg(ptr, SLCR_APER_CLK_CTRL_ADDR);
	printf("readreg 0x%x\n", RegVal);
#endif

	munmap((void *)ptr, page_size);
}

static int iicps_setup_master(u32 BaseAddr, int Role)
{
	u32 ControlReg;

	ControlReg = iicps_readreg(BaseAddr, IICPS_CTRL_REG);

	ControlReg |= 0x4E;			// Configure master mode
	if (Role == RX)
		ControlReg |= 0x01;
	else
		ControlReg &= 0xFFFFFFFE;

	iicps_writereg(BaseAddr, IICPS_CTRL_REG, ControlReg);
	iicps_writereg(BaseAddr, IICPS_IDR_REG, 0x000002FF);

	return PST_SUCCESS;
}

static int TransmitFifoFill(u32 BaseAddress)
{
	u8 NumBytes;
	int i;
	int NumBytesToSend;

	// Number of bytes to write to FIFO.
	NumBytes = IICPS_FIFO_DEPTH -
		iicps_readreg(BaseAddress, IICPS_TRANS_SIZE_REG);

	if (SendByteCount > NumBytes) {
		NumBytesToSend = NumBytes;
	} else {
		NumBytesToSend = SendByteCount;
	}

	// Send data to FIFO
	for (i = 0; i < NumBytesToSend; i++) {
		iicps_sendbyte(BaseAddress);
	}

	return SendByteCount;
}

int iicps_master_sendpolled(u32 BaseAddr, u8 *MsgPtr, int ByteCount, u16 SlaveAddr)
{
	u32 status;

	SendBufferPtr = MsgPtr;
	SendByteCount = ByteCount;

	if (ByteCount > IICPS_FIFO_DEPTH)
		iicps_writereg(BaseAddr, IICPS_CTRL_REG,
			iicps_readreg(BaseAddr, IICPS_CTRL_REG) | IICPS_CR_HOLD);

	iicps_setup_master(BaseAddr, TX);

	// Clear the interrupt status register before monitoring.
	status = iicps_readreg(BaseAddr, IICPS_ISR_REG);
	iicps_writereg(BaseAddr, IICPS_ISR_REG, status);

	// Transmit FIFO
	TransmitFifoFill(BaseAddr);

	iicps_writereg(BaseAddr, IICPS_ADDRESS_REG, SlaveAddr);

	status = iicps_readreg(BaseAddr, IICPS_ISR_REG);

	// Continue sending
	while (SendByteCount > 0 && (status & 0x244) == 0) {
		status = iicps_readreg(BaseAddr, IICPS_STATUS_REG);
		// Wait until FIFO is empty
		if ((status & 0x40) != 0) {
			status = iicps_readreg(BaseAddr, IICPS_ISR_REG);
			continue;
		}
		// Send data to transmit FIFO.
		TransmitFifoFill(BaseAddr);
	}

	// Check the completion of transmission
	while (!(status & 0x01)){
		status = iicps_readreg(BaseAddr, IICPS_ISR_REG);
		if ((status & 0x244) != 0) {
			return PST_FAILURE;
		}
	}

	iicps_writereg(BaseAddr, IICPS_CTRL_REG,
		iicps_readreg(BaseAddr,IICPS_CTRL_REG) & ~IICPS_CR_HOLD);

	return PST_SUCCESS;
}


int iicps_master_recvpolled(u32 BaseAddr, u8 *MsgPtr, int ByteCount, u16 SlaveAddr)
{
	u32 status;
	int IsHold = 0;
	int UpdateTxSize = 0;

	RecvBufferPtr = MsgPtr;
	RecvByteCount = ByteCount;

	iicps_setup_master(BaseAddr, RX);

	if(ByteCount > IICPS_FIFO_DEPTH) {
		iicps_writereg(BaseAddr, IICPS_CTRL_REG,
			iicps_readreg(BaseAddr, IICPS_CTRL_REG) | IICPS_CR_HOLD);
		IsHold = 1;
	}

	// Clear the interrupt status register before monitoring
	status = iicps_readreg(BaseAddr, IICPS_ISR_REG);
	iicps_writereg(BaseAddr, IICPS_ISR_REG, status);

	iicps_writereg(BaseAddr, IICPS_ADDRESS_REG, SlaveAddr);

	// Set up the transfer size register
	if (ByteCount > IICPS_MAX_TRANSFER_SIZE) {
		iicps_writereg(BaseAddr, IICPS_TRANS_SIZE_REG, IICPS_MAX_TRANSFER_SIZE);
		ByteCount = IICPS_MAX_TRANSFER_SIZE;
		UpdateTxSize = 1;
	}else {
		iicps_writereg(BaseAddr, IICPS_TRANS_SIZE_REG, ByteCount);
	}

	// Poll the interrupt status register
	status = iicps_readreg(BaseAddr, IICPS_ISR_REG);
	while ((RecvByteCount > 0) && ((status & 0x2a4) == 0)) {
		status = iicps_readreg(BaseAddr, IICPS_STATUS_REG);

		while (status & 0x20) {
			if ((RecvByteCount < 14) && IsHold) {
				IsHold = 0;
				iicps_writereg(BaseAddr, IICPS_CTRL_REG,
						iicps_readreg(BaseAddr, IICPS_CTRL_REG) & ~IICPS_CR_HOLD);
			}
			iicps_recvbyte(BaseAddr);
			ByteCount --;

			if (UpdateTxSize && (ByteCount == IICPS_FIFO_DEPTH + 1))
				break;

			status = iicps_readreg(BaseAddr, IICPS_STATUS_REG);
		}

		if (UpdateTxSize && (ByteCount == IICPS_FIFO_DEPTH + 1)) {
			// Wait while fifo is full
			while(iicps_readreg(BaseAddr, IICPS_TRANS_SIZE_REG) != (ByteCount - IICPS_FIFO_DEPTH));

			if ((RecvByteCount - IICPS_FIFO_DEPTH) > IICPS_MAX_TRANSFER_SIZE) {
				iicps_writereg(BaseAddr, IICPS_TRANS_SIZE_REG, IICPS_MAX_TRANSFER_SIZE);
				ByteCount = IICPS_MAX_TRANSFER_SIZE + IICPS_FIFO_DEPTH;
			}else {
				iicps_writereg(BaseAddr, IICPS_TRANS_SIZE_REG, RecvByteCount - IICPS_FIFO_DEPTH);
				UpdateTxSize = 0;
				ByteCount = RecvByteCount;
			}
		}

		status = iicps_readreg(BaseAddr, IICPS_ISR_REG);
	}

	iicps_writereg(BaseAddr, IICPS_CTRL_REG,
			iicps_readreg(BaseAddr,IICPS_CTRL_REG) & ~IICPS_CR_HOLD);

	if (status & 0x2a4) {
		return PST_FAILURE;
	}

	return PST_SUCCESS;
}

int iicps_isbusy(u32 BaseAddress)
{
	u32 status = iicps_readreg(BaseAddress, IICPS_STATUS_REG);
	if (status & 0x100)
		return 1;

	return 0;
}

int iicps_setsclk(u32 BaseAddress, u32 FsclHz)
{
	u32 Div_a;
	u32 Div_b;
	u32 ActualFscl;
	u32 Temp;
	u32 TempLimit;
	u32 LastError;
	u32 BestError;
	u32 CurrentError;
	u32 ControlReg;
	u32 CalcDivA;
	u32 CalcDivB;
	u32 BestDivA = 0;
	u32 BestDivB = 0;

	if (iicps_readreg((BaseAddress), IICPS_TRANS_SIZE_REG)) {
		return PST_FAILURE;
	}

	// Assume Div_a is 0 and calculate (divisor_a+1) x (divisor_b+1).
	Temp = (InputClockHz) / (22 * FsclHz);

	// If frequency 400KHz is selected, 384.6KHz should be set.
	// If frequency 100KHz is selected, 90KHz should be set.
	if(FsclHz > 384600) {
		FsclHz = 384600;
	} else if (FsclHz > 90000 && FsclHz <= 100000) {
		FsclHz = 90000;
	}

	// TempLimit helps in iterating over the consecutive value of Temp to
	// find the closest clock rate achievable with divisors.
	// Iterate over the next value only if fractional part is involved.
	TempLimit = ((InputClockHz) % (22 * FsclHz)) ? Temp + 1 : Temp;
	BestError = FsclHz;

	for ( ; Temp <= TempLimit ; Temp++) {
		LastError = FsclHz;
		CalcDivA = 0;
		CalcDivB = 0;
		CurrentError = 0;
		for (Div_b = 0; Div_b < 64; Div_b++) {
			Div_a = Temp / (Div_b + 1);
			if (Div_a != 0)
				Div_a = Div_a - 1;
			if (Div_a > 3)
				continue;
			ActualFscl = (InputClockHz) / (22 * (Div_a + 1) * (Div_b + 1));
			if (ActualFscl > FsclHz)
				CurrentError = (ActualFscl - FsclHz);
			else
				CurrentError = (FsclHz - ActualFscl);

			if (LastError > CurrentError) {
				CalcDivA = Div_a;
				CalcDivB = Div_b;
				LastError = CurrentError;
			}
		}

		// Capture the best divisors.
		if (LastError < BestError) {
			BestError = LastError;
			BestDivA = CalcDivA;
			BestDivB = CalcDivB;
		}
	}

	// Read the control register and mask the Divisors.
	ControlReg = iicps_readreg(BaseAddress, IICPS_CTRL_REG);
	ControlReg &= 0xFFFF00FF;
	ControlReg |= (BestDivA << 14) | (BestDivB << 8);
	iicps_writereg(BaseAddress, IICPS_CTRL_REG, ControlReg);

	return PST_SUCCESS;
}

u32 iicps_config(void)
{
	int fd;
	u32 IicPsAddress;

	page_size = sysconf(_SC_PAGESIZE);

	fd = open("/dev/mem", O_RDWR);
	IicPsAddress = (u32)mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, IIC_BASEADDR);
	printf("Mapping I/O: 0x%08x to vmem: 0x%08x\n", IIC_BASEADDR, IicPsAddress);
	close(fd);

	if (IicPsAddress == 0 || IicPsAddress == 0xFFFFFFFF)
	{
		printf("Error: Mapping failure\n");
		return 0;
	}

	slcr_iicps_reset();
	iicps_reset(IicPsAddress);

	return (IicPsAddress);
}
