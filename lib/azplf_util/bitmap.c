/******************************************************
 *    Filename:     bitmap.c
 *     Purpose:     bitmap management utility
 *  Created on: 	2016/01/11
 * Modified on:		2021/01/23
 *      Author: 	atsupi.com
 *     Version:		1.20
 ******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "azplf_bsp.h"
#include "bitmap.h"

//#define _DEBUG

int loadBitmapFile(char *filename, Bitmap *bmp)
{
	FILE *fp;
	u32 *buffer;
	int bytePerPixel;
	int dataSize;
	int x, y;
	int width, height;
	u32 *linePtr;
	u8 *ptr;

	if (bmp == NULL)
	{
		printf("Error: Invalid pointer to Bitmap class\n");
		return -1;
	}
	memset((void *)bmp, (int)0, sizeof(Bitmap));
	fp = fopen(filename, "rb");
	if (fp == NULL || fp == (void *)0xFFFFFFFF)
	{
		printf("Error: Cannot open file (%d)", ferror(fp));
		return ferror(fp);
	}
	printf("Opening file[%s]: handle is 0x%x\n", filename, (u32)fp);
	fread(&bmp->bfh.bfType, sizeof(bmp->bfh.bfType), 1, fp);
	fread(&bmp->bfh.bfSize, sizeof(bmp->bfh.bfSize), 1, fp);
	fread(&bmp->bfh.bfReserved1, sizeof(bmp->bfh.bfReserved1), 1, fp);
	fread(&bmp->bfh.bfReserved2, sizeof(bmp->bfh.bfReserved2), 1, fp);
	fread(&bmp->bfh.bfOffBits, sizeof(bmp->bfh.bfOffBits), 1, fp);

	fread(&bmp->bih.biSize, sizeof(bmp->bih.biSize), 1, fp);
	fread(&bmp->bih.biWidth, sizeof(bmp->bih.biWidth), 1, fp);
	fread(&bmp->bih.biHeight, sizeof(bmp->bih.biHeight), 1, fp);
	fread(&bmp->bih.biPlanes, sizeof(bmp->bih.biPlanes), 1, fp);
	fread(&bmp->bih.biBitCount, sizeof(bmp->bih.biBitCount), 1, fp);
	fread(&bmp->bih.biCompression, sizeof(bmp->bih.biCompression), 1, fp);
	fread(&bmp->bih.biSizeImage, sizeof(bmp->bih.biSizeImage), 1, fp);
	fread(&bmp->bih.biXPelsPerMeter, sizeof(bmp->bih.biXPelsPerMeter), 1, fp);
	fread(&bmp->bih.biYPelsPerMeter, sizeof(bmp->bih.biYPelsPerMeter), 1, fp);
	fread(&bmp->bih.biClrUsed, sizeof(bmp->bih.biClrUsed), 1, fp);
	fread(&bmp->bih.biClrImportant, sizeof(bmp->bih.biClrImportant), 1, fp);

	width  = bmp->bih.biWidth;
	height = bmp->bih.biHeight;
	bytePerPixel = bmp->bih.biBitCount / 8;
	dataSize = width * height;
	printf("Width=%d, Height=%d, bitCount=%d, dataSize=%d\n", width, height, bmp->bih.biBitCount, dataSize);
	// internal bitmap is 32bpp
	buffer = (u32 *)calloc(dataSize, sizeof(u32));
	linePtr = &buffer[(height - 1) * width];
	for (y = 0; y < height; y++)
	{
		ptr = (u8 *)(linePtr);
		for (x = 0; x < width; x++)
		{
			fread(ptr, 1, bytePerPixel, fp);
			ptr += 3;
			*ptr++ = 0;		// 32bit padding
		}
		// u32 data(x, y) = 0 << 24 | R << 16 | G << 8 | B;
		linePtr -= width;
	}
	bmp->data = buffer;
	fclose(fp);
	return 0;
}

int saveBitmapFile(Bitmap *bmp, char *filename)
{
	FILE *fp;
	int bytePerPixel;
	int dataSize;
	int x, y;
	int width, height;
	u32 *linePtr;

	if (bmp == NULL)
	{
		printf("Error: invalid data header\n");
		return -1;
	}
	width = bmp->bih.biWidth;
	height = bmp->bih.biHeight;
	if ( width < 1 || height < 1)
	{
		printf("Error: invalid width/height\n");
		return -1;
	}

	if (bmp->data == NULL)
	{
		printf("Error: invalid data pointer\n");
		return -1;
	}

	fp = fopen(filename, "wb");
	printf("Opening file[%s]: handle is 0x%x\n", filename, (u32)fp);
	if (fp)
	{
		printf("Writing bitmap file header: size(%d)\n", sizeof(bmp->bfh));
		printf("Writing bitmap info header: size(%d)\n", sizeof(bmp->bih));

		fwrite(&bmp->bfh.bfType, sizeof(bmp->bfh.bfType), 1, fp);
		fwrite(&bmp->bfh.bfSize, sizeof(bmp->bfh.bfSize), 1, fp);
		fwrite(&bmp->bfh.bfReserved1, sizeof(bmp->bfh.bfReserved1), 1, fp);
		fwrite(&bmp->bfh.bfReserved2, sizeof(bmp->bfh.bfReserved2), 1, fp);
		fwrite(&bmp->bfh.bfOffBits, sizeof(bmp->bfh.bfOffBits), 1, fp);

		fwrite(&bmp->bih, sizeof(bmp->bih), 1, fp);

		bytePerPixel = bmp->bih.biBitCount / 8;
		dataSize = width * height * bytePerPixel;
		printf("Writing bitmap data: size(%d)\n", dataSize);
		if (dataSize != bmp->bih.biSizeImage)
		{
			printf("Warning: dataSize(%d) is not same as the original size(%d)\n", dataSize, bmp->bih.biSizeImage);
		}
		for (y = 0; y < height; y++)
		{
			linePtr = &bmp->data[(height - 1 - y) * width];
#if defined (_DEBUG)
			printf("  data[%d]: ", y);
			printf("0x%08x\n", *linePtr);
#endif
			for (x = 0; x < width; x++)
			{
				fwrite(&linePtr[x], 1, bytePerPixel, fp);
			}
		}
		fclose(fp);
		printf("saveBitmapFile: Success.\n");
		return 0;
	}
	return -1;
}

int setBitmapPixel(Bitmap *bmp, u32 x, u32 y, u8 r, u8 g, u8 b)
{
	if (bmp == NULL || bmp->data == NULL)
	{
		printf("Error: invalid data pointer\n");
		return -1;
	}
	if (x >= bmp->bih.biWidth || y >= bmp->bih.biHeight)
	{
		printf("Error: invalid point [%d,%d]\n", x, y);
		return -1;
	}
	bmp->data[y * bmp->bih.biWidth + x] = RGBDATA(0, r, g, b);
	return 0;
}

#define horizontalFill(ptr, pix, size)				{ u32 *tmp = ptr; int i; for (i = 0; i < (size); i++) *(ptr)++ = (pix); ptr = tmp; }
#define verticalFill(ptr, pix, size, stride)		{ u32 *tmp = ptr; int i; for (i = 0; i < (size); i++) { *(ptr) = (pix); ptr += stride; } ptr = tmp; }

int putBitmapRectagle(Bitmap *bmp, u32 x1, u32 y1, u32 x2, u32 y2, u8 r, u8 g, u8 b)
{
	u32 pix = RGBDATA(0, r, g, b);
	int y;
	u32 *ptr;
	int width;

	if (bmp == NULL || bmp->data == NULL)
	{
		printf("Error: invalid data pointer\n");
		return -1;
	}
	if (x1 >= bmp->bih.biWidth || y1 >= bmp->bih.biHeight || x2 >= bmp->bih.biWidth || y2 >= bmp->bih.biHeight)
	{
		printf("Error: invalid point [%d,%d]-[%d,%d]\n", x1, y1, x2, y2);
		return -1;
	}
	if (x1 > x2) swap(x1, x2);
	if (y1 > y2) swap(y1, y2);

	width = bmp->bih.biWidth;
	ptr = &bmp->data[y1 * width + x1];
	horizontalFill(ptr, pix, x2 - x1 + 1);
	for (y = 0; y < (y2 - y1 - 1); y++)
	{
		ptr += width;
		ptr[0] = pix;
		ptr[x2 - x1] = pix;
	}
	ptr = &bmp->data[y2 * width + x1];
	horizontalFill(ptr, pix, x2 - x1 + 1);

	return 0;
}

int fillBitmapRectagle(Bitmap *bmp, u32 x1, u32 y1, u32 x2, u32 y2, u8 r, u8 g, u8 b)
{
	u32 pix = RGBDATA(0, r, g, b);
	int y;
	u32 *ptr;
	int width;

	if (bmp == NULL || bmp->data == NULL)
	{
		printf("Error: invalid data pointer\n");
		return -1;
	}
	if (x1 >= bmp->bih.biWidth || y1 >= bmp->bih.biHeight || x2 >= bmp->bih.biWidth || y2 >= bmp->bih.biHeight)
	{
		printf("Error: invalid point [%d,%d]-[%d,%d]\n", x1, y1, x2, y2);
		return -1;
	}
	if (x1 > x2) swap(x1, x2);
	if (y1 > y2) swap(y1, y2);

	width = bmp->bih.biWidth;
	ptr = &bmp->data[y1 * width + x1];
	for (y = 0; y < (y2 - y1 + 1); y++)
	{
		horizontalFill(ptr, pix, x2 - x1 + 1);
		ptr += width;
	}

	return 0;
}

int putBitmapLine(Bitmap *bmp, u32 x1, u32 y1, u32 x2, u32 y2, u8 r, u8 g, u8 b)
{
	if (bmp == NULL || bmp->data == NULL)
	{
		printf("Error: Invalid Source data pointer.\n");
		return -1;
	}

	if (x1 > x2) swap(x1, x2);
	if (y1 > y2) swap(y1, y2);

	u32 *ptr = &bmp->data[y1 * bmp->bih.biWidth + x1];
	u32 pix = RGBDATA(0, r, g, b);
	if (x1 == x2)
	{
		verticalFill(ptr, pix, y2 - y1 + 1, bmp->bih.biWidth);
	}
	else if (y1 == y2)
	{
		horizontalFill(ptr, pix, x2 - x1 + 1);
	}
	else
	{
		//ToDo:
		printf("Warning!!!!!: draw line function is not implemented yet.\n");
	}

	return 0;
}

int copyBitmapRect(Bitmap *dst, Bitmap *src, u32 dstx, u32 dsty, u32 srcx, u32 srcy, int sizex, int sizey)
{
	int x, y;
	u32 *psrc, *pdst;
	if (src == NULL || src->data == NULL)
	{
		printf("Error: Invalid Source data pointer.\n");
		return -1;
	}
	if (dst == NULL || dst->data == NULL)
	{
		printf("Error: Invalid Destination data pointer.\n");
		return -1;
	}
	if (srcx + sizex >= src->bih.biWidth || srcy + sizey >= src->bih.biHeight)
	{
		printf("Error: Source range overflow.\n");
		return -1;
	}
	if (dstx + sizex >= dst->bih.biWidth || dsty + sizey >= dst->bih.biHeight)
	{
		printf("Error: Destination range overflow.\n");
		return -1;
	}
	psrc = &src->data[srcy * src->bih.biWidth + srcx];
	pdst = &dst->data[dsty * dst->bih.biWidth + dstx];
	for (y = 0; y < sizey; y++)
	{
		for (x = 0; x < sizex; x++)
		{
			*pdst++ = *psrc++;
		}
		// line-feed
		psrc += src->bih.biWidth - sizex;
		pdst += dst->bih.biWidth - sizex;
	}
	return 0;
}

int putBitmapText(Bitmap *bmp, Bitmap *font, u32 x, u32 y, u8 ch)
{
	int srcx, srcy;
	int result;

	if (bmp == NULL || bmp->data == NULL)
	{
		printf("Error: Invalid Source data pointer.\n");
		return -1;
	}
	if (font == NULL || font->data == NULL)
	{
		printf("Error: Invalid Destination data pointer.\n");
		return -1;
	}

	srcx = (ch % 16) * FONTSIZEX;
	srcy = (ch / 16) * FONTSIZEY;
	result = copyBitmapRect(bmp, font, x, y, srcx, srcy, FONTSIZEX, FONTSIZEY);

	return result;
}

int drawBitmapString(Bitmap *bmp, Bitmap *font, u32 x, u32 y, char *str, int len)
{
	int i;
	int result;
	u8 ch;

	for (i = 0; i < len; i++)
	{
		ch = (u8)(*str++);
		result = putBitmapText(bmp, font, x, y, ch - 32); // Font bitmap does not include the first 32 characters.
		x += FONTSIZEX;

		if (result)
			return (result);
	}
	return 0;
}
