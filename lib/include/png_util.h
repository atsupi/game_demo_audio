/******************************************************
 *    Filename:     png_util.h
 *     Purpose:     png management utility
 *  Created on: 	2016/01/12
 * Modified on:		2021/02/01
 *      Author: 	atsupi.com
 *     Version:		1.10
 ******************************************************/

#ifndef PNG_UTIL_H_
#define PNG_UTIL_H_

#include "png.h"
#include "bitmap.h"

extern void saveBitmap2PngFile(Bitmap *bmp, char *fn);
extern void loadPngFile2Bitmap(Bitmap *bmp, char *fn);

#endif /* PNG_UTIL_H_ */
