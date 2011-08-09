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
	palette = new sRGB[256];
	progressiveFactor = 1;
}

cImage::~cImage()
{
	if(!lowMem)
		delete[] complexImage;
	delete[] image16;
	delete[] image8;
	delete[] alpha;
	delete[] zBuffer;
	delete[] colorIndexBuf16;
	delete[] gammaTable;
	delete[] palette;
	if (previewAllocated)
		delete[] preview;
}

void cImage::SetLowMem(bool low_mem) {
	if (lowMem != low_mem) {
		if (lowMem)
			delete[] complexImage;
		else
			complexImage = new sComplexImage[width * height];
		lowMem = low_mem;
	}
}


void cImage::AllocMem(void)
{
	if (width > 0 && height > 0)
	{
		if(!lowMem)
			complexImage = new sComplexImage[width * height];
		image16 = new sRGB16[width * height];
		image8 = new sRGB8[width * height];
		zBuffer = new float[width * height];
		alpha = new unsigned short[width * height];
		colorIndexBuf16 = new unsigned short[width * height];
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
		delete[] complexImage;
	delete[] image16;
	delete[] image8;
	delete[] alpha;
	delete[] zBuffer;
	delete[] colorIndexBuf16;
	width = w;
	height = h;
	AllocMem();
}

void cImage::ClearImage(void)
{
	memset(image16, 0, (unsigned long int)sizeof(sRGB16) * width * height);
	memset(image8, 0, (unsigned long int)sizeof(sRGB8) * width * height);
	memset(alpha, 0, (unsigned long int)sizeof(unsigned short) * width * height);
	memset(colorIndexBuf16, 0, (unsigned long int)sizeof(unsigned short) * width * height);

	if (!lowMem)
		memset(complexImage, 0, (unsigned long int)sizeof(sComplexImage) * width * height);

	for (long int i = 0; i < width * height; ++i)
		zBuffer[i] = 1e20;
}

sRGB16 cImage::CalculatePixel(sComplexImage &pixel, unsigned short &alpha_, float &zBuf, unsigned short colorIndex, double fogVisBack, double fogVisFront)
{
	double mLightR = ecol.mainLightColour.R / 65536.0;
	double mLightG = ecol.mainLightColour.G / 65536.0;
	double mLightB = ecol.mainLightColour.B / 65536.0;

	double R=0.0;
	double G=0.0;
	double B=0.0;

	int alpha2 = 65535;
	if (zBuf > 1e19) alpha2 = 0;

	sRGB color = { 256, 256, 256 };
	if (sw.coloringEnabled)
	{
		int color_number = (int) (colorIndex * adj.coloring_speed + 256 * adj.paletteOffset) % 65536;
		color = IndexToColour(color_number);
	}
	//double jasSuma1 = ((1.0 - adj.shading) + adj.shading * pixel.shadingBuf16 / 4096.0) * ((1.0 - adj.ambient) * pixel.shadowsBuf16 / 4096.0 * adj.directLight + adj.ambient);
	double jasSuma1 = ((1.0 - adj.shading) + adj.shading * pixel.shadingBuf16 / 4096.0) * (pixel.shadowsBuf16 / 4096.0 * adj.directLight) * (1.0 - adj.ambient) + adj.ambient;
	if (zBuf > 1e19) jasSuma1 = 0;

	double jasSuma2 = (adj.specular * pixel.specularBuf16 / 4096.0) * ((1.0 - adj.ambient) * pixel.shadowsBuf16 / 4096.0 * adj.directLight + adj.ambient);

	jasSuma1 *= adj.mainLightIntensity;
	jasSuma2 *= adj.mainLightIntensity;

	if(sw.raytracedReflections)
	{
		R = (double)pixel.backgroundBuf16.R + (adj.reflect * pixel.reflectBuf16.R / 256.0 + color.R / 256.0 * (jasSuma1 * mLightR
				+ adj.globalIlum * pixel.ambientBuf16.R / 4096.0 + pixel.auxLight.R / 4096.0) + jasSuma2 * mLightR + pixel.auxSpecular.R / 4096.0) * 65536.0;
		G = (double)pixel.backgroundBuf16.G + (adj.reflect * pixel.reflectBuf16.G / 256.0 + color.G / 256.0 * (jasSuma1 * mLightG
				+ adj.globalIlum * pixel.ambientBuf16.G / 4096.0 + pixel.auxLight.G / 4096.0) + jasSuma2 * mLightG + pixel.auxSpecular.G / 4096.0) * 65336.0;
		B = (double)pixel.backgroundBuf16.B + (adj.reflect * pixel.reflectBuf16.B / 256.0 + color.B / 256.0 * (jasSuma1 * mLightB
				+ adj.globalIlum * pixel.ambientBuf16.B / 4096.0 + pixel.auxLight.B / 4096.0) + jasSuma2 * mLightB + pixel.auxSpecular.B / 4096.0) * 65536.0;
	}
	else
	{
		R = (double)pixel.backgroundBuf16.R + (adj.reflect * pixel.reflectBuf16.R / 256.0 * pixel.ambientBuf16.R / 4096.0 + color.R / 256.0 * (jasSuma1 * mLightR
				+ adj.globalIlum * pixel.ambientBuf16.R / 4096.0 + pixel.auxLight.R / 4096.0) + jasSuma2 * mLightR + pixel.auxSpecular.R / 4096.0) * 65536.0;
		G = (double)pixel.backgroundBuf16.G + (adj.reflect * pixel.reflectBuf16.G / 256.0 * pixel.ambientBuf16.G / 4096.0 + color.G / 256.0 * (jasSuma1 * mLightG
				+ adj.globalIlum * pixel.ambientBuf16.G / 4096.0 + pixel.auxLight.G / 4096.0) + jasSuma2 * mLightG + pixel.auxSpecular.G / 4096.0) * 65336.0;
		B = (double)pixel.backgroundBuf16.B + (adj.reflect * pixel.reflectBuf16.B / 256.0 * pixel.ambientBuf16.B / 4096.0 + color.B / 256.0 * (jasSuma1 * mLightB
				+ adj.globalIlum * pixel.ambientBuf16.B / 4096.0 + pixel.auxLight.B / 4096.0) + jasSuma2 * mLightB + pixel.auxSpecular.B / 4096.0) * 65536.0;
	}

	sRGB col = { R, G, B };
	if (sw.fogEnabled)
	{
		col = PostRendering_Fog(zBuf, fogVisFront, fogVisBack + fogVisFront, ecol.fogColor, col);
	}
	R = col.R;
	G = col.G;
	B = col.B;

	//volumetric fog
	double fogDensity = pixel.fogDensity16 / 65535.0;
	double fogN = 1.0 - fogDensity;
	alpha2 += fogDensity;

	R = R * fogN + pixel.volumetricLight.R * fogDensity;
	G = G * fogN + pixel.volumetricLight.G * fogDensity;
	B = B * fogN + pixel.volumetricLight.B * fogDensity;

	//glow
	double glow = pixel.glowBuf16 * adj.glow_intensity / 512.0;
	double glowN = 1.0 - glow;
	if (glowN < 0.0) glowN = 0.0;

	double glowR = (ecol.glow_color1.R * glowN + ecol.glow_color2.R * glow);
	double glowG = (ecol.glow_color1.G * glowN + ecol.glow_color2.G * glow);
	double glowB = (ecol.glow_color1.B * glowN + ecol.glow_color2.B * glow);

	if(sw.volumetricLightEnabled)
	{
		R = R * glowN + glowR * glow + pixel.volumetricLight.R * 16.0 * adj.volumetricLightIntensity;
		G = G * glowN + glowG * glow + pixel.volumetricLight.G * 16.0 * adj.volumetricLightIntensity;
		B = B * glowN + glowB * glow + pixel.volumetricLight.B * 16.0 * adj.volumetricLightIntensity;
	}
	else
	{
		R = R * glowN + glowR * glow;
		G = G * glowN + glowG * glow;
		B = B * glowN + glowB * glow;
	}

	R *= adj.brightness;
	G *= adj.brightness;
	B *= adj.brightness;

	alpha2 += 65535.0 * glow;
	if (alpha2 > 65535) alpha2 = 65535;

	if (R > 65535) R = 65535;
	if (R < 0) R = 0;
	if (G > 65535) G = 65535;
	if (G < 0) G = 0;
	if (B > 65535) B = 65535;
	if (B < 0) B = 0;

	sRGB16 newPixel16;
	newPixel16.R = gammaTable[(unsigned int)R];
	newPixel16.G = gammaTable[(unsigned int)G];
	newPixel16.B = gammaTable[(unsigned int)B];
	alpha_ = alpha2;

	return newPixel16;
}

sRGB16 cImage::CalculateAmbientPixel(sRGB16 ambient16, unsigned short colorIndex, sRGB16 oldPixel16)
{
	sRGB color = { 256, 256, 256 };
	if (sw.coloringEnabled)
	{
		int color_number = (int) (colorIndex * adj.coloring_speed + 256 * adj.paletteOffset) % 65536;
		color = IndexToColour(color_number);
	}
	sRGB ambientAdd = {0,0,0};
	ambientAdd.R = adj.brightness * color.R / 256.0 * adj.globalIlum * ambient16.R / 4096.0 * 65536;
	ambientAdd.G = adj.brightness * color.G / 256.0 * adj.globalIlum * ambient16.G / 4096.0 * 65536;
	ambientAdd.B = adj.brightness * color.B / 256.0 * adj.globalIlum * ambient16.B / 4096.0 * 65536;
	int newR = oldPixel16.R + ambientAdd.R;
	int newG = oldPixel16.G + ambientAdd.G;
	int newB = oldPixel16.B + ambientAdd.B;
	if(newR > 65535) newR = 65535;
	if(newG > 65535) newG = 65535;
	if(newB > 65535) newB = 65535;

	sRGB16 newPixel = {newR, newG, newB};
	//sRGB16 newPixel = {color.R, color.G, color.B};
	return newPixel;
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

		double fog_visibility = pow(10, adj.fogVisibility / 10 - 2.0);
		double fog_visibility_front = pow(10, adj.fogVisibilityFront / 10 - 2.0) - 10.0;

		for (int y = 0; y < height; y += progressiveFactor)
		{
			for (int x = 0; x < width; x += progressiveFactor)
			{
				unsigned long int address = x + y * width;
				sComplexImage pixel = complexImage[address];
				unsigned short alpha2 = alpha[address];
				float zBuf = zBuffer[address];
				unsigned short colorIndex = colorIndexBuf16[address];

				sRGB16 newPixel16 = CalculatePixel(pixel, alpha2, zBuf, colorIndex, fog_visibility, fog_visibility_front);

				complexImage[address] = pixel;
				image16[address] = newPixel16;
				alpha[address] = alpha2;
				zBuffer[address] = zBuf;
			}
			for (int x = 0; x <= width - progressiveFactor; x += progressiveFactor)
			{
				sRGB16 pixel = image16[x + y * width];
				int alpha2 = alpha[x + y * width];
				for (int yy = 0; yy < progressiveFactor; yy++)
				{
					for (int xx = 0; xx < progressiveFactor; xx++)
					{
						if (xx == 0 && yy == 0) continue;
						image16[x + xx + (y + yy) * width] = pixel;
						alpha[x + xx + (y + yy) * width] = alpha2;
					}
				}
			}
		}
	}
}

void cImage::SetPalette(sRGB *newPalette)
{
	for (int i = 0; i < 256; i++)
	{
		palette[i] = newPalette[i];
	}
}

sRGB cImage::IndexToColour(int index)
{
	double R1, R2, G1, G2, B1, B2;
	double RK, GK, BK;
	sRGB colour = { 255, 255, 255 };
	int kol, delta;
	if (index < 0)
	{
		colour = palette[255];
	}
	else
	{
		index = index % (255 * 256);
		kol = index / 256;
		if (kol < 255)
		{
			R1 = palette[kol].R;
			G1 = palette[kol].G;
			B1 = palette[kol].B;
			R2 = palette[kol + 1].R;
			G2 = palette[kol + 1].G;
			B2 = palette[kol + 1].B;
			RK = (R2 - R1) / 256.0;
			GK = (G2 - G1) / 256.0;
			BK = (B2 - B1) / 256.0;
			delta = index % 256;
			colour.R = R1 + (RK * delta);
			colour.G = G1 + (GK * delta);
			colour.B = B1 + (BK * delta);
		}
	}
	return colour;
}

int cImage::GetUsedMB(void)
{
	long int mb = 0;
	if (lowMem)
	{
		long int zBufferSize = (long int)width * height * sizeof(float);
		long int alphaSize = (long int)width * height * sizeof(unsigned short);
		long int image16Size = (long int)width * height * sizeof(sRGB16);
		long int colorSize = (long int)width * height * sizeof(unsigned short);
		mb = (zBufferSize + alphaSize + image16Size + colorSize) / 1024 / 1024;
	}
	else
	{
		long int zBufferSize = (long int)width * height * sizeof(float);
		long int alphaSize = (long int)width * height * sizeof(unsigned short);
		long int image16Size = (long int)width * height * sizeof(sRGB16);
		long int complexSize = (long int)width * height * sizeof(sComplexImage);
		long int colorSize = (long int)width * height * sizeof(unsigned short);
		mb = (long int)(zBufferSize + alphaSize + image16Size + complexSize + colorSize) / 1024 / 1024;
	}
	return mb;
}

void cImage::SetImageParameters(sImageAdjustments adjustments, sEffectColours effectColours, sImageSwitches switches)
{
	adj = adjustments;
	ecol = effectColours;
	sw = switches;
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
		sComplexImage pixel = complexImage[x + y * width];
		float zBufferTemp = zBuffer[x + y * width];
		unsigned short colorTemp = colorIndexBuf16[x + y * width];

		for (int yy = 0; yy < pFactor; yy++)
		{
			for (int xx = 0; xx < pFactor; xx++)
			{
				if (xx == 0 && yy == 0) continue;
				complexImage[x + xx + (y + yy) * width] = pixel;
				zBuffer[x + xx + (y + yy) * width] = zBufferTemp;
				colorIndexBuf16[x + xx + (y + yy) * width] = colorTemp;
			}
		}
	}
}

sRGB PostRendering_Fog(double z, double min, double max, sRGB fog_color, sRGB color_before)
{
	sRGB color;

	int R = fog_color.R;
	int G = fog_color.G;
	int B = fog_color.B;

	double dist = max - min;

	double fog = (z - min) / dist;
	if (fog < 0.0) fog = 0.0;
	if (fog > 1.0) fog = 1.0;

	double a = fog;
	double aN = 1.0 - a;

	color.R = (color_before.R * aN + R * a);
	color.G = (color_before.G * aN + G * a);
	color.B = (color_before.B * aN + B * a);

	return color;
}
