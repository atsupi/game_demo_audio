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
	iicps_writereg((BaseAddress), IICPS_DATA_OFFSET, *SendBufferPtr++);
	SendByteCount--;
}

static void iicps_recvbyte(BaseAddress)
{
	*RecvBufferPtr++ = (u8)iicps_readreg(BaseAddress, IICPS_DATA_OFFSET);
	RecvByteCount--;
}

void iicps_abort(u32 BaseAddr)
{
	u32 maskreg;
	u32 status;

	// Pause all of interrupts
	maskreg = iicps_readreg(BaseAddr, IICPS_IMR_OFFSET);
	iicps_writereg(BaseAddr, IICPS_IDR_OFFSET, 0x2FF);

	// Reset control register and clear FIFO
	iicps_writereg(BaseAddr, IICPS_CR_OFFSET, 0x40);

	// Read and write interrupt status to consume all of interrupts
	status = iicps_readreg(BaseAddr, IICPS_ISR_OFFSET);
	iicps_writereg(BaseAddr, IICPS_ISR_OFFSET, status);

	// Restore interrupt status
	maskreg &= ~maskreg;
	iicps_writereg(BaseAddr, IICPS_IER_OFFSET, maskreg);
}

void iicps_reset(u32 BaseAddr)
{
	// Force to abort transactions
	iicps_abort(BaseAddr);

	// Reset registers
	iicps_writereg(BaseAddr, IICPS_CR_OFFSET, 0);
	iicps_writereg(BaseAddr, IICPS_TIME_OUT_OFFSET, 0x1F);
	iicps_writereg(BaseAddr, IICPS_IDR_OFFSET, 0x000002FF);
}

static u32 slcr_readreg(u32 adr, u32 offset)
{
	return (*(volatile unsigned int *)(adr + offset));
}

static void slcr_writereg(u32 adr, u32 offset, u32 value)
{
	*(volatile unsigned int *)(adr + offset) = value;
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

//	// Check if HOLD bit is not set.
	ControlReg = iicps_readreg(BaseAddr, IICPS_CR_OFFSET);
//	if ((ControlReg & IICPS_CR_HOLD) == 0 && iicps_isbusy(BaseAddr))
//			return PST_FAILURE;

	// Configure master mode
	ControlReg |= 0x4E;

	if (Role == RX)
		ControlReg |= 0x01;
	else
		ControlReg &= 0xFFFFFFFE;

	iicps_writereg(BaseAddr, IICPS_CR_OFFSET, ControlReg);
	iicps_writereg(BaseAddr, IICPS_IDR_OFFSET, 0x000002FF);

	return PST_SUCCESS;
}

static int TransmitFifoFill(u32 BaseAddress)
{
	u8 AvailBytes;
	int LoopCnt;
	int NumBytesToSend;

	/*
	 * Determine number of bytes to write to FIFO.
	 */
	AvailBytes = IICPS_FIFO_DEPTH -
		iicps_readreg(BaseAddress, IICPS_TRANS_SIZE_OFFSET);

	if (SendByteCount > AvailBytes) {
		NumBytesToSend = AvailBytes;
	} else {
		NumBytesToSend = SendByteCount;
	}

	/*
	 * Fill FIFO with amount determined above.
	 */
	for (LoopCnt = 0; LoopCnt < NumBytesToSend; LoopCnt++) {
		iicps_sendbyte(BaseAddress);
	}

	return SendByteCount;
}

int iicps_master_sendpolled(u32 BaseAddr, u8 *MsgPtr,
		 int ByteCount, u16 SlaveAddr)
{
	u32 status;
	u32 Intrs;

	/*
	 * Assert validates the input arguments.
	 */
	SendBufferPtr = MsgPtr;
	SendByteCount = ByteCount;

	if (ByteCount > IICPS_FIFO_DEPTH)
		iicps_writereg(BaseAddr, IICPS_CR_OFFSET,
			iicps_readreg(BaseAddr, IICPS_CR_OFFSET) | IICPS_CR_HOLD);

	iicps_setup_master(BaseAddr, TX);

	/*
	 * Intrs keeps all the error-related interrupts.
	 */
	Intrs = 0x244;

	/*
	 * Clear the interrupt status register before use it to monitor.
	 */
	status = iicps_readreg(BaseAddr, IICPS_ISR_OFFSET);
	iicps_writereg(BaseAddr, IICPS_ISR_OFFSET, status);

	/*
	 * Transmit first FIFO full of data.
	 */
	TransmitFifoFill(BaseAddr);

	iicps_writereg(BaseAddr, IICPS_ADDR_OFFSET, SlaveAddr);

	status = iicps_readreg(BaseAddr, IICPS_ISR_OFFSET);

	/*
	 * Continue sending as long as there is more data and
	 * there are no errors.
	 */
	while ((SendByteCount > 0) &&
		((status & Intrs) == 0)) {
		status = iicps_readreg(BaseAddr, IICPS_SR_OFFSET);

		/*
		 * Wait until transmit FIFO is empty.
		 */
		if ((status & 0x40) != 0) {
			status = iicps_readreg(BaseAddr, IICPS_ISR_OFFSET);
			continue;
		}

		/*
		 * Send more data out through transmit FIFO.
		 */
		TransmitFifoFill(BaseAddr);
	}

	/*
	 * Check for completion of transfer.
	 */
	while (!(status & 0x01)){

		status = iicps_readreg(BaseAddr, IICPS_ISR_OFFSET);
		/*
		 * If there is an error, tell the caller.
		 */
		if ((status & Intrs) != 0) {
			return PST_FAILURE;
		}
	}

	iicps_writereg(BaseAddr, IICPS_CR_OFFSET,
		iicps_readreg(BaseAddr,IICPS_CR_OFFSET) & ~IICPS_CR_HOLD);

	return PST_SUCCESS;
}


int iicps_master_recvpolled(u32 BaseAddr, u8 *MsgPtr,
				int ByteCount, u16 SlaveAddr)
{
	u32 Intrs;
	u32 status;
	int IsHold = 0;
	int UpdateTxSize = 0;

	/*
	 * Assert validates the input arguments.
	 */
	RecvBufferPtr = MsgPtr;
	RecvByteCount = ByteCount;

	iicps_setup_master(BaseAddr, RX);

	if(ByteCount > IICPS_FIFO_DEPTH) {
		iicps_writereg(BaseAddr, IICPS_CR_OFFSET,
			iicps_readreg(BaseAddr, IICPS_CR_OFFSET) | IICPS_CR_HOLD);
		IsHold = 1;
	}

	/*
	 * Clear the interrupt status register before use it to monitor.
	 */
	status = iicps_readreg(BaseAddr, IICPS_ISR_OFFSET);
	iicps_writereg(BaseAddr, IICPS_ISR_OFFSET, status);

	iicps_writereg(BaseAddr, IICPS_ADDR_OFFSET, SlaveAddr);

	/*
	 * Set up the transfer size register so the slave knows how much
	 * to send to us.
	 */
	if (ByteCount > IICPS_MAX_TRANSFER_SIZE) {
		iicps_writereg(BaseAddr, IICPS_TRANS_SIZE_OFFSET, IICPS_MAX_TRANSFER_SIZE);
		ByteCount = IICPS_MAX_TRANSFER_SIZE;
		UpdateTxSize = 1;
	}else {
		iicps_writereg(BaseAddr, IICPS_TRANS_SIZE_OFFSET, ByteCount);
	}

	/*
	 * Intrs keeps all the error-related interrupts.
	 */
	Intrs = 0x2a4;

	/*
	 * Poll the interrupt status register to find the errors.
	 */
	status = iicps_readreg(BaseAddr, IICPS_ISR_OFFSET);
	while ((RecvByteCount > 0) &&
			((status & Intrs) == 0)) {
		status = iicps_readreg(BaseAddr, IICPS_SR_OFFSET);

		while (status & 0x20) {
			if ((RecvByteCount < 14) && IsHold) {
				IsHold = 0;
				iicps_writereg(BaseAddr, IICPS_CR_OFFSET,
						iicps_readreg(BaseAddr, IICPS_CR_OFFSET) & ~IICPS_CR_HOLD);
			}
			iicps_recvbyte(BaseAddr);
			ByteCount --;

			if (UpdateTxSize &&
				(ByteCount == IICPS_FIFO_DEPTH + 1))
				break;

			status = iicps_readreg(BaseAddr, IICPS_SR_OFFSET);
		}

		if (UpdateTxSize && (ByteCount == IICPS_FIFO_DEPTH + 1)) {
			/*
			 * wait while fifo is full
			 */
			while(iicps_readreg(BaseAddr,
				IICPS_TRANS_SIZE_OFFSET) !=
				(ByteCount - IICPS_FIFO_DEPTH));

			if ((RecvByteCount - IICPS_FIFO_DEPTH) >
				IICPS_MAX_TRANSFER_SIZE) {

				iicps_writereg(BaseAddr,
					IICPS_TRANS_SIZE_OFFSET,
					IICPS_MAX_TRANSFER_SIZE);
				ByteCount = IICPS_MAX_TRANSFER_SIZE +
						IICPS_FIFO_DEPTH;
			}else {
				iicps_writereg(BaseAddr,
					IICPS_TRANS_SIZE_OFFSET,
					RecvByteCount -
					IICPS_FIFO_DEPTH);
				UpdateTxSize = 0;
				ByteCount = RecvByteCount;
			}
		}

		status = iicps_readreg(BaseAddr, IICPS_ISR_OFFSET);
	}

	iicps_writereg(BaseAddr, IICPS_CR_OFFSET,
			iicps_readreg(BaseAddr,IICPS_CR_OFFSET) & ~IICPS_CR_HOLD);

	if (status & Intrs) {
		return PST_FAILURE;
	}

	return PST_SUCCESS;
}

int iicps_isbusy(u32 BaseAddress)
{
	u32 status;

	status = iicps_readreg(BaseAddress, IICPS_SR_OFFSET);
	if (status & 0x100) {
		return 1;
	}else {
		return 0;
	}
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

	if (iicps_readreg((BaseAddress), IICPS_TRANS_SIZE_OFFSET)) {
		return PST_FAILURE;
	}

	/*
	 * Assume Div_a is 0 and calculate (divisor_a+1) x (divisor_b+1).
	 */
	Temp = (InputClockHz) / (22 * FsclHz);

	/*
	 * If the answer is negative or 0, the Fscl input is out of range.
	 */
	if (0 == Temp) {
		return PST_FAILURE;
	}

	/*
	 * If frequency 400KHz is selected, 384.6KHz should be set.
	 * If frequency 100KHz is selected, 90KHz should be set.
	 * This is due to a hardware limitation.
	 */
	if(FsclHz > 384600) {
		FsclHz = 384600;
	}

	if((FsclHz <= 100000) && (FsclHz > 90000)) {
		FsclHz = 90000;
	}

	/*
	 * TempLimit helps in iterating over the consecutive value of Temp to
	 * find the closest clock rate achievable with divisors.
	 * Iterate over the next value only if fractional part is involved.
	 */
	TempLimit = ((InputClockHz) % (22 * FsclHz)) ?
							Temp + 1 : Temp;
	BestError = FsclHz;

	for ( ; Temp <= TempLimit ; Temp++)
	{
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

			ActualFscl = (InputClockHz) /
						(22 * (Div_a + 1) * (Div_b + 1));

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

		/*
		 * Used to capture the best divisors.
		 */
		if (LastError < BestError) {
			BestError = LastError;
			BestDivA = CalcDivA;
			BestDivB = CalcDivB;
		}
	}

	/*
	 * Read the control register and mask the Divisors.
	 */
	ControlReg = iicps_readreg(BaseAddress, IICPS_CR_OFFSET);
	ControlReg &= 0xFFFF00FF;
	ControlReg |= (BestDivA << 14) | (BestDivB << 8);
	iicps_writereg(BaseAddress, IICPS_CR_OFFSET, ControlReg);

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
