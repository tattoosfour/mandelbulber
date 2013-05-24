/*
 * cimage.hpp
 *
 *  Created on: 2010-08-29
 *      Author: krzysztof
 */

#ifndef CIMAGE_HPP_
#define CIMAGE_HPP_

#include <gtk-2.0/gtk/gtk.h>

//global variables
struct sRGB8
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

struct sRGB16
{
	unsigned short R;
	unsigned short G;
	unsigned short B;
};

struct sRGBfloat
{
	float R;
	float G;
	float B;
};

struct sRGB
{
	int R;
	int G;
	int B;
};

struct sImageAdjustments
{
  double shading;
  double directLight;
  double ambient;
  double specular;
  double reflect;
  double globalIlum;
  double brightness;
  double glow_intensity;
  double fogVisibility;
  double fogVisibilityFront;
  double coloring_speed;
  double imageGamma;
  double paletteOffset;
  double mainLightIntensity;
  double volumetricLightIntensity;
};

struct sEffectColours
{
	sRGB glow_color1;
	sRGB glow_color2;
	sRGB fogColor;
	sRGB mainLightColour;
};

struct sImageSwitches
{
	bool coloringEnabled;
	bool fogEnabled;
	bool raytracedReflections;
	bool volumetricLightEnabled;
	bool iterFogEnabled;
};

struct sAllImageData
{
	sRGBfloat imageFloat;
	unsigned short alphaBuffer;
	unsigned short opacityBuffer;
	sRGB8 colourBuffer;
	float zBuffer;
};

class cImage
{
public:
	cImage(int w, int h, bool low_mem = false);
	~cImage();
	void ChangeSize(int w, int h);
	void ClearImage(void);
	inline void PutPixelImage(int x, int y, sRGBfloat pixel)	{if (x >= 0 && x < width && y >= 0 && y < height) imageFloat[x + y * width] = pixel;}
	inline void PutPixelImage16(int x, int y, sRGB16 pixel)	{if (x >= 0 && x < width && y >= 0 && y < height) image16[x + y * width] = pixel;}
	inline void PutPixelColour(int x, int y, sRGB8 pixel)	{if (x >= 0 && x < width && y >= 0 && y < height) colourBuffer[x + y * width] = pixel;}
	inline void PutPixelZBuffer(int x, int y, float pixel) {if (x >= 0 && x < width && y >= 0 && y < height) zBuffer[x + y * width] = pixel;}
	inline void PutPixelAlpha(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) alphaBuffer[x + y * width] = pixel;}
	inline void PutPixelOpacity(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) opacityBuffer[x + y * width] = pixel;}

  inline sRGBfloat GetPixelImage(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return imageFloat[x + y * width]; else return BlackFloat();}
  inline sRGB16 GetPixelImage16(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return image16[x + y * width]; else return Black16();}
  inline short int GetPixelAlpha(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return alphaBuffer[x + y * width]; else return 0;}
  inline short int GetPixelOpacity(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return opacityBuffer[x + y * width]; else return 0;}
  inline sRGB8 GetPixelColor(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return colourBuffer[x + y * width]; else return Black8();}
  inline float GetPixelZBuffer(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return zBuffer[x + y * width]; else return 1e20;}

  sRGB16* GetImage16Ptr(void) {return image16;}
  unsigned short* GetAlphaBufPtr(void) {return alphaBuffer;}

  void CompileImage(void);

  int GetWidth(void) {return width;}
  int GetHeight(void) {return height;}
  int GetPreviewWidth(void) {return previewWidth;}
  int GetPreviewHeight(void) {return previewHeight;}
  int GetUsedMB(void);
  void SetImageParameters(sImageAdjustments adjustments);
  sImageAdjustments* GetImageAdjustments(void) {return &adj;}

  unsigned char* ConvertTo8bit(void);
  unsigned char* CreatePreview(double scale);
  void UpdatePreview(void);
  unsigned char* GetPreviewPtr(void);
  bool IsPreview(void);
  void RedrawInWidget(GtkWidget *dareaWidget);
  double GetPreviewScale() {return previewScale;}
  void Squares(int y, int progressiveFactor);
  void CalculateGammaTable(void);
  sRGB16 CalculatePixel(sRGBfloat pixel);
	int progressiveFactor;

private:
  sRGB8 Interpolation(float x, float y);
	void AllocMem(void);
	inline sRGB16 Black16(void) {sRGB16 black = {0,0,0};return black;}
	inline sRGB8 Black8(void) {sRGB8 black = {0,0,0};return black;}
	inline sRGBfloat BlackFloat(void) {sRGBfloat black = {0.0,0.0,0.0};return black;}

	sRGB8 *image8;
	sRGB16 *image16;
	sRGBfloat *imageFloat;

	unsigned short *alphaBuffer;
	unsigned short *opacityBuffer;
	sRGB8 *colourBuffer;
	float *zBuffer;

	sRGB8 *preview;
	sImageAdjustments adj;
	int width;
	int height;
	int *gammaTable;
	bool previewAllocated;
	int previewWidth;
	int previewHeight;
	double previewScale;
	bool lowMem;
};

sRGB PostRendering_Fog(double z, double min, double max, sRGB fog_color, sRGB color_before);

#endif /* CIMAGE_HPP_ */
