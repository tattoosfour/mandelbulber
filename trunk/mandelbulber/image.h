/*
 * image.h
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof marczak
 */



#ifndef IMAGE_H_
#define IMAGE_H_

#include <gtk-2.0/gtk/gtk.h>
#include "texture.hpp"
#include "cimage.hpp"

//struct sRGB8
//{
//	unsigned char R;
//	unsigned char G;
//	unsigned char B;
//};

struct sSSAOparams
{
	int threadNo;
	cImage *image;
	double persp;
	int quality;
	int done;
	bool fishEye;
};


extern sRGB *buddhabrotImg;
extern bool isBuddhabrot;
extern double buddhabrotAutoBright;

extern guint64 histogram[256];
extern unsigned int histogram2[1000];

void NowaPaleta(sRGB *p, double nasycenie);
//void PutPixel(int x, int y, int R, int G, int B);
//void PutPixelAlfa(int x, int y, int R, int G, int B, int alfa, double skip_f);
void Bitmap32to8(sComplexImage *cImage, guchar *bitmapa8, guchar *bitmapa8_big);
void DrawHistogram(void);
void DrawHistogram2(void);
//void ShadedBackground(bool losuj, sRGB col1, sRGB col2);
//void BitmapBackground(guchar *tlo, int width, int height);
void PostRendering_SSAO(cImage *image, double persp, int quality);
void PostRendering_DOF(cImage *image, double deep, double neutral, double persp);
void DrawPalette(sRGB *palette);
void StoreImage8(sComplexImage *image, sRGB8 *image8);
void MakeStereoImage(sRGB8 *left, sRGB8 *right, guchar *stereoImage);
void StereoPreview(guchar *stereoImage);
void ScaleImage(guchar *imageBig, guchar *imageSmall);

#endif /* IMAGE_H_ */
