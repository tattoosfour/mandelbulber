/*
 * image.h
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof marczak
 */


#ifndef IMAGE_H_
#define IMAGE_H_

#include "cimage.hpp"

struct sSSAOparams
{
	int threadNo;
	cImage *image;
	double persp;
	int quality;
	int done;
	bool fishEye;
	int progressive;
	bool quiet;
};


extern sRGB *buddhabrotImg;
extern bool isBuddhabrot;
extern double buddhabrotAutoBright;

extern guint64 histogram[256];
extern unsigned int histogram2[1000];

void NowaPaleta(sRGB *p, double nasycenie);
void DrawHistogram(void);
void DrawHistogram2(void);
void PostRendering_SSAO(cImage *image, double persp, int quality, bool fishEye, bool quiet);
void PostRendering_DOF(cImage *image, double deep, double neutral, double persp);
void DrawPalette(sRGB *palette);
void StoreImage8(sComplexImage *image, sRGB8 *image8);
void MakeStereoImage(cImage *left, cImage *right, guchar *stereoImage);
void StereoPreview(cImage *temoraryImage, guchar *stereoImage);


#endif /* IMAGE_H_ */
