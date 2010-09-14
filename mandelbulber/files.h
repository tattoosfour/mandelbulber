/*
 * files.h
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof marczak
 */

#ifndef FILES_H_
#define FILES_H_

#include <png.h>
//#include <jpeglib.h>
#include <setjmp.h>

extern "C"
{
#include <jpeglib.h>
}
#include "cimage.hpp"

struct my_error_mgr
{
	struct jpeg_error_mgr pub; /* "public" fields */
	jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

struct sImageRGBA16
{
	unsigned short R;
	unsigned short G;
	unsigned short B;
	unsigned short A;
};

extern char logfileName[1000];

int IndexFilename(char* fullname, char* filename, char* extension, int number);
//METHODDEF(void) my_error_exit(j_common_ptr cinfo);
int LoadJPEG(char *filename, JSAMPLE *image);
bool CheckJPEGsize(char *filename, int *width, int *height);
void SaveJPEG(char *filename, int quality, int width, int height, JSAMPLE *image);
void SavePNG(char *filename, int quality, int width, int height, png_byte *image);
void SavePNG16(char *filename, int quality, int width, int height, cImage *image);
void SavePNG16Alpha(char *filename, int quality, int width, int height, cImage *image);
bool FileIfExist(char *filename);
void WriteLog(const char *text);
void WriteLogDouble(const char *text, double value);

#endif /* FILES_H_ */
