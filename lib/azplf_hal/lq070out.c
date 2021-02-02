/******************************************************
 *    Filename:     lq070out.c
 *     Purpose:     LQ070 LCD display driver
 *  Created on: 	2021/01/20
 * Modified on:		2021/01/26
 *      Author: 	atsupi.com
 *     Version:		1.20
 ******************************************************/

//#define DEBUG

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "azplf_bsp.h"
#include "lq070out.h"

static u32 page_size;

void lq070out_init(LQ070outInstance *inst, u32 baseAddr)
{
	int fd;
	u32 result;

	inst->baseAddress = baseAddr;
	page_size = sysconf(_SC_PAGESIZE);

	printf("In lq070out_init()\n");
	page_size = sysconf(_SC_PAGESIZE);
	printf("File page size=0x%08x (%dKB)\n", page_size, page_size>>10);

	fd = open("/dev/mem", O_RDWR);
	result = (u32)mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, baseAddr);
	printf("Mapping I/O: 0x%08x to vmem: 0x%08x\n", baseAddr, result);
	close(fd);

	if (result == 0 || result == 0xFFFFFFFF)
	{
		printf("Error: Mapping failure\n");
		return;
	}

	inst->virtAddress = result;
}

void lq070out_deinit(LQ070outInstance *inst)
{
	if (inst->virtAddress)
		munmap((void *)inst->virtAddress, page_size);
}

void lq070out_write_reg(u32 adr, u32 offset, u32 value)
{
	if (!adr)
	{
		printf("lq070out_write_reg baseaddress error!\n");
		return;
	}
	*(volatile unsigned int *)(adr + offset) = value;
}

void lq070out_setmode(LQ070outInstance *inst, int mode)
{
	lq070out_write_reg(inst->virtAddress, LQ070OUT_MODE_OFFSET, (u32)mode);
}
