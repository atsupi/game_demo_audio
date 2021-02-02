/******************************************************
 *    Filename:     bitmap.h
 *     Purpose:     bitmap management utility
 *  Created on: 	2016/01/11
 * Modified on:		2021/01/19
 *      Author: 	atsupi.com
 *     Version:		1.10
 ******************************************************/

#ifndef BITMAP_H_
#define BITMAP_H_

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef int            i32;

typedef struct tagBitmapFileHeader {
    u16 bfType;
    u32 bfSize;
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
} BitmapFileHeader;

typedef struct tagBitmapInfoHeader {
    u32  biSize;
    i32  biWidth;
    i32  biHeight;
    u16  biPlanes;
    u16  biBitCount;
    u32  biCompression;
    u32  biSizeImage;
    u32  biXPelsPerMeter;
    u32  biYPelsPerMeter;
    u32  biClrUsed;
    u32  biClrImportant;
} BitmapInfoHeader;

// u32 data(x, y) = 0 << 24 | R << 16 | G << 8 | B;
typedef struct tagBitmap {
	BitmapFileHeader bfh;
	BitmapInfoHeader bih;
	u32 *data;
} Bitmap;

#define BI_RGB					0

#define RGBDATA(a, r, g, b)		((r << 16)|(g << 8)|(b))

#define FONTSIZEX				16
#define FONTSIZEY				16

int loadBitmapFile(char *filename, Bitmap *bmp);
int saveBitmapFile(Bitmap *bmp, char *filename);
int setBitmapPixel(Bitmap *bmp, u32 x, u32 y, u8 r, u8 g, u8 b);
int putBitmapRectagle(Bitmap *bmp, u32 x1, u32 y1, u32 x2, u32 y2, u8 r, u8 g, u8 b);
int fillBitmapRectagle(Bitmap *bmp, u32 x1, u32 y1, u32 x2, u32 y2, u8 r, u8 g, u8 b);
int putBitmapLine(Bitmap *bmp, u32 x1, u32 y1, u32 x2, u32 y2, u8 r, u8 g, u8 b);
int putBitmapText(Bitmap *bmp, Bitmap *font, u32 x, u32 y, u8 ch);
int drawBitmapString(Bitmap *bmp, Bitmap *font, u32 x, u32 y, char *str, int len);
int copyBitmapRect(Bitmap *dst, Bitmap *src, u32 dstx, u32 dsty, u32 srcx, u32 srcy, int sizex, int sizey);

#endif /* BITMAP_H_ */
