/*
 * cimage.cpp
 *
 *  Created on: 2010-08-29
 *      Author: krzysztof
 */

#include <gtk-2.0/gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "cimage.hpp"


cImage::cImage(int w, int h, bool low_mem)
{
	width = w;
	height = h;
	previewAllocated = false;
	lowMem = low_mem;
	AllocMem();
	gammaTable = new int[65536];
	progressiveFactor = 1;
}

cImage::~cImage()
{
	if(!lowMem)
	delete[] imageFloat;
	delete[] image16;
	delete[] image8;
	delete[] alphaBuffer;
	delete[] opacityBuffer;
	delete[] colourBuffer;
	delete[] zBuffer;
	delete[] gammaTable;
	if (previewAllocated)
		delete[] preview;
}

void cImage::AllocMem(void)
{
	if (width > 0 && height > 0)
	{
		imageFloat =  new sRGBfloat[width * height];
		image16 = new sRGB16[width * height];
		image8 = new sRGB8[width * height];
		zBuffer = new float[width * height];
		alphaBuffer = new unsigned short[width * height];
		opacityBuffer = new unsigned short[width * height];
		colourBuffer = new sRGB8[width * height];
		ClearImage();
	}
	else
	{
		fprintf(stderr, "Error! Cannot allocate memory for image (wrong image size)\n");
	}

	if (previewAllocated)
		delete[] preview;
	previewAllocated = false;

	preview = 0;
}

void cImage::ChangeSize(int w, int h)
{
	if(!lowMem)
	delete[] imageFloat;
	delete[] image16;
	delete[] image8;
	delete[] alphaBuffer;
	delete[] opacityBuffer;
	delete[] colourBuffer;
	delete[] zBuffer;
	width = w;
	height = h;
	AllocMem();
}

void cImage::ClearImage(void)
{
	memset(imageFloat, 0, (unsigned long int)sizeof(sRGBfloat) * width * height);
	memset(image16, 0, (unsigned long int)sizeof(sRGB16) * width * height);
	memset(image8, 0, (unsigned long int)sizeof(sRGB8) * width * height);
	memset(alphaBuffer, 0, (unsigned long int)sizeof(unsigned short) * width * height);
	memset(opacityBuffer, 0, (unsigned long int)sizeof(unsigned short) * width * height);
	memset(colourBuffer, 0, (unsigned long int)sizeof(sRGB8) * width * height);

	for (long int i = 0; i < width * height; ++i)
		zBuffer[i] = 1e20;
}

sRGB16 cImage::CalculatePixel(sRGBfloat pixel)
{

	float R = pixel.R * adj.brightness;
	float G = pixel.G * adj.brightness;
	float B = pixel.B * adj.brightness;

	R = (R - 0.5) * adj.contrast + 0.5;
	G = (G - 0.5) * adj.contrast + 0.5;
	B = (B - 0.5) * adj.contrast + 0.5;

	if (R > 1.0) R = 1.0;
	if (R < 0.0) R = 0.0;
	if (G > 1.0) G = 1.0;
	if (G < 0.0) G = 0.0;
	if (B > 1.0) B = 1.0;
	if (B < 0.0) B = 0.0;

	sRGB16 newPixel16;

	newPixel16.R = R * 65535.0;
	newPixel16.G = G * 65535.0;
	newPixel16.B = B * 65535.0;
	newPixel16.R = gammaTable[newPixel16.R];
	newPixel16.G = gammaTable[newPixel16.G];
	newPixel16.B = gammaTable[newPixel16.B];

	return newPixel16;
}


void cImage::CalculateGammaTable(void)
{
	for (int i = 0; i < 65536; i++)
	{
		gammaTable[i] = pow(i / 65536.0, 1.0 / adj.imageGamma) * 65535;
	}
}

void cImage::CompileImage(void)
{
	if (!lowMem)
	{
		CalculateGammaTable();

		for (int y = 0; y < height; y += progressiveFactor)
		{
			for (int x = 0; x < width; x += progressiveFactor)
			{
				unsigned long int address = x + y * width;
				sRGBfloat pixel = imageFloat[address];
				sRGB16 newPixel16 = CalculatePixel(pixel);
				image16[address] = newPixel16;
			}
			for (int x = 0; x <= width - progressiveFactor; x += progressiveFactor)
			{
				sRGB16 pixel = image16[x + y * width];
				int alpha2 = alphaBuffer[x + y * width];
				for (int yy = 0; yy < progressiveFactor; yy++)
				{
					for (int xx = 0; xx < progressiveFactor; xx++)
					{
						if (xx == 0 && yy == 0) continue;
						image16[x + xx + (y + yy) * width] = pixel;
						alphaBuffer[x + xx + (y + yy) * width] = alpha2;
					}
				}
			}
		}
	}
}

int cImage::GetUsedMB(void)
{
	long int mb = 0;

	long int zBufferSize = (long int) width * height * sizeof(float);
	long int alphaSize = (long int) width * height * sizeof(unsigned short);
	long int imageFloatSize = (long int) width * height * sizeof(sRGBfloat);
	long int image16Size = (long int) width * height * sizeof(sRGB16);
	long int image8Size = (long int) width * height * sizeof(sRGB8);
	long int colorSize = (long int) width * height * sizeof(sRGB8);
	long int opacitySize = (long int) width * height * sizeof(unsigned short);
	mb = (long int) (zBufferSize + alphaSize + image16Size + image8Size + imageFloatSize + colorSize + opacitySize) / 1024 / 1024;

	return mb;
}

void cImage::SetImageParameters(sImageAdjustments adjustments)
{
	adj = adjustments;
}

unsigned char* cImage::ConvertTo8bit(void)
{
	for (long int i = 0; i < width * height; i++)
	{
		image8[i].R = image16[i].R / 256;
		image8[i].G = image16[i].G / 256;
		image8[i].B = image16[i].B / 256;
	}
	unsigned char* ptr = (unsigned char*) image8;
	return ptr;
}

sRGB8 cImage::Interpolation(float x, float y)
{
	sRGB8 colour = { 0, 0, 0 };
	if (x >= 0 && x < width - 1 && y >= 0 && y < height - 1)
	{
		int ix = x;
		int iy = y;
		int rx = (x - ix) * 256;
		int ry = (y - iy) * 256;
		sRGB8 k1 = image8[iy * width + ix];
		sRGB8 k2 = image8[iy * width + ix + 1];
		sRGB8 k3 = image8[(iy + 1) * width + ix];
		sRGB8 k4 = image8[(iy + 1) * width + ix + 1];
		colour.R = (k1.R * (255 - rx) * (255 - ry) + k2.R * (rx) * (255 - ry) + k3.R * (255 - rx) * ry + k4.R * (rx * ry)) / 65536;
		colour.G = (k1.G * (255 - rx) * (255 - ry) + k2.G * (rx) * (255 - ry) + k3.G * (255 - rx) * ry + k4.G * (rx * ry)) / 65536;
		colour.B = (k1.B * (255 - rx) * (255 - ry) + k2.B * (rx) * (255 - ry) + k3.B * (255 - rx) * ry + k4.B * (rx * ry)) / 65536;
	}
	return colour;
}

unsigned char* cImage::CreatePreview(double scale)
{
	int w = width * scale;
	int h = height * scale;
	if (previewAllocated) delete[] preview;
	preview = new sRGB8[w * h];
	memset(preview, 0, (unsigned long int)sizeof(sRGB8) * (w * h));
	previewAllocated = true;
	previewWidth = w;
	previewHeight = h;
	previewScale = scale;
	unsigned char* ptr = (unsigned char*) preview;
	return ptr;
}

void cImage::UpdatePreview(void)
{
	if (previewAllocated)
	{
		int w = previewWidth;
		int h = previewHeight;

		if (width == w && height == h)
		{
			memcpy(preview, image8, width * height * sizeof(sRGB8));
		}
		else
		{
			float scaleX = (float) width / w;
			float scaleY = (float) height / h;

			//number of pixels to sum
			int countX = (float) width / w + 1;
			int countY = (float) height / h + 1;
			int factor = countX * countY;

			float deltaX = scaleX / countX;
			float deltaY = scaleY / countY;

			for (int y = 0; y < h; y++)
			{
				for (int x = 0; x < w; x++)
				{
					int R = 0;
					int G = 0;
					int B = 0;
					for (int j = 0; j < countY; j++)
					{
						float yy = y * scaleY + j * deltaY;
						for (int i = 0; i < countX; i++)
						{
							float xx = x * scaleX + i * deltaX;
							if (xx > 0 && xx < width - 1 && yy > 0 && yy < height - 1)
							{
								sRGB8 oldPixel = Interpolation(xx, yy);
								R += oldPixel.R;
								G += oldPixel.G;
								B += oldPixel.B;
							}
						}//next i
					}//next j
					sRGB8 newpixel;
					newpixel.R = R / factor;
					newpixel.G = G / factor;
					newpixel.B = B / factor;
					preview[x + y * w] = newpixel;
				}//next x
			}//next y
		}
	}
	else
	{
		fprintf(stderr, "Error! Preview has not been prepared\n");
	}
}

unsigned char* cImage::GetPreviewPtr(void)
{
	unsigned char* ptr = 0;
	if (previewAllocated)
	{
		ptr = (unsigned char*) preview;
	} else abort();
	return ptr;
}

bool cImage::IsPreview(void)
{
	if (previewAllocated)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void cImage::RedrawInWidget(GtkWidget *dareaWidget)
{
	if (IsPreview())
	{
		gdk_draw_rgb_image(dareaWidget->window, dareaWidget->style->fg_gc[GTK_STATE_NORMAL], 0, 0, GetPreviewWidth(), GetPreviewHeight(), GDK_RGB_DITHER_MAX, GetPreviewPtr(),
				GetPreviewWidth() * 3);
	}
}

void cImage::Squares(int y, int pFactor)
{
	progressiveFactor = pFactor;
	for (int x = 0; x <= width - pFactor; x += pFactor)
	{
		sRGBfloat pixelTemp = imageFloat[x + y * width];
		float zBufferTemp = zBuffer[x + y * width];
		sRGB8 colourTemp = colourBuffer[x + y * width];
		unsigned short alphaTemp = alphaBuffer[x + y * width];

		for (int yy = 0; yy < pFactor; yy++)
		{
			for (int xx = 0; xx < pFactor; xx++)
			{
				if (xx == 0 && yy == 0) continue;
				imageFloat[x + xx + (y + yy) * width] = pixelTemp;
				zBuffer[x + xx + (y + yy) * width] = zBufferTemp;
				colourBuffer[x + xx + (y + yy) * width] = colourTemp;
				alphaBuffer[x + xx + (y + yy) * width] = alphaTemp;
			}
		}
	}
}


