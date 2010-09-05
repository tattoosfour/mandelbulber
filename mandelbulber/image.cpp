/********************************************************
 /                   MANDELBULBER                        *
 /                                                       *
 / author: Krzysztof Marczak                             *
 / contact: buddhi1980@gmail.com                         *
 / licence: GNU GPL                                      *
 ********************************************************/

/*
 * image.cpp
 *
 *  Created on: 2010-01-23
 *      Author: krzysztof
 */
#include <gtk-2.0/gtk/gtk.h>
#include "image.h"
#include "cimage.hpp"
#include "common_math.h"
#include "math.h"
#include "interface.h"
#include "algebra.hpp"
#include "shaders.h"
#include "fractal.h"

sRGB *buddhabrotImg;
bool isBuddhabrot = false;
double buddhabrotAutoBright = 1.0;

guint64 histogram[256];
unsigned int histogram2[1000];

//******************* Nowa paleta kolorow ****************
void NowaPaleta(sRGB *p, double nasycenie)
{
	int R, G, B, Y;

	for (int i = 0; i < 255; i++)
	{
		Y = (Random(255) - 128) / (1.0 + nasycenie);
		p[i].R = R = Y + 128 + (Random(255) - 128) * nasycenie;
		p[i].G = G = Y + 128 + (Random(255) - 128) * nasycenie;
		p[i].B = B = Y + 128 + (Random(255) - 128) * nasycenie;
		if (R < 0) p[i].R = 0;
		if (G < 0) p[i].G = 0;
		if (B < 0) p[i].B = 0;
		if (R > 255) p[i].R = 255;
		if (G > 255) p[i].G = 255;
		if (B > 255) p[i].B = 255;
	}
	p[255].R = 255;
	p[255].G = 255;
	p[255].B = 255;
}

//*********************** PutPixel ************************
//rysowanie pixela na 3x32-bitowej bitmapie
//we: 	x,y - współrzędne piksela
//		R,G,B - kolor (składowe 16-bitowe)
/*
 void PutPixel(int x, int y, int R, int G, int B)
 {
 if (x >= 0 && x < IMAGE_WIDTH && y >= 0 && y < IMAGE_HEIGHT)
 {
 guint64 adres = ((guint64) x + y * IMAGE_WIDTH) * 3;
 if (R > 65279) R = 65279;
 if (G > 65279) G = 65279;
 if (B > 65279) B = 65279;
 if (R < 0) R = 0;
 if (G < 0) G = 0;
 if (B < 0) B = 0;
 rgbbuf32[adres] = R;
 rgbbuf32[adres + 1] = G;
 rgbbuf32[adres + 2] = B;
 }
 }
 */

//*********************** PutPixelAlfa ********************
//rysowanie pixela na 3x32-bitowej bitmapie z przezroczystoscia
//we: 	x,y - współrzędne piksela
//		R,G,B - kolor (składowe 16-bitowe)
//		alfa - wsp. przenikania 0-65536
/*
 void PutPixelAlfa(int x, int y, int R, int G, int B, int alfa, double skip_f)
 {
 if (x >= 0 && x < IMAGE_WIDTH && y >= 0 && y < IMAGE_HEIGHT)
 {
 guint64 adres = ((guint64) x + y * IMAGE_WIDTH) * 3;
 if (R > 16777216) R = 16777216;
 if (G > 16777216) G = 16777216;
 if (B > 16777216) B = 16777216;
 if (alfa > 65279) alfa = 65279;
 if (R < 0) R = 0;
 if (G < 0) G = 0;
 if (B < 0) B = 0;
 if (alfa < 0) alfa = 0;
 double a = alfa / 65536.0;
 double aN = 1.0 - a;
 double aPow = pow(aN, skip_f);
 double aPowN = 1.0 - aPow;
 unsigned int oldR = rgbbuf32[adres];
 unsigned int oldG = rgbbuf32[adres + 1];
 unsigned int oldB = rgbbuf32[adres + 2];
 //rgbbuf32[adres] = ((unsigned int) oldR * alfaN + (unsigned int) R * alfa) / 65536;
 //rgbbuf32[adres + 1] = ((unsigned int) oldG * alfaN + (unsigned int) G * alfa) / 65536;
 //rgbbuf32[adres + 2] = ((unsigned int) oldB * alfaN + (unsigned int) B * alfa) / 65536;
 rgbbuf32[adres] = (oldR * aPow + R * aPowN);
 rgbbuf32[adres + 1] = (oldG * aPow + G * aPowN);
 rgbbuf32[adres + 2] = (oldB * aPow + B * aPowN);

 }
 }
 */

//*********************** Bitmap32to8 *********************
//konwersja bitmapy 3x32-bitowej na 24-bitową
//we:	bitmapa32 - 3x32-bitowa bitmapa
//wy:	bitmapa8 - 24-bitowa bitmapa
/*void Bitmap32to8(sComplexImage *cImage, guchar *bitmapa8, guchar *bitmapa8_big)
 {
 for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
 {
 bitmapa8_big[i * 3] = cImage[i].image.R / 256;
 bitmapa8_big[i * 3 + 1] = cImage[i].image.G / 256;
 bitmapa8_big[i * 3 + 2] = cImage[i].image.B / 256;
 }
 ScaleImage(bitmapa8_big, bitmapa8);
 }
 */

void ScaleImage(guchar *imageBig, guchar *imageSmall)
{
	int width = mainImage->GetWidth();
	int height = mainImage->GetHeight();

	if (!SCALE_ZOOM)
	{
		int factor = SCALE * SCALE;
		for (int y = 0; y < height / SCALE; y++)
		{
			for (int x = 0; x < width / SCALE; x++)
			{
				int rSuma = 0;
				int gSuma = 0;
				int bSuma = 0;
				guint64 adres2 = ((guint64) y * (width / SCALE) + x) * 3;
				for (int j = 0; j < SCALE; j++)
				{
					for (int i = 0; i < SCALE; i++)
					{
						guint64 adres = (((guint64) y * SCALE + j) * width + x * SCALE + i) * 3;
						rSuma += imageBig[adres];
						gSuma += imageBig[adres + 1];
						bSuma += imageBig[adres + 2];
					}
				}
				imageSmall[adres2] = rSuma / factor;
				imageSmall[adres2 + 1] = gSuma / factor;
				imageSmall[adres2 + 2] = bSuma / factor;
			}
		}
	}
	else
	{
		for (int y = 0; y < height * SCALE; y++)
		{
			for (int x = 0; x < width * SCALE; x++)
			{
				guint64 adres = (((guint64) y / SCALE) * width + x / SCALE) * 3;
				guint64 adres2 = ((guint64) y * (width * SCALE) + x) * 3;
				int R = imageBig[adres];
				int G = imageBig[adres + 1];
				int B = imageBig[adres + 2];
				imageSmall[adres2] = R;
				imageSmall[adres2 + 1] = G;
				imageSmall[adres2 + 2] = B;
			}
		}
	}
}

void PostRendering_DOF(cImage *image, double deep, double neutral, double persp)
{
	isPostRendering = true;

	int width = image->GetWidth();
	int height = image->GetHeight();

	sRGB16 *temp_image = new sRGB16[width * height];
	unsigned short *temp_alpha = new unsigned short[width * height];
	sSortZ *temp_sort = new sSortZ[width * height];
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int ptr = x+y*width;
			temp_image[ptr] = image->GetPixelImage(x,y);
			temp_alpha[ptr] = image->GetPixelAlpha(x,y);
			temp_sort[ptr].z = image->GetPixelZBuffer(x,y);
			temp_sort[ptr].i = ptr;
		}
	}

	if (!noGUI)
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), "Rendering Depth Of Field effect. Sorting zBuffer");
		while (gtk_events_pending())
			gtk_main_iteration();
	}

	QuickSortZBuffer(temp_sort, 1, height * width - 1);

	if (!noGUI)
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), "Rendering Depth Of Field effect. Done 0%");
		while (gtk_events_pending())
			gtk_main_iteration();
	}

	double last_time = clock() / CLOCKS_PER_SEC;
	double min = -1.0 / persp;

	for (int i = 0; i < height * width; i++)
	{
		int ii = temp_sort[height * width - i - 1].i;
		int x = ii % width;
		int y = ii / width;
		double z = image->GetPixelZBuffer(x,y);
		double blur = fabs((double) z - neutral) / (z - min) * deep;
		if (blur > 100) blur = 100.0;
		int size = blur;
		sRGB16 center = temp_image[x + y * width];
		unsigned short center_alpha = temp_alpha[x + y * width];
		double factor = blur * blur * sqrt(blur);

		for (int yy = y - size; yy <= y + size; yy++)
		{
			for (int xx = x - size; xx <= x + size; xx++)
			{
				if (xx >= 0 && xx < width && yy >= 0 && yy < height)
				{
					int dx = xx - x;
					int dy = yy - y;
					double r = sqrt(dx * dx + dy * dy);
					double op = (blur - r) / factor;
					if (op > 1.0) op = 1.0;
					if (op < 0.0) op = 0.0;
					if (op > 0.0)
					{
						double opN = 1.0 - op;
						sRGB16 old = image->GetPixelImage(xx,yy);
						unsigned old_alpha = image->GetPixelAlpha(xx,yy);
						sRGB16 pixel;
						pixel.R = old.R * opN + center.R * op;
						pixel.G = old.G * opN + center.G * op;
						pixel.B = old.B * opN + center.B * op;
						unsigned short alpha = old_alpha * opN + center_alpha * op;
						image->PutPixelImage(xx,yy,pixel);
						image->PutPixelAlpha(xx,yy,alpha);
					}
				}
			}
		}
		double time = clock() / CLOCKS_PER_SEC;
		if (time - last_time > 5.0 && !noGUI && image->IsPreview())
		{
			char progressText[1000];
			last_time = clock() / CLOCKS_PER_SEC;
			double percent_done = (double) i / (height * width) * 100.0;
			sprintf(progressText, "Rendering Depth Of Field effect. Done %.1f%%", percent_done);
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);

			image->ConvertTo8bit();
			image->UpdatePreview();
			image->RedrawInWidget(darea);

			while (gtk_events_pending())
				gtk_main_iteration();
		}
		if (!isPostRendering) break;

	}

	if (!noGUI)
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), "Rendering Deptp Of Field effect. Done 100%");
		while (gtk_events_pending())
			gtk_main_iteration();
	}

	isPostRendering = false;
	delete temp_image;
	delete temp_alpha;
	delete temp_sort;
}

void ThreadSSAO(void *ptr)
{
	sSSAOparams *param;
	param = (sSSAOparams*) ptr;

	int quality = param->quality;
	double persp = param->persp;
	int threadNo = param->threadNo;
	cImage *image = param->image;
	int width = image->GetWidth();
	int height = image->GetHeight();

	double *cosinus = new double[quality];
	double *sinus = new double[quality];
	for (int i = 0; i < quality; i++)
	{
		sinus[i] = sin((double) i / quality * 2.0 * M_PI);
		cosinus[i] = cos((double) i / quality * 2.0 * M_PI);
	}

	double scale_factor = (double) width / (quality * quality) / 2.0;
	double aspectRatio = (double) width / height;

	bool sphericalPersp = param->fishEye;
	double fov = param->persp;

	for (int y = threadNo; y < height; y += NR_THREADS)
	{

		for (int x = 0; x < width; x++)
		{
			double z = image->GetPixelZBuffer(x,y);
			double total_ambient = 0;

			if (z < 1e19)
			{
				//printf("SSAO point on object\n");
				double x2, y2;
				if (sphericalPersp)
				{
					x2 = M_PI * ((double) x / width - 0.5) * aspectRatio;
					y2 = M_PI * ((double) y / height - 0.5);
					x2 = sin(fov * x2) * z;
					y2 = sin(fov * y2) * z;
				}
				else
				{
					x2 = ((double) x / width - 0.5) * aspectRatio;
					y2 = ((double) y / height - 0.5);
					x2 = x2 * (1.0 + z * persp);
					y2 = y2 * (1.0 + z * persp);
				}

				double ambient = 0;

				for (int angle = 0; angle < quality; angle++)
				{
					double ca = cosinus[angle];
					double sa = sinus[angle];

					double max_diff = -1e50;

					for (double r = 1.0; r < quality; r += 1.0)
					{
						double rr = r * r * scale_factor;
						double xx = x + rr * ca;
						double yy = y + rr * sa;

						if ((int) xx == (int) x && (int) yy == (int) y) continue;
						if (xx < 0 || xx > width - 1 || yy < 0 || yy > height - 1) continue;
						double z2 = image->GetPixelZBuffer(xx,yy);

						double xx2, yy2;
						if (sphericalPersp)
						{
							xx2 = M_PI * (xx / width - 0.5) * aspectRatio;
							yy2 = M_PI * (yy / height - 0.5);
							xx2 = sin(fov * xx2) * z2;
							yy2 = sin(fov * yy2) * z2;
						}
						else
						{
							xx2 = (xx / width - 0.5) * aspectRatio;
							yy2 = (yy / height - 0.5);
							xx2 = xx2 * (1.0 + z2 * persp);
							yy2 = yy2 * (1.0 + z2 * persp);
						}

						double dx = xx2 - x2;
						double dy = yy2 - y2;
						double dz = z2 - z;
						double dr = sqrt(dx * dx + dy * dy);
						double diff = -dz / dr;

						if (diff > max_diff) max_diff = diff;

					}
					double max_angle = atan(max_diff);

					ambient += -max_angle / M_PI + 0.5;

				}

				total_ambient = ambient / quality;
				if (total_ambient < 0) total_ambient = 0;

			}

			sRGB16 ambient = {total_ambient * 4096.0,total_ambient * 4096.0,total_ambient * 4096.0};
			image->PutPixelAmbient(x,y,ambient);
		}

		param->done++;

		if (!isPostRendering) break;
	}
	delete sinus;
	delete cosinus;

}

void PostRendering_SSAO(cImage *image, double persp, int quality)
{
	isPostRendering = true;

	int height = image->GetHeight();

	if (!noGUI)
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), "Rendering Screen Space Ambient Occlusion");
		while (gtk_events_pending())
			gtk_main_iteration();
	}

	GThread *Thread[NR_THREADS];
	GError *err[NR_THREADS];

	sSSAOparams thread_param[NR_THREADS];
	for (int i = 0; i < NR_THREADS; i++)
	{
		err[i] = NULL;
	}

	for (int i = 0; i < NR_THREADS; i++)
	{
		//sending some parameters to thread
		thread_param[i].threadNo = i;
		thread_param[i].image = image;
		thread_param[i].persp = persp;
		thread_param[i].fishEye = Interface_data.fishEye;
		thread_param[i].quality = quality;
		thread_param[i].done = 0;

		//creating thread
		Thread[i] = g_thread_create((GThreadFunc) ThreadSSAO, &thread_param[i], TRUE, &err[i]);
	}

	double last_time = (double) clock() / CLOCKS_PER_SEC;

	int total_done;

	if (!noGUI)
	{
		do
		{
			total_done = 0;
			for (int i = 0; i < NR_THREADS; i++)
			{
				total_done += thread_param[i].done;
			}

			double time = (double) clock() / CLOCKS_PER_SEC;

			if (time - last_time > 0.5)
			{
				char progressText[1000];
				last_time = (double) clock() / CLOCKS_PER_SEC;
				double percent_done = (double) total_done / height * 100.0;
				sprintf(progressText, "Rendering Screen Space Ambient Occlusion. Done %.1f%%", percent_done);
				gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), progressText);
				while (gtk_events_pending())
					gtk_main_iteration();
			}
		} while (total_done < height && isPostRendering);

		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(Interface.progressBar), "Rendering Screen Space Ambient Occlusion. Done 100%");
		while (gtk_events_pending())
			gtk_main_iteration();
	}
	else
	{
		printf("Rendering Screen Space Ambient Occlusion\n");
	}
	for (int i = 0; i < NR_THREADS; i++)
	{
		g_thread_join(Thread[i]);
		//printf("Rendering thread #%d finished\n", i + 1);
	}

	isPostRendering = false;
}

void DrawHistogram(void)
{
	GdkGC *GC = gdk_gc_new(darea2->window);
	GdkColor color_black = { 0, 0, 0, 0 };
	gdk_gc_set_rgb_fg_color(GC, &color_black);
	gdk_draw_rectangle(darea2->window, GC, true, 0, 0, 256, 128);

	GdkColor color_red = { 0, 65535, 0, 0 };
	gdk_gc_set_rgb_fg_color(GC, &color_red);

	guint64 max = 0;
	for (int i = 0; i < 64; i++)
	{
		if (histogram[i] > max) max = histogram[i];
	}

	for (int i = 0; i < 64; i++)
	{
		int height = 127.0 * histogram[i] / max;
		gdk_draw_rectangle(darea2->window, GC, true, i * 4, 127 - height, 4, height);

	}
}

void DrawHistogram2(void)
{

	GdkGC *GC = gdk_gc_new(darea2->window);
	GdkColor color_black = { 0, 10000, 10000, 10000 };
	gdk_gc_set_rgb_fg_color(GC, &color_black);
	gdk_draw_rectangle(darea2->window, GC, true, 256, 0, 256, 128);
	GdkColor color_red = { 0, 0, 65535, 0 };
	gdk_gc_set_rgb_fg_color(GC, &color_red);

	unsigned int max = 0;
	for (int i = 0; i < 256; i++)
	{
		if (histogram2[i] > max) max = histogram2[i];
	}

	for (int i = 0; i < 256; i++)
	{
		int height = 127.0 * histogram2[i] / max;
		gdk_draw_rectangle(darea2->window, GC, true, i * 1 + 256, 127 - height, 1, height);

	}
}

//************************** ShadedBackgroud *****************************
/*
 void ShadedBackground(bool losuj, sRGB col1, sRGB col2)
 {
 static double wsp1, wsp2, wsp3, wsp4, wsp5, wsp6, wsp7, wsp8;
 if (losuj)
 {
 wsp1 = Random(1000) / 200.0 + 0.1;
 wsp2 = Random(1000) / 200.0 + 0.1;
 wsp3 = Random(1000) / 200.0 + 0.1;
 wsp4 = Random(1000) / 200.0 + 0.1;
 wsp5 = Random(1000) / 200.0 + 0.1;
 wsp6 = Random(1000) / 200.0 + 0.1;
 wsp7 = Random(1000) / 200.0 + 0.1;
 wsp8 = Random(1000) / 200.0 + 0.1;
 }
 for (int y = 0; y < IMAGE_HEIGHT; y++)
 {

 for (int x = 0; x < IMAGE_WIDTH; x++)
 {

 double xx = (double) x / IMAGE_WIDTH;
 double yy = (double) y / IMAGE_HEIGHT;
 int nrkolor = 1024.0 * (2.0 + sin(xx * wsp1 + wsp5) * cos(yy * wsp2 + wsp6) + sin(xx * wsp3 + wsp7) * cos(yy * wsp4 + wsp8));
 sRGB color = Przekolor(nrkolor, paleta, 256);
 int R = color.R * 256;
 int G = color.G * 256;
 int B = color.B * 256;
 nrkolor = 1024.0 + 1024.0 * (2.0 + sin(xx * wsp5 + wsp1) * cos(yy * wsp6 + wsp2) + sin(xx * wsp7 + wsp3) * cos(yy * wsp8 + wsp4));
 color = Przekolor(nrkolor, paleta, 256);
 R = (R + color.R * 256) / 2;
 G = (G + color.G * 256) / 2;
 B = (B + color.B * 256) / 2;

 //PutPixel(x, y, R / 4, G / 4, B / 4);


 PutPixel(x, y, 0, 0, 0);
 }

 }
 }
 */

//************************** BitmapBackgroud *****************************
/*
 void BitmapBackground(guchar *tlo, int width, int height)
 {
 double scaleX = (double) width / (IMAGE_WIDTH + 2);
 double scaleY = (double) height / (IMAGE_HEIGHT + 2);
 for (int y = 0; y < IMAGE_WIDTH; y++)
 {
 for (int x = 0; x < IMAGE_HEIGHT; x++)
 {
 int xx = x * scaleX;
 int yy = y * scaleY;
 for (int i = 0; i < 3; i++)
 {

 int adres1 = (x + y * IMAGE_WIDTH) * 3 + i;
 int adres2 = (xx + yy * width) * 3 + i;
 rgbbuf32[adres1] = tlo[adres2] * 256;
 }
 }
 }
 }
 */

void DrawPalette(sRGB *palette)
{
	mainImage->SetPalette(palette);

	if (paletteViewCreated)
	{
		double colWidth = 10;
		GdkGC *GC = gdk_gc_new(dareaPalette->window);
		for (int i = 0; i < 640; i++)
		{
			int number = (int) (i*256.0/colWidth + Interface_data.palette_offset*256.0);
			sRGB color = mainImage->IndexToColour(number);
			GdkColor gdk_color = { 0, color.R * 256, color.G * 256, color.B * 256 };
			gdk_gc_set_rgb_fg_color(GC, &gdk_color);
			gdk_draw_line(dareaPalette->window, GC, i, 0, i, 30);
		}
	}
}

/*
void StoreImage8(sComplexImage *image, sRGB8 *image8)
{
	for (int y = 0; y < IMAGE_HEIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			unsigned int address = x + y * IMAGE_WIDTH;
			image8[address].R = image[address].image.R / 256;
			image8[address].G = image[address].image.G / 256;
			image8[address].B = image[address].image.B / 256;
		}
	}
}

void MakeStereoImage(sRGB8 *left, sRGB8 *right, guchar *stereoImage)
{
	for (int y = 0; y < IMAGE_HEIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			unsigned int addressSource = x + y * IMAGE_WIDTH;
			unsigned int addressDest = (x + y * IMAGE_WIDTH * 2) * 3;
			stereoImage[addressDest + 0] = right[addressSource].R;
			stereoImage[addressDest + 1] = right[addressSource].G;
			stereoImage[addressDest + 2] = right[addressSource].B;
			stereoImage[addressDest + 0 + IMAGE_WIDTH * 3] = left[addressSource].R;
			stereoImage[addressDest + 1 + IMAGE_WIDTH * 3] = left[addressSource].G;
			stereoImage[addressDest + 2 + IMAGE_WIDTH * 3] = left[addressSource].B;
		}
	}
}

void StereoPreview(guchar *stereoImage)
{
	guchar *preview = new guchar[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
	memset(preview, 0, IMAGE_WIDTH * IMAGE_HEIGHT * 3);
	for (int y = 0; y < IMAGE_HEIGHT / 2; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH; x++)
		{
			int R = 0;
			int G = 0;
			int B = 0;
			unsigned int addressDest = (x + (y + IMAGE_HEIGHT / 4) * IMAGE_WIDTH) * 3;
			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					unsigned int addressSource = ((x * 2 + i) + (y * 2 + j) * IMAGE_WIDTH * 2) * 3;
					R += stereoImage[addressSource + 0];
					G += stereoImage[addressSource + 1];
					B += stereoImage[addressSource + 2];
				}
			}
			R = R / 4;
			G = G / 4;
			B = B / 4;
			preview[addressDest + 0] = R;
			preview[addressDest + 1] = G;
			preview[addressDest + 2] = B;
		}
	}
	ScaleImage(preview, rgbbuf);
	RedrawImage(darea, rgbbuf);
	delete preview;
}
*/

