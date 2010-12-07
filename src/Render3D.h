/********************************************************
 /                   MANDELBULBER                        *
 /                                                       *
 / author: Krzysztof Marczak                             *
 / contact: buddhi1980@gmail.com                         *
 / licence: GNU GPL                                      *
 ********************************************************/

#ifndef RENDER3D_H_
#define RENDER3D_H_

#include "image.h"
#include "texture.hpp"
#include "fractparams.h"

struct sParam
{
  int start;
  int z;
  int progressive;
  int progressiveStart;
  sParamRender param;
  cImage *image;
  int *done;
  int *thread_done;
};

//global
extern guint64 N_counter;
extern guint64 Loop_counter;
extern guint64 DE_counter;
extern guint64 Pixel_counter;
extern int Missed_DE_counter;
extern double start_time;
extern bool isRendering;
extern bool isPostRendering;
extern int NR_THREADS;
extern bool noGUI;

extern cImage mainImage;

double
real_clock(void);
void MainRender(void);
void *MainThread(void *ptr);
void Render(sParamRender param, cImage *image, GtkWidget *outputDarea);
void InitMainParameters(sParamRender *fractParam, sParamSpecial *fractSpecial);
void InitMainImage(cImage *image, int width, int height, double previewScale, GtkWidget *drawingArea);
bool LoadTextures(sParamRender *params);
void ThumbnailRender(char *settingsFile, cImage *miniImage, int mode);

#endif /* RENDER3D_H_ */
