/********************************************************
 /                   MANDELBULBER                        *
 /                                                       *
 / author: Krzysztof Marczak                             *
 / contact: buddhi1980@gmail.com                         *
 / licence: GNU GPL                                      *
 ********************************************************/

#ifndef RENDER3D_H_
#define RENDER3D_H_

#include "fractal.h"
#include <gtk-2.0/gtk/gtk.h>
#include "image.h"
#include "cimage.hpp"
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

extern cImage *mainImage;

double
real_clock(void);
void MainRender(void);
void *MainThread(void *ptr);
void Render(sParamRender param, cImage *image);
void InitMainParameters(sParamRender *fractParam, sParamSpecial *fractSpecial);
void InitMainImage(cImage *image, int width, int height, double previewScale, GtkWidget *drawingArea);

#endif /* RENDER3D_H_ */
