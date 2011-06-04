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

struct sRGB
{
	int R;
	int G;
	int B;
};

struct sComplexImage
{
	unsigned short shadowsBuf16;
	unsigned short shadingBuf16;
	unsigned short specularBuf16;
	sRGB16 auxLight;
	sRGB16 auxSpecular;
	unsigned short glowBuf16;
	sRGB16 reflectBuf16;
	sRGB16 ambientBuf16;
	sRGB16 backgroundBuf16;
	sRGB16 volumetricFog;
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
};

class cImage
{
public:
	cImage(int w, int h, bool low_mem = false);
	~cImage();
	void ChangeSize(int w, int h);
	void SetLowMem(bool low_mem);
	void ClearImage(void);
	inline void PutPixelImage(int x, int y, sRGB16 pixel)	{if (x >= 0 && x < width && y >= 0 && y < height) image16[x + y * width] = pixel;	}
	inline void PutPixelZBuffer(int x, int y, float pixel) {if (x >= 0 && x < width && y >= 0 && y < height) zBuffer[x + y * width] = pixel;}
	inline void PutPixelShadow(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].shadowsBuf16 = pixel;}
	inline void PutPixelShading(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].shadingBuf16 = pixel;}
	inline void PutPixelSpecular(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].specularBuf16 = pixel;}
	inline void PutPixelAuxLight(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].auxLight = pixel;}
	inline void PutPixelAuxSpecular(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].auxSpecular = pixel;}
	inline void PutPixelGlow(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].glowBuf16 = pixel;}
	inline void PutPixelReflect(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].reflectBuf16 = pixel;}
	inline void PutPixelAmbient(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].ambientBuf16 = pixel;}
	inline void PutPixelColor(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) colorIndexBuf16[x + y * width] = pixel;}
	inline void PutPixelBackground(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].backgroundBuf16 = pixel;}
	inline void PutPixelAlpha(int x, int y, unsigned short pixel) {if (x >= 0 && x < width && y >= 0 && y < height) alpha[x + y * width] = pixel;}
	inline void PutPixelVolumetricFog(int x, int y, sRGB16 pixel) {if (x >= 0 && x < width && y >= 0 && y < height) complexImage[x + y * width].volumetricFog = pixel;}
  inline sRGB16 GetPixelImage(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return image16[x + y * width]; else return Black16();}
  inline short int GetPixelAlpha(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return alpha[x + y * width]; else return 0;}
  inline short int GetPixelColor(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return colorIndexBuf16[x + y * width]; else return 0;}
  inline sRGB16 GetPixelAmbient(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return complexImage[x + y * width].ambientBuf16; else return Black16();}
  inline float GetPixelZBuffer(int x, int y)  {if (x >= 0 && x < width && y >= 0 && y < height) return zBuffer[x + y * width]; else return 1e20;}
  sRGB16* GetImage16Ptr(void) {return image16;}
  void CompileImage(void);
  void SetPalette(sRGB *palette);
  sRGB* GetPalettePtr() {return palette;}
  sRGB IndexToColour(int index);
  int GetWidth(void) {return width;}
  int GetHeight(void) {return height;}
  int GetPreviewWidth(void) {return previewWidth;}
  int GetPreviewHeight(void) {return previewHeight;}
  int GetUsedMB(void);
  void SetImageParameters(sImageAdjustments adjustments, sEffectColours effectColours, sImageSwitches switches);
  void SetImageAdjustments(sImageAdjustments adjustments) {adj = adjustments;}
  sImageAdjustments* GetImageAdjustments(void) {return &adj;}
  sEffectColours* GetEffectColours(void) {return &ecol;}
  sImageSwitches* GetImageSwitches(void) {return &sw;}
  unsigned char* ConvertTo8bit(void);
  unsigned char* CreatePreview(double scale);
  void UpdatePreview(void);
  unsigned char* GetPreviewPtr(void);
  bool IsPreview(void);
  bool IsLowMemMode(void) {return lowMem;}
  void RedrawInWidget(GtkWidget *dareaWidget);
  double GetPreviewScale() {return previewScale;}
  void Squares(int y, int progressiveFactor);
  void CalculateGammaTable(void);
  sRGB16 CalculatePixel(sComplexImage &pixel, unsigned short &alpha, float &zBuf, unsigned short colorIndex, double fogVisBack, double fogVisFront);
  sRGB16 CalculateAmbientPixel(sRGB16 ambient16, unsigned short colorIndex, sRGB16 oldPixel16);
	int progressiveFactor;

private:
  sRGB8 Interpolation(float x, float y);
	void AllocMem(void);
	inline sRGB16 Black16(void) {sRGB16 black = {0,0,0};return black;}
  sComplexImage *complexImage;
	sRGB16 *image16;
	sRGB8 *image8;
	unsigned short *alpha;
	float *zBuffer;
	unsigned short *colorIndexBuf16;

	sRGB8 *preview;
	sImageAdjustments adj;
	sEffectColours ecol;
	sImageSwitches sw;
	sRGB *palette;
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
