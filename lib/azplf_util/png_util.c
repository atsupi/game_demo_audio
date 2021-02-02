/******************************************************
 *    Filename:     png_util.c
 *     Purpose:     png management utility
 *  Created on: 	2016/01/12
 * Modified on:		2021/02/01
 *      Author: 	atsupi.com
 *     Version:		1.10
 ******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "png_util.h"

//#define _DEBUG

#ifdef _DEBUG
static void Dump8(png_bytep buf)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		printf(" %02X", buf[i]);
	}
	printf("\n");
}
#endif

static void flushfunc(png_structp png_ptr)
{
	FILE *fp = (FILE *)png_get_io_ptr(png_ptr);
}

static void writefunc(png_structp png_ptr, png_bytep buf, png_size_t size)
{
#ifdef _DEBUG
	printf("INFO: buf=0x%x, size=%d\n", (u32)buf, size);
	if (size == 8) Dump8(buf);
#endif
	FILE *fp = (FILE *)png_get_io_ptr(png_ptr);
	fwrite(buf, size, 1, fp);
}

void saveBitmap2PngFile(Bitmap *bmp, char *fn)
{
	png_infop info_ptr;
	png_structp png_ptr;
	int i, j;

#ifdef _DEBUG
	printf("INFO: saveBitmap2PngFile start\n");
#endif
	FILE *fp;
	fp = fopen(fn, "wb");
#ifdef _DEBUG
	printf("INFO: file[0x%0x] opened.\n", (u32)fp);
#endif
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);
	u32 width = bmp->bih.biWidth;
	u32 height = bmp->bih.biHeight;
	u32 bpp = bmp->bih.biBitCount;
	u32 widthBytes = width * bpp / 8;
	u32 sizeImage = widthBytes * height;

#ifdef _DEBUG
	printf(" width=%u, height=%u, bpp=%u\n", width, height, bpp);
#endif
	png_bytepp image_rows;
	image_rows = (png_bytepp)calloc(height, sizeof(png_bytep));
	u8 b, g, r, a;

	for (i = 0; i < height; i++) {
		u32 row = i;
		png_bytep dst = image_rows[i] = (png_bytep)calloc(1, widthBytes);
		png_bytep src = (png_bytep)bmp->data + row * width * 4; // internal bitmap is 32bpp
		for (j = 0; j < width; j++) {
			b = *src++;
			g = *src++;
			r = *src++;
			a = *src++;
			*dst++ = r;
			*dst++ = g;
			*dst++ = b;
			if (bpp == 32)
				*dst++ = 0xFF;
		}
	}

#ifdef _DEBUG
	printf("INFO: saving file.\n");
#endif
	png_set_write_fn(png_ptr, (png_voidp)fp, (png_rw_ptr)writefunc, (png_flush_ptr)flushfunc);
	int color_type = (bpp == 24)? PNG_COLOR_TYPE_RGB: PNG_COLOR_TYPE_RGB_ALPHA;
#ifdef _DEBUG
	printf("INFO: color_type=%d\n", color_type);
#endif
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, color_type, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

#ifdef _DEBUG
	printf("INFO: saving info.\n");
#endif
	png_write_info(png_ptr, info_ptr);
#ifdef _DEBUG
	printf("INFO: saving image.\n");
#endif
	png_write_image(png_ptr, image_rows);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	for (i = 0; i < height; i++) free(image_rows[i]);
	free(image_rows);

	fclose(fp);
#ifdef _DEBUG
	printf("INFO: Finished.\n");
#endif
}

static void readfunc(png_structp png_ptr, png_bytep buf, png_size_t size)
{
	FILE *fp = (FILE *)png_get_io_ptr(png_ptr);
	fread(buf, size, 1, fp);
}

void loadPngFile2Bitmap(Bitmap *bmp, char *fn)
{
	int i, j;
	FILE *fp;
	u8 signature[8];
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytepp image_rows;
	u32 width;
	u32 height;
	u32 bpp;
	int color_type;
	u8 r, g, b, a;

	printf("INFO: loadPngFile2Bitmap start\n");
	fp = fopen(fn, "rb");
	printf("INFO: file[0x%0x] opened.\n", (u32)fp);
	fread(signature, sizeof(signature), 1, fp);
	if (!png_check_sig(signature, sizeof(signature))) goto FINISH;
	fseek(fp, 0, SEEK_SET);
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);

	printf("INFO: loading file.\n");
	png_set_read_fn(png_ptr, (png_voidp)fp, (png_rw_ptr)readfunc);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bpp, &color_type, NULL, NULL, NULL);
	printf("INFO: width=%u, height=%u, bpp=%u, color_type=%d\n", width, height, bpp, color_type);
	image_rows = (png_bytepp)calloc(height, sizeof(png_bytep));
	for (i = 0; i < height; i++) {
		image_rows[i] = (png_bytep)calloc(1, png_get_rowbytes(png_ptr, info_ptr));
	}

	printf("INFO: loading image...");
	png_read_image(png_ptr, image_rows);
	printf("done.\n");
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	//ToDo: fill bitmap headers for saving data to .bmp file.
	bmp->bfh.bfType          = 0x4D42; // BM
	bmp->bfh.bfOffBits       = 54;
	bmp->bfh.bfSize          = width * height * 4 + bmp->bfh.bfOffBits;
	bmp->bfh.bfReserved1     = 0;
	bmp->bfh.bfReserved2     = 0;

	bmp->bih.biSize          = sizeof(bmp->bih);
	bmp->bih.biWidth         = width;
	bmp->bih.biHeight        = height;
	bmp->bih.biSizeImage     = width * height * 4;
	bmp->bih.biBitCount      = 32;
	bmp->bih.biPlanes        = 1;
	bmp->bih.biCompression   = BI_RGB;
	bmp->bih.biXPelsPerMeter = 0x2E20;
	bmp->bih.biYPelsPerMeter = 0x2E20;
	bmp->bih.biClrUsed       = 0;
	bmp->bih.biClrImportant  = 0;

	printf("INFO: copy image data\n");
	bmp->data = (u32 *)calloc(width * height, sizeof(u32));
	for (i = 0; i < height; i++) {
		png_bytep dst = (png_bytep)bmp->data + i * width * 4; // internal bitmap is 32bpp
		png_bytep src = image_rows[i];
		for (j = 0; j < width; j++) {
			r = *src++;
			g = *src++;
			b = *src++;
			*dst++ = b;
			*dst++ = g;
			*dst++ = r;
			if (bpp == 32)
				*dst++ = *src++;
			else
				*dst++ = 0xFF;
		}
	}

	for (i = 0; i < height; i++) free(image_rows[i]);
	free(image_rows);

FINISH:
	printf("INFO: Finished.\n");
	fclose(fp);
}

